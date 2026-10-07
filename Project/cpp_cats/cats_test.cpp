#include "cats.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

namespace {

bool close(double left, double right) {
    return std::abs(left - right) < 1e-9;
}

}  // namespace

int main() {
    using namespace cats;

    const std::vector<std::string> paragraphs{"hi", "how are you", "fine"};
    assert(pick(paragraphs, [](const std::string& text) { return text.size() <= 4; }, 0) == "hi");
    assert(pick(paragraphs, [](const std::string& text) { return text.size() <= 4; }, 1) == "fine");
    assert(pick(paragraphs, [](const std::string& text) { return text.size() <= 4; }, 2).empty());

    const auto about_dogs = about({"dog", "dogs", "pup", "puppy"});
    assert(about_dogs("Cute Dog!"));
    assert(!about_dogs("That is a cat."));
    assert(about_dogs("Nice pup."));

    assert(close(accuracy("Cute Dog!", "Cute Dog."), 50.0));
    assert(close(accuracy("Cute", "Cute Dog."), 100.0));
    assert(close(accuracy("", ""), 100.0));
    assert(close(wpm("0123456789", 60), 2.0));

    assert(autocorrect("hwllo", {"butter", "hello", "potato"},
                       [](const std::string&, const std::string&, int) { return 10; }, 20) == "butter");
    assert(furry_fixes("range", "rungs", 10) == 2);
    assert(furry_fixes("pill", "pillage", 10) == 3);
    assert(minimum_mewtations("cats", "scat", 10) == 2);
    assert(minimum_mewtations("purng", "purring", 10) == 2);
    assert(minimum_mewtations("ckiteus", "kittens", 10) == 3);
    assert(minimum_mewtations("abc", "", 10) == 3);
    assert(minimum_mewtations("abc", "", 2) == 3);
    assert(final_diff("teh", "the", 2) == 1);

    ProgressReport uploaded{-1, -1.0};
    const double progress = report_progress({"how", "are", "you"},
                                            {"how", "are", "you", "doing", "today"}, 2,
                                            [&](const ProgressReport& report) { uploaded = report; });
    assert(close(progress, 0.6));
    assert(uploaded.id == 2 && close(uploaded.progress, 0.6));

    const auto result = time_per_word({"collar", "plush", "blush", "repute"},
                                      {{75, 81, 84, 90, 92}, {19, 29, 35, 36, 38}});
    assert((result.times == std::vector<std::vector<double>>{{6, 3, 6, 2}, {10, 6, 1, 2}}));
    assert(close(get_time(result, 1, 2), 1.0));

    const auto fastest = fastest_words({{"Just", "have", "fun"}, {{5, 1, 3}, {4, 1, 6}}});
    assert((fastest == std::vector<std::vector<std::string>>{{"have", "fun"}, {"Just"}}));

    int calls = 0;
    auto cached = memo_diff([&](const std::string&, const std::string&, int) { return ++calls; });
    assert(cached("a", "b", 1) == 1);
    assert(cached("a", "b", 1) == 1);
    assert(calls == 1);

    std::cout << "All C++ tests passed.\n";
}
