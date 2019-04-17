#include "Engine.h"

#include <fstream>
#include <iostream>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "MessageBus.h"


#define QUIT_APPLICATION true
#define KEEP_RUNNING false


namespace Match3 {

extern MessageBus* _messageBus_;

Engine::Engine(const std::filesystem::path& currentDirectory) {
    currentDirectory_ = currentDirectory;

    {
        std::ifstream engineConfigStream(currentDirectory_ / "config" / "engine.json");
        engineConfigStream >> engineConfig_;
    }
    windowConfig_ = engineConfig_["window"];


    // Initialize SDL //

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cout << "Failed to init SDL2: " << SDL_GetError() << std::endl;
        finishState_ = InitializationError;
        return;
    }

    int sdlImageInitFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if ((IMG_Init(sdlImageInitFlags) & sdlImageInitFlags) != sdlImageInitFlags) {
        std::cout << "Failed to init SDL2_image: " << IMG_GetError() << std::endl;
        finishState_ = InitializationError;
        return;
    }


    // Create window //

    window_ = SDL_CreateWindow(
        "Match3",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        windowConfig_["width"], windowConfig_["height"],
        SDL_WINDOW_ALLOW_HIGHDPI
    );

    if (window_ == nullptr) {
        std::cout << "Failed to create window: " << SDL_GetError() << std::endl;
        finishState_ = InitializationError;
        return;
    }

    _messageBus_->Notify(MessageBus::Key::_Engine_Started);


    // TEST //
    backgroundSurface_ = IMG_Load((currentDirectory_ / "img" / "Backdrop13.jpg").generic_string().c_str());

    if (backgroundSurface_ == nullptr) {
        std::cout << "Failed to load texture img/Backdrop13.jpg: " << SDL_GetError() << std::endl;
        finishState_ = ResourceLoadingError;
        return;
    }
    // /TEST //
}

Engine::~Engine() {
    // TEST //
    SDL_FreeSurface(backgroundSurface_);
    // /TEST //

    SDL_DestroyWindow(window_);
    IMG_Quit();
    SDL_Quit();
}

void Engine::Run() {
    unsigned int mainLoopStart = 0;
    unsigned int nextFrame = 0;
    unsigned int mainLoopEnd = 0;
    const unsigned int msPerFrame = engineConfig_["msPerFrame"];

    while (true) {
        mainLoopStart = SDL_GetTicks();
        nextFrame = mainLoopStart + msPerFrame;

        if (processInput() == QUIT_APPLICATION) {
            break;
        }
        update();
        render();

        mainLoopEnd = SDL_GetTicks();
        if (nextFrame > mainLoopEnd) {
            SDL_Delay(std::min(nextFrame - mainLoopEnd, msPerFrame));
        }
    }

    finishState_ = WindowClosed;
}

bool Engine::processInput() {
    SDL_Event windowEvent;

    if (SDL_PollEvent(&windowEvent)) {
        if (windowEvent.type == SDL_QUIT) {
            return QUIT_APPLICATION;
        }
    }

    return KEEP_RUNNING;
}

void Engine::update() {
}

void Engine::render() {
    // TEST //
    SDL_Surface*const windowSurface = SDL_GetWindowSurface(window_);

    SDL_BlitSurface(backgroundSurface_, nullptr, windowSurface, nullptr);
    SDL_UpdateWindowSurface(window_);
    // /TEST //
}

} // namespace Match3
