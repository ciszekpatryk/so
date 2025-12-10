#!/bin/bash
export LC_ALL=C
set -e

if [ $# -lt 1 ]; then
    echo "Użycie: $0 katalog1 [katalog2 ...]"
    exit 1
fi

#tablice kompresorów, dekompresorów i rozszerzeń
compressors=("gzip -k" "bzip2 -k" "xz -k" "zstd -k" "lz4 -f" "7z a -bd -y")
decompressors=("gzip -dk" "bzip2 -dk" "xz -dk" "zstd -d -k" "lz4 -d" "")
extensions=(".gz" ".bz2" ".xz" ".zst" ".lz4" ".7z")

for dir in "$@"; do
    echo "$dir"
    tmp_dir=$(mktemp -d)
    tarfile="$tmp_dir/archive.tar"
    tar cf "$tarfile" -C "$dir" .

    echo -e "name\tcompress\tdecompress\tratio"
    original_size=$(stat -c%s "$tarfile")

    for i in ${!compressors[@]}; do
        comp="${compressors[$i]}"
        decomp="${decompressors[$i]}"
        ext="${extensions[$i]}"
        compfile="$tarfile$ext"

        #kompresja
        start=$(date +%s.%N)
        if [[ "$ext" == ".lz4" ]]; then
            $comp "$tarfile" "$compfile" >/dev/null 2>&1
        elif [[ "$ext" == ".7z" ]]; then
            $comp "$compfile" "$tarfile" >/dev/null 2>&1
        else
            $comp "$tarfile" >/dev/null 2>&1
        fi
        end=$(date +%s.%N)
        comp_time=$(echo "$end - $start" | bc -l)

        #dekompresja
        start=$(date +%s.%N)
        if [[ "$ext" == ".lz4" ]]; then
            $decomp "$compfile" "$tmp_dir/archive_lz4_decompressed.tar" >/dev/null 2>&1
            rm -f "$tmp_dir/archive_lz4_decompressed.tar"
        elif [[ "$ext" == ".7z" ]]; then
            7z x "$compfile" -bd -y -o"$tmp_dir" >/dev/null 2>&1
        else
            $decomp "$compfile" -c >/dev/null 2>&1
        fi
        end=$(date +%s.%N)
        decomp_time=$(echo "$end - $start" | bc -l)

        #ratio
        ratio=$(echo "scale=2; 100 * $(stat -c%s "$compfile") / $original_size" | bc)

        printf "%s\t%f\t%f\t%s%%\n" "$(echo $comp | awk '{print $1}')" "$comp_time" "$decomp_time" "$ratio"

        #czyszczenie
        rm -f "$compfile"
    done

    rm -f "$tarfile"
    rm -rf "$tmp_dir"
    echo
done

