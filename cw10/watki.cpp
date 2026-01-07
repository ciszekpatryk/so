#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <cmath>
#include <cctype>

#define LETTER_COUNT 26

struct context {
    std::string content;
    unsigned long count[LETTER_COUNT] = {0};
    double sumOfSquares = 0.0;
    std::mutex mutex;
};

void countPart(context& ctx, size_t start, size_t end) {
    for (size_t i = start; i < end; i++) {
        unsigned char c = ctx.content[i];

        // liczenie pierwiastków ASCII
        double value = std::sqrt((double)c);

        // jeśli to litera
        if (std::isalpha(c)) {
            char lower = std::tolower(c);
            int index = lower - 'a';

            std::lock_guard<std::mutex> lock(ctx.mutex);
            ctx.count[index]++;
            ctx.sumOfSquares += value;
        } else {
            std::lock_guard<std::mutex> lock(ctx.mutex);
            ctx.sumOfSquares += value;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Użycie: program <plik> [ilosc_watkow]\n";
        return 1;
    }

    int threadCount;
    if (argc >= 3) {
        threadCount = std::stoi(argv[2]);
    } else {
        threadCount = std::thread::hardware_concurrency();
    }

    context ctx;

    // wczytanie pliku
    std::ifstream file(argv[1]);
    if (!file) {
        std::cout << "Nie mozna otworzyc pliku\n";
        return 1;
    }

    ctx.content.assign(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );

    size_t length = ctx.content.size();
    size_t partSize = length / threadCount;

    std::vector<std::thread> threads;

    for (int i = 0; i < threadCount; i++) {
        size_t start = i * partSize;
        size_t end = (i == threadCount - 1) ? length : start + partSize;

        threads.push_back(
            std::thread(countPart, std::ref(ctx), start, end)
        );
    }

    // czekamy na wątki
    for (auto& t : threads) {
        t.join();
    }

    // wypisanie wyników
    std::cout << "Liczba wystapien liter:\n";
    for (int i = 0; i < LETTER_COUNT; i++) {
        std::cout << char('a' + i) << ": " << ctx.count[i] << "\n";
    }

    std::cout << "\nSuma pierwiastkow kodow ASCII: "
              << ctx.sumOfSquares << "\n";

    return 0;
}

