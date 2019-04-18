#pragma once

#include <queue>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "Engine/AtlasImage.h"


namespace Match3 {

class GridView {
public:
    GridView();
private:
    json configuration_;

    class ImagePoolCompare {
    public:
        bool operator() (const std::shared_ptr<const AtlasImage> lhs, const std::shared_ptr<const AtlasImage> rhs) {
            bool isLhsVisible = lhs->IsVisible();

            if (isLhsVisible == rhs->IsVisible()) {
                return lhs < rhs;
            } else {
                return isLhsVisible;
            }
        }
    };
    std::priority_queue<std::shared_ptr<AtlasImage>, std::vector<std::shared_ptr<AtlasImage>>, ImagePoolCompare> tilePool_;
};

} // namespace Match3
