#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pthread.h>
#include <cmath>
#include <cctype>
#include <cstring>
#include <cstdlib>

using namespace std;

struct SharedData {
    unsigned int letters[26];
    double sum;
    pthread_mutex_t mutex;
};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "Użycie: " << argv[0] << " plik [liczba_procesów]\n";
        return 1;
    }

    int processes;
    if (argc >= 3) {
        processes = atoi(argv[2]);
        if (processes < 1)
            processes = 1;
    } else {
        processes = sysconf(_SC_NPROCESSORS_ONLN);
        if (processes < 1)
            processes = 1;
    }

    /* Otwieranie pliku */
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        close(fd);
        return 1;
    }

    size_t file_size = st.st_size;
    if (file_size == 0) {
        cout << "Plik jest pusty\n";
        close(fd);
        return 0;
    }

    /* Mapowanie pliku */
    char *file_data = (char *)mmap(nullptr, file_size,
                                   PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_data == MAP_FAILED) {
        perror("mmap pliku");
        close(fd);
        return 1;
    }

    /* Pamięć współdzielona */
    SharedData *shared = (SharedData *)mmap(nullptr, sizeof(SharedData),
                                            PROT_READ | PROT_WRITE,
                                            MAP_SHARED | MAP_ANONYMOUS,
                                            -1, 0);
    if (shared == MAP_FAILED) {
        perror("mmap shared");
        return 1;
    }

    memset(shared->letters, 0, sizeof(shared->letters));
    shared->sum = 0.0;

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&shared->mutex, &attr);

    size_t chunk = file_size / processes;

    /* Tworzenie procesów */
    for (int i = 0; i < processes; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            size_t start = i * chunk;
            size_t end = (i == processes - 1) ? file_size : start + chunk;

            unsigned int local_letters[26] = {0};
            double local_sum = 0.0;

            for (size_t j = start; j < end; j++) {
                unsigned char c = file_data[j];
                local_sum += sqrt((double)c);

                if (isalpha(c)) {
                    c = tolower(c);
                    if (c >= 'a' && c <= 'z') {
                        local_letters[c - 'a']++;
                    }
                }
            }

            pthread_mutex_lock(&shared->mutex);
            for (int k = 0; k < 26; k++) {
                shared->letters[k] += local_letters[k];
            }
            shared->sum += local_sum;
            pthread_mutex_unlock(&shared->mutex);

            munmap(file_data, file_size);
            exit(0);
        }
    }

    /* Czekanie na procesy potomne */
    for (int i = 0; i < processes; i++) {
        wait(nullptr);
    }

    /* Wyniki */
    cout << "Wystąpienia liter:\n";
    for (int i = 0; i < 26; i++) {
        cout << char('a' + i) << ": " << shared->letters[i] << endl;
    }

    cout << "Suma pierwiastków kodów ASCII: " << shared->sum << endl;

    /* Sprzątanie */
    pthread_mutex_destroy(&shared->mutex);
    munmap(shared, sizeof(SharedData));
    munmap(file_data, file_size);
    close(fd);

    return 0;
}

