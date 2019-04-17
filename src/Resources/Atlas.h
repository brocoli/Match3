#pragma once

#include "SDL.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace Match3 {

struct Atlas {
    SDL_Texture* texture_ = nullptr;
    json configuration_;
    std::unordered_map<std::string, SDL_Rect> rects_;
};

} // namespace Match3