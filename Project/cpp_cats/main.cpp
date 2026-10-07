#include "cats.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef CATS_DATA_DIR
#define CATS_DATA_DIR "data"
#endif

namespace {

void print_usage(const char* executable) {
    std::cout << "Usage: " << executable << " -t [topic ...]\n"
              << "\nOptions:\n"
              << "  -t, --test  Run the typing test\n"
              << "  -h, --help  Show this help\n";
}

void run_typing_test(const std::vector<std::string>& topics) {
    auto paragraphs = cats::lines_from_file(
        (std::filesystem::path(CATS_DATA_DIR) / "sample_paragraphs.txt").string());
    std::mt19937 generator(std::random_device{}());
    std::shuffle(paragraphs.begin(), paragraphs.end(), generator);
    const cats::Selector select = topics.empty() ? cats::Selector([](const std::string&) { return true; })
                                                  : cats::about(topics);

    for (std::size_t index = 0;; ++index) {
        const std::string source = cats::pick(paragraphs, select, index);
        if (source.empty()) {
            std::cout << "No more matching paragraphs are available.\n";
            return;
        }

        std::cout << "Type the following paragraph and then press enter.\n"
                  << "If you type only part of it, only that part will be scored.\n\n"
                  << source << "\n\n";
        const auto start = std::chrono::steady_clock::now();
        std::string typed;
        if (!std::getline(std::cin, typed) || typed.empty()) {
            std::cout << "Goodbye.\n";
            return;
        }
        const auto stop = std::chrono::steady_clock::now();
        const double elapsed = std::chrono::duration<double>(stop - start).count();

        std::cout << "\nNice work!\n"
                  << "Words per minute: " << cats::wpm(typed, elapsed) << '\n'
                  << "Accuracy:         " << cats::accuracy(typed, source) << "%\n\n"
                  << "Press enter for the next paragraph or type q to quit.\n";
        std::string command;
        if (!std::getline(std::cin, command) || command == "q") {
            return;
        }
    }
}

}  // namespace

int main(int argc, char* argv[]) {
    bool run_test = false;
    std::vector<std::string> topics;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "-t" || argument == "--test") {
            run_test = true;
        } else if (argument == "-h" || argument == "--help") {
            print_usage(argv[0]);
            return 0;
        } else {
            topics.push_back(cats::lower(argument));
        }
    }

    if (!run_test) {
        print_usage(argv[0]);
        return 0;
    }
    try {
        run_typing_test(topics);
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
