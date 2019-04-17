#pragma once

#include "AtlasHandler.h"

#include <memory>
#include <filesystem>


namespace Match3 {

class Resources {
public:
    Resources(const std::filesystem::path& currentDirectory, SDL_Renderer* renderer) {
        atlasHandler_ = std::make_shared<AtlasHandler>(AtlasHandler(currentDirectory, renderer));
    }

    std::shared_ptr<AtlasHandler> GetAtlasHandler() { return atlasHandler_; }

private:
    std::shared_ptr<AtlasHandler> atlasHandler_;
};

} // namespace Match3
