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
#include "Resources/Resources.h"


#define QUIT_APPLICATION true
#define KEEP_RUNNING false


namespace Match3 {

extern MessageBus* _messageBus_;

Engine::Engine(const std::filesystem::path& currentDirectory, unsigned int randomSeed) :
    currentDirectory_(currentDirectory),
    randomGenerator_(std::mt19937(randomSeed))
{
    {
        std::ifstream engineConfigStream(currentDirectory_ / "config" / "engine.json");
        engineConfigStream >> engineConfig_;
    }


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

    json windowConfig = engineConfig_["window"];
    window_ = SDL_CreateWindow(
        "Match3",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        windowConfig["width"], windowConfig["height"],
        SDL_WINDOW_ALLOW_HIGHDPI
    );

    if (window_ == nullptr) {
        std::cout << "Failed to create window: " << SDL_GetError() << std::endl;
        finishState_ = InitializationError;
        return;
    }


    // Create renderer //

    renderer_ = SDL_CreateRenderer(window_, -1, 0);

    if (renderer_ == nullptr) {
        std::cout << "Failed to create renderer: " << SDL_GetError() << std::endl;
        finishState_ = InitializationError;
        return;
    }

    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);


    // Create resources handler //

    resources_ = std::make_shared<Resources>(currentDirectory, renderer_);
}

Engine::~Engine() {
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
    IMG_Quit();
    SDL_Quit();
}

void Engine::Run() {
    unsigned int mainLoopStart = 0;
    unsigned int nextFrame = 0;
    unsigned int mainLoopEnd = 0;
    const unsigned int msPerFrame = engineConfig_["msPerFrame"];

    _messageBus_->Notify("/Engine/Started");

    while (true) {
        mainLoopStart = SDL_GetTicks();
        nextFrame = mainLoopStart + msPerFrame;

        if (processInput() == QUIT_APPLICATION) {
            break;
        }
        _messageBus_->Notify("/Engine/Tick");
        render();

        mainLoopEnd = SDL_GetTicks();
        if (nextFrame > mainLoopEnd) {
            SDL_Delay(std::min(nextFrame - mainLoopEnd, msPerFrame));
        }
    }

    finishState_ = WindowClosed;
    _messageBus_->Notify("/Engine/Stopped");
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

void Engine::render() {
    SDL_RenderClear(renderer_);

    for each (auto renderable in renderables_) {
        renderable->Render(renderer_);
    }

    SDL_RenderPresent(renderer_);
}

} // namespace Match3
