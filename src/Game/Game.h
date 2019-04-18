#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "Engine/MessageBus.h"


namespace Match3 {

class Game {
public:
    Game();
    ~Game();

    void loadAllGameTextures();
    void insertBackgroundImage();

private:
    MessageBus::CallbackPtr onEngineStarted_;

};

} // namespace Match3
