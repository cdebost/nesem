# nesem

## Requirements

* C++20 compiler
* CMake
* Conan (`pip install conan`)

## Building

Install dependencies: `conan install . -if=build --build=missing`

Init cmake: `cd build && cmake ..`

Build: `cd build && cmake --build .`
