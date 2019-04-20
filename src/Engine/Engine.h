#pragma once

#ifdef __APPLE__
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif
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

    Engine(const fs::path& currentDirectory, unsigned int randomSeed);
    ~Engine();

    void Run();

    FinishState GetFinishState() { return finishState_; }
    const fs::path& GetCurrentDirectory() const { return currentDirectory_; }
    std::shared_ptr<Resources> GetResources() { return resources_; }
    std::mt19937& GetRandomGenerator() { return randomGenerator_; }
    const json& GetEngineConfiguration() const { return engineConfig_; }
    const json& GetWindowConfiguration() const { return engineConfig_["window"]; }

    void InsertRenderable(std::shared_ptr<Renderable> renderable) { renderables_.push_back(renderable); }
    void ToFrontRenderable(std::shared_ptr<Renderable> renderable);

private:
    bool processInput();
    void render();

    fs::path currentDirectory_;
    std::mt19937 randomGenerator_;
    json engineConfig_;

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    std::shared_ptr<Resources> resources_;

    FinishState finishState_ = Nothing;

    std::list<std::shared_ptr<Renderable>> renderables_;

    bool mouseLeftButtonIsDown_;
    int mouseInputStartX_;
    int mouseInputStartY_;
};


} // namespace Match3
