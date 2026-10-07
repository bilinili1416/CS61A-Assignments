#include "cats.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace cats {

std::vector<std::string> lines_from_file(const std::string& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Could not open file: " + path);
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        const auto first = line.find_first_not_of(" \t\n\r\f\v");
        const auto last = line.find_last_not_of(" \t\n\r\f\v");
        lines.push_back(first == std::string::npos ? "" : line.substr(first, last - first + 1));
    }
    return lines;
}

std::string remove_punctuation(const std::string& text) {
    std::string result;
    result.reserve(text.size());
    for (unsigned char character : text) {
        if (!std::ispunct(character)) {
            result.push_back(static_cast<char>(character));
        }
    }
    return result;
}

std::string lower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return text;
}

std::vector<std::string> split(const std::string& text) {
    std::istringstream input(text);
    std::vector<std::string> words;
    for (std::string word; input >> word;) {
        words.push_back(std::move(word));
    }
    return words;
}

std::string pick(const std::vector<std::string>& paragraphs,
                 const Selector& select,
                 std::size_t k) {
    for (const auto& paragraph : paragraphs) {
        if (select(paragraph)) {
            if (k == 0) {
                return paragraph;
            }
            --k;
        }
    }
    return "";
}

Selector about(const std::vector<std::string>& subject) {
    std::unordered_set<std::string> subjects;
    for (const auto& word : subject) {
        if (lower(word) != word) {
            throw std::invalid_argument("subjects should be lowercase");
        }
        subjects.insert(word);
    }

    return [subjects = std::move(subjects)](const std::string& paragraph) {
        for (const auto& word : split(lower(remove_punctuation(paragraph)))) {
            if (subjects.count(word) != 0) {
                return true;
            }
        }
        return false;
    };
}

double accuracy(const std::string& typed, const std::string& source) {
    const auto typed_words = split(typed);
    const auto source_words = split(source);
    if (typed_words.empty()) {
        return source_words.empty() ? 100.0 : 0.0;
    }

    std::size_t matches = 0;
    const auto comparable = std::min(typed_words.size(), source_words.size());
    for (std::size_t index = 0; index < comparable; ++index) {
        if (typed_words[index] == source_words[index]) {
            ++matches;
        }
    }
    return 100.0 * static_cast<double>(matches) / static_cast<double>(typed_words.size());
}

double wpm(const std::string& typed, double elapsed_seconds) {
    if (elapsed_seconds <= 0.0) {
        throw std::invalid_argument("Elapsed time must be positive");
    }
    return static_cast<double>(typed.size()) / 5.0 * 60.0 / elapsed_seconds;
}

namespace {

struct DiffKey {
    std::string typed;
    std::string source;
    int limit;

    bool operator==(const DiffKey& other) const {
        return typed == other.typed && source == other.source && limit == other.limit;
    }
};

struct DiffKeyHash {
    std::size_t operator()(const DiffKey& key) const {
        std::size_t seed = std::hash<std::string>{}(key.typed);
        seed ^= std::hash<std::string>{}(key.source) + 0x9e3779b9U + (seed << 6U) + (seed >> 2U);
        seed ^= std::hash<int>{}(key.limit) + 0x9e3779b9U + (seed << 6U) + (seed >> 2U);
        return seed;
    }
};

int bounded_edit_distance(const std::string& typed,
                          const std::string& source,
                          int limit,
                          bool allow_transposition) {
    if (limit < 0) {
        return limit + 1;
    }
    if (typed.empty() || source.empty()) {
        const int remaining = static_cast<int>(typed.size() + source.size());
        return remaining <= limit ? remaining : limit + 1;
    }
    if (std::abs(static_cast<int>(typed.size()) - static_cast<int>(source.size())) > limit) {
        return limit + 1;
    }

    const int too_large = limit + 1;
    std::vector<int> previous_previous(source.size() + 1, too_large);
    std::vector<int> previous(source.size() + 1, too_large);
    std::vector<int> current(source.size() + 1, too_large);
    for (std::size_t column = 0; column <= source.size(); ++column) {
        previous[column] = static_cast<int>(column) <= limit
                               ? static_cast<int>(column)
                               : too_large;
    }

    for (std::size_t row = 1; row <= typed.size(); ++row) {
        std::fill(current.begin(), current.end(), too_large);
        if (static_cast<int>(row) <= limit) {
            current[0] = static_cast<int>(row);
        }
        const std::size_t start = row > static_cast<std::size_t>(limit)
                                      ? row - static_cast<std::size_t>(limit)
                                      : 1;
        const std::size_t finish = std::min(source.size(), row + static_cast<std::size_t>(limit));
        int row_best = too_large;
        for (std::size_t column = start; column <= finish; ++column) {
            const int substitution_cost = typed[row - 1] == source[column - 1] ? 0 : 1;
            int value = std::min({previous[column] + 1,
                                  current[column - 1] + 1,
                                  previous[column - 1] + substitution_cost});
            if (allow_transposition && row > 1 && column > 1 &&
                typed[row - 1] == source[column - 2] &&
                typed[row - 2] == source[column - 1]) {
                value = std::min(value, previous_previous[column - 2] + 1);
            }
            current[column] = std::min(value, too_large);
            row_best = std::min(row_best, current[column]);
        }
        if (row_best > limit) {
            return too_large;
        }
        previous_previous.swap(previous);
        previous.swap(current);
    }
    return std::min(previous[source.size()], too_large);
}

void validate_words_and_times(const WordsAndTimes& value) {
    for (const auto& player_times : value.times) {
        if (player_times.size() != value.words.size()) {
            throw std::invalid_argument("There should be one word per time");
        }
    }
}

}  // namespace

