#pragma once

#include <memory>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "Engine/MessageBus.h"

#include "GridView.h"

namespace Match3 {

class Game {
public:
    Game();
    ~Game();

private:
    void loadAllGameTextures();
    void insertBackgroundImage();
    void createGridView();

    MessageBus::CallbackPtr onEngineStarted_;
    std::shared_ptr<GridView> gridView_;
};

} // namespace Match3
