#pragma once

#include "Renderable.h"

#include <memory>

#include "SDL.h"

#include "Resources/Atlas.h"


namespace Match3 {

class AtlasImage : public Renderable {
public:
    AtlasImage(const std::string& atlasName, const std::string& imageName, int x, int y, bool visible);

    virtual void Render(SDL_Renderer* renderer);

    void SetToImage(const std::string& imageName);
    void SetVisible(bool visible) { visible_ = visible; };

    SDL_Rect& GetSrcRect() { return srcRect_; }
    SDL_Rect& GetDstRect() { return dstRect_; }

private:
    std::shared_ptr<const Atlas> atlas_;
    SDL_Rect srcRect_;
    SDL_Rect dstRect_;

    bool visible_;
};

} // namespace Match3
