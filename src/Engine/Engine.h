#pragma once

#include <filesystem>
#include <random>

#include <SDL.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "Renderable.h"
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

    Engine(const std::filesystem::path& currentDirectory, unsigned int randomSeed);
    ~Engine();

    void Run();

    FinishState GetFinishState() { return finishState_; }
    const std::filesystem::path& GetCurrentDirectory() const { return currentDirectory_; }
    std::shared_ptr<Resources> GetResources() { return resources_; }
    std::mt19937& GetRandomGenerator() { return randomGenerator_; }
    const json& GetWindowConfiguration() const { return engineConfig_["window"]; }

    void InsertRenderable(std::shared_ptr<Renderable> renderable) { renderables_.push_back(renderable); }

private:
    bool processInput();
    void render();

    std::filesystem::path currentDirectory_;
    std::mt19937 randomGenerator_;
    json engineConfig_;

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    std::shared_ptr<Resources> resources_;

    std::list<std::shared_ptr<Renderable>> renderables_{};

    FinishState finishState_ = Nothing;
};


} // namespace Match3
