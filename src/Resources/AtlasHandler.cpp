#include "AtlasHandler.h"

#include <fstream>
#include <iostream>
#include <unordered_map>


namespace Match3 {

void AtlasHandler::Load(const std::string& filename) {
    json configuration;
    {
        std::ifstream atlasConfigStream((currentDirectory_ / "atlas" / (filename + ".json")).generic_string());
        atlasConfigStream >> configuration;
    }

    std::unordered_map<std::string, SDL_Rect> rects;
    {
        json images = configuration["images"];

        for (auto it = images.begin(); it != images.end(); ++it) {
            auto imageName = it.key();
            json data = it.value()["rect"];

            rects[imageName] = SDL_Rect{ data["x"], data["y"], data["w"], data["h"] };
        }
    }

    SDL_Texture* texture;
    {
        SDL_Surface* surface = IMG_Load((currentDirectory_ / "atlas" / filename).generic_string().c_str());
        if (surface == nullptr) {
            std::cout << "Failed to load image atlas/" << filename << ": " << SDL_GetError() << std::endl;
            return; //TODO: place a dummy "textureNotFound" texture in the atlas map
        }

        texture = SDL_CreateTextureFromSurface(renderer_, surface);
        SDL_FreeSurface(surface);
    }

    atlas_[filename] = std::make_shared<Atlas>(Atlas{ texture, configuration, rects });
}

} // namespace Match3