DiffFunction memo_diff(DiffFunction diff_function) {
    return [diff_function = std::move(diff_function),
            cache = std::unordered_map<DiffKey, int, DiffKeyHash>{}](
               const std::string& typed, const std::string& source, int limit) mutable {
        DiffKey key{typed, source, limit};
        const auto found = cache.find(key);
        if (found != cache.end()) {
            return found->second;
        }
        const int result = diff_function(typed, source, limit);
        cache.emplace(std::move(key), result);
        return result;
    };
}

std::string autocorrect(const std::string& typed_word,
                        const std::vector<std::string>& word_list,
                        const DiffFunction& diff_function,
                        int limit) {
    if (std::find(word_list.begin(), word_list.end(), typed_word) != word_list.end()) {
        return typed_word;
    }
    if (word_list.empty()) {
        return typed_word;
    }

    const std::string* best_word = nullptr;
    int best_difference = limit + 1;
    for (const auto& word : word_list) {
        const int difference = diff_function(typed_word, word, limit);
        if (difference < best_difference) {
            best_difference = difference;
            best_word = &word;
        }
    }
    return best_word != nullptr && best_difference <= limit ? *best_word : typed_word;
}

int furry_fixes(const std::string& typed, const std::string& source, int limit) {
    int differences = std::abs(static_cast<int>(typed.size()) - static_cast<int>(source.size()));
    const std::size_t shared_length = std::min(typed.size(), source.size());
    for (std::size_t index = 0; index < shared_length && differences <= limit; ++index) {
        differences += typed[index] != source[index] ? 1 : 0;
    }
    return differences > limit ? limit + 1 : differences;
}

int minimum_mewtations(const std::string& typed, const std::string& source, int limit) {
    return bounded_edit_distance(typed, source, limit, false);
}

int final_diff(const std::string& typed, const std::string& source, int limit) {
    return bounded_edit_distance(typed, source, limit, true);
}

double report_progress(const std::vector<std::string>& typed,
                       const std::vector<std::string>& source,
                       int user_id,
                       const UploadFunction& upload) {
    std::size_t correct = 0;
    while (correct < typed.size() && correct < source.size() && typed[correct] == source[correct]) {
        ++correct;
    }
    const double progress = source.empty()
                                ? (typed.empty() ? 1.0 : 0.0)
                                : static_cast<double>(correct) / static_cast<double>(source.size());
    upload(ProgressReport{user_id, progress});
    return progress;
}

WordsAndTimes time_per_word(const std::vector<std::string>& words,
                            const std::vector<std::vector<double>>& timestamps_per_player) {
    WordsAndTimes result{words, {}};
    result.times.reserve(timestamps_per_player.size());
    for (const auto& timestamps : timestamps_per_player) {
        if (timestamps.size() != words.size() + 1) {
            throw std::invalid_argument("Each player needs one start time and one timestamp per word");
        }
        std::vector<double> durations;
        durations.reserve(words.size());
        for (std::size_t index = 1; index < timestamps.size(); ++index) {
            durations.push_back(timestamps[index] - timestamps[index - 1]);
        }
        result.times.push_back(std::move(durations));
    }
    return result;
}

double get_time(const WordsAndTimes& words_and_times,
                std::size_t player_num,
                std::size_t word_index) {
    validate_words_and_times(words_and_times);
    if (player_num >= words_and_times.times.size() || word_index >= words_and_times.words.size()) {
        throw std::out_of_range("player or word index is out of range");
    }
    return words_and_times.times[player_num][word_index];
}

std::vector<std::vector<std::string>> fastest_words(const WordsAndTimes& words_and_times) {
    validate_words_and_times(words_and_times);
    std::vector<std::vector<std::string>> result(words_and_times.times.size());
    if (words_and_times.times.empty() && !words_and_times.words.empty()) {
        throw std::invalid_argument("At least one player is required when words are present");
    }

    for (std::size_t word = 0; word < words_and_times.words.size(); ++word) {
        std::size_t fastest_player = 0;
        for (std::size_t player = 1; player < words_and_times.times.size(); ++player) {
            if (words_and_times.times[player][word] < words_and_times.times[fastest_player][word]) {
                fastest_player = player;
            }
        }
        result[fastest_player].push_back(words_and_times.words[word]);
    }
    return result;
}

}  // namespace cats
