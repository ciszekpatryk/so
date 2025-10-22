#!/usr/bin/env python3
import sys
import time

def main():
    if len(sys.argv) != 2:
        print("Uzycie: python3 licznik.py <nazwa_pliku>")
        sys.exit(1)

    filename = sys.argv[1]

    with open(filename, "w") as f:
        i=0
        while True:
            f.write(f"{i}\n")
            f.flush()
            time.sleep(1)
            i += 1

if __name__ == "__main__":
    main()