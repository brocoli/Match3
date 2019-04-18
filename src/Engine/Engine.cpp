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


bool constexpr QUIT_APPLICATION = true;
bool constexpr KEEP_RUNNING = false;


namespace Match3 {

extern MessageBus* _messageBus_;

Engine::Engine(const std::filesystem::path& currentDirectory, unsigned int randomSeed) :
    currentDirectory_(currentDirectory),
    randomGenerator_(std::mt19937(randomSeed)),
    mouseLeftButtonIsDown_(false),
    mouseInputStartX_(-1),
    mouseInputStartY_(-1)
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

void Engine::ToFrontRenderable(std::shared_ptr<Renderable> renderable) {
    renderables_.remove(renderable);
    renderables_.push_back(renderable);
}

bool Engine::processInput() {
    SDL_Event windowEvent;

    while (SDL_PollEvent(&windowEvent) != 0) {
        switch (windowEvent.type) {
            case SDL_QUIT: {
                return QUIT_APPLICATION;
            } break;
            case SDL_MOUSEBUTTONDOWN: {
                auto event = (SDL_MouseButtonEvent*)(&windowEvent);
                if (event->button == SDL_BUTTON_LEFT) {
                    mouseInputStartX_ = event->x;
                    mouseInputStartY_ = event->y;
                    mouseLeftButtonIsDown_ = true;

                    std::shared_ptr<json> data = std::make_shared<json>(json{
                        { "x", mouseInputStartX_ },
                        { "y", mouseInputStartY_ },
                        { "initialX", mouseInputStartX_ },
                        { "initialY", mouseInputStartY_ },
                        { "phase", "started" },
                    });

                    _messageBus_->Notify("/Engine/Input/Mouse", data);
                }
            } break;
            case SDL_MOUSEBUTTONUP: {
                auto event = (SDL_MouseButtonEvent*)(&windowEvent);
                if (event->button == SDL_BUTTON_LEFT) {
                    std::shared_ptr<json> data = std::make_shared<json>(json{
                        { "x", event->x },
                        { "y", event->y },
                        { "initialX", mouseInputStartX_ },
                        { "initialY", mouseInputStartY_ },
                        { "phase", "stopped" },
                    });

                    mouseInputStartX_ = -1;
                    mouseInputStartY_ = -1;
                    mouseLeftButtonIsDown_ = false;

                    _messageBus_->Notify("/Engine/Input/Mouse", data);
                }
            } break;
            case SDL_MOUSEMOTION: {
                if (mouseLeftButtonIsDown_) {
                    auto event = (SDL_MouseMotionEvent*)(&windowEvent);
                    std::shared_ptr<json> data = std::make_shared<json>(json{
                        { "x", event->x },
                        { "y", event->y },
                        { "initialX", mouseInputStartX_ },
                        { "initialY", mouseInputStartY_ },
                        { "phase", "moved" },
                    });
                    _messageBus_->Notify("/Engine/Input/Mouse", data);
                }
            } break;
            default: {
            } break;
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
