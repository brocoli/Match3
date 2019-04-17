#pragma once

#include <filesystem>

#include <SDL.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "Resources/Resources.h"


namespace Match3 {

class Engine {
public:
    enum FinishState {
        Nothing,
        InitializationError,
        ResourceLoadingError,
        WindowClosed,
    };

    Engine(const std::filesystem::path& currentDirectory);
    ~Engine();

    void Run();
    FinishState GetFinishState() { return finishState_; }

    Resources* GetResources() { return resources_; }

private:
    bool processInput();
    void update();
    void render();

    std::filesystem::path currentDirectory_;
    json engineConfig_;

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    Resources* resources_ = nullptr;

    FinishState finishState_ = Nothing;
};


} // namespace Match3
