# Cats — C++17 port

This is a native C++ rebuild of the CS61A Cats typing project. The original
Python coursework remains in the repository; the C++ implementation lives in
`cats.hpp`, `cats.cpp`, and `main.cpp`.

The library includes paragraph selection, topic matching, accuracy and WPM,
autocorrect, bounded edit distance, progress reporting, per-word timing, and
fastest-player calculation. `final_diff` additionally recognizes adjacent
character transpositions.

## Build and test

With CMake and any C++17 compiler:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

On a single-config generator, run the typing test with:

```sh
./build/cats_cpp -t
./build/cats_cpp -t cats dogs
```

On Visual Studio generators the executable is usually under
`build/Debug/cats_cpp.exe` (or the selected configuration directory).

Run `cats_cpp --help` for command-line help.
