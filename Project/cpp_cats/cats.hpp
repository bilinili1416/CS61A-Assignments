#pragma once

#include <functional>
#include <string>
#include <vector>

namespace cats {

using Selector = std::function<bool(const std::string&)>;
using DiffFunction = std::function<int(const std::string&, const std::string&, int)>;

struct ProgressReport {
    int id;
    double progress;
};

using UploadFunction = std::function<void(const ProgressReport&)>;

struct WordsAndTimes {
    std::vector<std::string> words;
    std::vector<std::vector<double>> times;
};

std::vector<std::string> lines_from_file(const std::string& path);
std::string remove_punctuation(const std::string& text);
std::string lower(std::string text);
std::vector<std::string> split(const std::string& text);

std::string pick(const std::vector<std::string>& paragraphs,
                 const Selector& select,
                 std::size_t k);
Selector about(const std::vector<std::string>& subject);
double accuracy(const std::string& typed, const std::string& source);
double wpm(const std::string& typed, double elapsed_seconds);

DiffFunction memo_diff(DiffFunction diff_function);
std::string autocorrect(const std::string& typed_word,
                        const std::vector<std::string>& word_list,
                        const DiffFunction& diff_function,
                        int limit);
int furry_fixes(const std::string& typed, const std::string& source, int limit);
int minimum_mewtations(const std::string& typed, const std::string& source, int limit);
int final_diff(const std::string& typed, const std::string& source, int limit);

double report_progress(const std::vector<std::string>& typed,
                       const std::vector<std::string>& source,
                       int user_id,
                       const UploadFunction& upload);
WordsAndTimes time_per_word(const std::vector<std::string>& words,
                            const std::vector<std::vector<double>>& timestamps_per_player);
double get_time(const WordsAndTimes& words_and_times,
                std::size_t player_num,
                std::size_t word_index);
std::vector<std::vector<std::string>> fastest_words(const WordsAndTimes& words_and_times);

}  // namespace cats
