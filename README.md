# Match-3

This is a simple match-3 project using SDL2 directly.
Built with limited time for, so it's a little rough on the
edges here and there.


## How to compile

You will need git (for a submodule) and CMake, as well as
modern compilers.

On Windows, use Visual Studio 16 2019 generators.
On macOS, use the Xcode generator.

Read below for instructions on project dependencies.

### Dependencies

#### Catch2

You need to install Catch2 on your own.
The way I did it was by using the git repository method:
`https://github.com/catchorg/Catch2/blob/master/docs/cmake-integration.md#installing-catch2-from-git-repository`

#### nlohmann_json

nlohmann_json is included as a git submodule. Make sure to
`git submodule update --init --recursive`. The CMakeLists
provided will build it and include its single header.

#### SDL2 and SDL_Image

This project also depends on **SDL2**, and **SDL2_image**.
Instructions vary per operating system:

###### Windows

The development libraries must be placed in their respective
folders inside `external-dynamic-dependencies`.

For convenience I made a branch `windows` that has them in
the correct place already.

If you need to download them, here are the download links
to the specific versions I used:

* SDL2: `https://www.libsdl.org/release/SDL2-devel-2.0.9-VC.zip`
* SDL2_image: `https://www.libsdl.org/projects/SDL_image/release/SDL2_image-2.0.4-win32-x64.zip`

###### macOS

Use Homebrew. `brew install sdl2 sdl2_image`

###### Linux

Install the dependencies from your package manager.
NB: Linux build is not tested/supported yet.

#### Boost (in macOS exclusively)

Due to this program using some C++17 features and Apple
not providing them, we substitute them with the Boost
library's implementations of the same classes/functions

Again, just use Homebrew and everything should be fine.
`brew install boost`

## Code Commentary

### Directory Structure

##### root:

* **cmake/** - CMake modules for finding dependencies.
* **external-code-dependencies/** - Dependencies that are compiled in the project's build process (currently only nlohmann_json).
* **external-dynamic-dependencies/** - used by the Windows build, where package managers are not as common.
* **runtime-resources/** - Images and configuration files that are used by the executable. If we had sounds this is where they would be placed.
* **src/** - Source files
* **test/** - Unit tests

##### src/:

* **Engine/** - The "game engine" of this project. The main loop, message bus, basic object definitions and calls to SDL functions are here. Closely related to Resources/
* **Game/** - Classes that are specific to this game.
* **Resources/** - Resource handling classes -- textures, atlas. Configuration file loading and storage should be here, but right now they aren't yet.
* **Util/** - Generic utilities.
* **main.cpp** - Initialization and teardown. Declares the two globals of the application: the *Engine* and *MessageBus*.

##### src/Engine/:

* **AtlasImage.cpp** - A *Renderable* image that's part of an *Atlas*
* **Engine.cpp** - The game engine. Runs the main loop, initilizes the *Resources* managers and the *Game*.
* **MessageBus.cpp** - Central (and global) Observer pattern implementation. More on this later.
* **Renderable.cpp** - Base class for objects that can be rendered by the Engine with SDL.
* **Tween.cpp** - Object holding all of the data required for an instance of a tween task.
* **TweenRunner.cpp** - Manager that can run tweens.

##### src/Game/:

* **Game.cpp** - Loads the resource instances and instances the Grid.
* **GridLogic.cpp** - Game business rules and core mutable state. Loosely coupled with *GridView* via the *MessageBus*: its methods are mostly reactive to the user's intended actions (as parsed by the view), and they generate a kind of game replay log that the view can represent in turn. Can be viewed as a mix of a Model and a Controller in a more traditional MVC architecture.
* **GridView.cpp** - Human interface of the game grid. Moves pieces around, interprets user input events into actions, and represents the game replay log as visual changes for the user. Can be viewed as a mix of a View and a Controller in a more traditional MVC architecture.

##### src/Resources/:

* **Atlas.cpp** - Your standard texture atlas.
* **AtlasHandler.cpp** - Loader and storage class of texture atlas. In bigger projects, we use this kind of structure to guarantee that we don't load the same texture multiple times... but it's a bit superfluous here.
* **Resources.cpp** - Storage class for convenient access of resource handlers. Instead of *Engine* being a global, this should be one and *Engine* should be inaccessible... but alas.

### Unconventional Structures

#### Why the two globals?

For convenience and simplicity.

In the case of message passing, we substantial benefits from doing it in a centralized manner, and in the case of resource management, by definition you need it to be centralized.

This centralization means we're forced to have these structures available to most of the application, and using a global variable is the easiest way to achieve this.

Also, singletons are awful: implicit initialization does more harm than good, and the "upside" of forcing a single instance is irrelevant at best, harmful when you actually need multiple instances for unit testing, and Service locators are way too complex for this project.

### Why the MessageBus? Isn't conventional access faster and safer?

This can be discussed in two ways: the philosophical one and the pragmatic one.

The philosophical argument is that there's a difference in the level of dependency between two classes like i.e. the *GridView* and the *GridLogic* in comparison with i.e. the dependency that *GridLogic* has on *Array2D*. Without *Array2D* *GridLogic* cannot function. You could ask what you could test about *GridLogic* while mocking *Array2D* and not much would remain (if anything remains at all). In comparison, *GridView* can exist without *GridLogic* and vice-versa. They just provide the triggers and data for each other's "work", but these triggers could be emitted by something else, for instance a QA engineer typing commands on a console, or an UI testing framework.

Good software design practices inform us that we should map these conceptual traits and differences into our code structures, and so direct part-of and handling-of relationships should be reified with composition and conventional references, while loose dependencies should talk through an event bus.

In the practical side, notice how easy it becomes to build things like a console for controlling the game, loggers with different scopes, a replay/sequence player. How easy it becomes to isolate components to write unit tests (you pretty much only have to mock the *MessageBus*, and you can use the same mock for most classes.

## Areas of Possible Future Improvement

There are many, many things that are lacking or could be improved:

* Use an OpenGL/Vulkan/DirectX custom renderer with batching instead of SDL's renderer.
* This engine does not support relative positions, renderable object groups, rotations, convenient scaling, color manipulation, SOUND, TEXT, a framerate that's not constant fixed for the graphical side of parts, etc.. etc... It's really the bare minimum (not even enough to build an UI)
* The MessageBus is lacking the implementation of message tiers (except for the root event), which is critical to keep it organized when the project grows.
* Engine should not be a global, Resources should.
* More animations, better animations
* Doxygen or some similar documentation
* Many more unit tests. Tests for anything that requires mocking anthing.
* In that regard, having a mocking library as a dependency.
* Better pattern recognition in the Grid
* Hints and Shuffle
* Special blocks
* A more complete easing library, with easing combinators too.
* A HUD, with points or limited moves, etc...
* And definitely many, many more things...

## Thanks

Thank you for allowing me to participate in the application process and taking this test. It was interesting to use SDL alone for this (I'm more used to using OpenGL and only use SDL for the window and input events, and at the other end of the spectrum, using a fully-fledged multi-purpose game engine). It had been a while since I've built something in C++ from the ground up too.

Hopefully you'll like what you see here, and we'll continue the process. In any case, we'll keep talking =)