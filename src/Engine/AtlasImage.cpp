#include "AtlasImage.h"

#include "Engine/Engine.h"
#include "Engine/MessageBus.h"


namespace Match3 {

extern Engine* _engine_;
extern MessageBus* _messageBus_;

AtlasImage::AtlasImage(const std::string& atlasName, const std::string& imageName, int x, int y, bool visible) :
    srcRect_(),
    dstRect_(),
    visible_(visible)
{
    atlas_ = _engine_->GetResources()->GetAtlasHandler()->Get(atlasName);
    auto rects = atlas_->rects_;
    srcRect_ = SDL_Rect(rects[imageName]);

    dstRect_.x = x;
    dstRect_.y = y;
    dstRect_.w = srcRect_.w;
    dstRect_.h = srcRect_.h;
}

void AtlasImage::Render(SDL_Renderer* renderer) {
    if (visible_) {
        SDL_RenderCopy(renderer, atlas_->texture_, &srcRect_, &dstRect_);
    }
}

void AtlasImage::SetImage(const std::string& imageName) {
    auto rects = atlas_->rects_;
    srcRect_ = SDL_Rect(rects[imageName]);
}


} // namespace Match3
