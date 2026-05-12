# Building Octal Engine

## Requirements
- C++20
- CMake3.20

## Building
Setup project with CMake:
```bash
mkdir build
cmake -S . -B build
```
it'll produce a `build` directory with all the necessary files to build the engine.

for release build, add `-DCMAKE_BUILD_TYPE=Release` to the CMake command:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```
for debug build, add `-DCMAKE_BUILD_TYPE=Debug` to the CMake command:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```

Then build with:
```bash
cmake --build build
```
it'll produce the `Sandbox` executable and `OctalEngine.lib` in the `build` directory.