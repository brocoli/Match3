#pragma once

#include <queue>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "Util/Array2D.h"
#include "Engine/AtlasImage.h"


namespace Match3 {

class GridView {
public:
    using Cell = AtlasImage;

    GridView();
    void FillGrid();
private:
    Int2D calculateXYFromCoordinates(size_t j, size_t i);

    size_t layoutCellsX_;
    size_t layoutCellsY_;
    int layoutCellWidth_;
    int layoutCellHeight_;
    int layoutCellMarginX_;
    int layoutCellMarginY_;

    int layoutOffsetX_;
    int layoutOffsetY_;

    class ImagePoolCompare {
    public:
        bool operator() (const std::shared_ptr<const Cell> lhs, const std::shared_ptr<const Cell> rhs) {
            bool isLhsVisible = lhs->IsVisible();

            if (isLhsVisible == rhs->IsVisible()) {
                return lhs < rhs;
            } else {
                return isLhsVisible;
            }
        }
    };
    std::priority_queue<std::shared_ptr<Cell>, std::vector<std::shared_ptr<Cell>>, ImagePoolCompare> tilePool_;

    Util::Array2D<std::shared_ptr<Cell>> tileImageByGridPosition_;
};

} // namespace Match3
