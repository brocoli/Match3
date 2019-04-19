#pragma once

#include <memory>
#ifdef APPLE
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

#include "AtlasHandler.h"


namespace Match3 {

class Resources {
public:
    Resources(const fs::path& currentDirectory, SDL_Renderer* renderer) {
        atlasHandler_ = std::make_shared<AtlasHandler>(AtlasHandler(currentDirectory, renderer));
    }

    std::shared_ptr<AtlasHandler> GetAtlasHandler() { return atlasHandler_; }

private:
    std::shared_ptr<AtlasHandler> atlasHandler_;
};

} // namespace Match3
