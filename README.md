# Match-3

This is a simple match-3 project using SDL2 directly.
Originally built for an interview test.


# How to compile

Use CMake and generate makefiles / solutions for your
preferred compiler / build system.

## Dependencies

### nlohmann_json

nlohmann_json is included as a git submodule. Make sure to
`git submodule update --init --recursive`. The CMakeLists
provided will build it and copy the dynamic library to the
executable's folder.

### SDL2 and its modules

This project also depends on **SDL2**, **SDL2_image**, **SDL2_ttf** and
**SDL2_mixer**. Download them and place the contents of their
respective development libraries into their folders inside
`external-dynamic-dependencies`. They should have an
`include` folder with the `.h` inside, and a `lib` folder
with the dynamically linked libraries inside.


# Licenses

Licences to the above described 3rd-party software are
included with their source code whenever appropriate, or
copied alongside the compiled library files otherwise.
