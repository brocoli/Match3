# Match-3

This is a simple match-3 project using SDL2 directly.
Built with limited time for, so it's a little rough on the
edges here and there.


# How to compile

You will need git and CMake.
Read below for instructions on project dependencies.

## Dependencies

### Catch2

You need to install Catch2 on your own.
The way I did it was by using the git repository method:
`https://github.com/catchorg/Catch2/blob/master/docs/cmake-integration.md#installing-catch2-from-git-repository`

### nlohmann_json

nlohmann_json is included as a git submodule. Make sure to
`git submodule update --init --recursive`. The CMakeLists
provided will build it and include its single header.

### SDL2 and SDL_Image

This project also depends on **SDL2**, and **SDL2_image**.
Instructions vary per operating system:

#### Windows

The development libraries must be placed in their respective
folders inside `external-dynamic-dependencies`.

For convenience I made a branch `windows` that has them in
the correct place already.

If you need to download them, here are the download links
to the specific versions I used:

* SDL2: `https://www.libsdl.org/release/SDL2-devel-2.0.9-VC.zip`
* SDL2_image: `https://www.libsdl.org/projects/SDL_image/release/SDL2_image-2.0.4-win32-x64.zip`

#### Mac OSX

Download the development library installers, and install
them.

* SDL2: https://www.libsdl.org/release/SDL2-2.0.9.dmg
* SDL2_image: `https://www.libsdl.org/projects/SDL_image/release/SDL2_image-2.0.4-win32-x64.zip`

NB: Mac OSX build is not tested/supported yet.

#### Linux

Install the dependencies from your package manager.

NB: Linux build is not tested/supported yet.
