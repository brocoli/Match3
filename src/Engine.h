#pragma once

#include <filesystem>

#include <SDL.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;


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

private:
    bool processInput();
    void update();
    void render();

    FinishState finishState_ = Nothing;

    json engineConfig_;
    json windowConfig_;

    std::filesystem::path currentDirectory_;

    SDL_Window* window_ = nullptr;

    // TEST //
    SDL_Surface* backgroundSurface_ = nullptr;
    // /TEST //
};


} // namespace Match3
