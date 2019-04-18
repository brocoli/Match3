#pragma once

#include <functional>
#include <optional>
#include <queue>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "Util/Array2D.h"
#include "Engine/AtlasImage.h"
#include "Engine/MessageBus.h"


namespace Match3 {

using Size2D = std::pair<size_t, size_t>;

class GridView {
public:
    using Cell = AtlasImage;
    using Task = std::function<bool(void)>;

    GridView();
    ~GridView();
private:
    Int2D calculateXYFromCoordinates(size_t j, size_t i);
    std::optional<Size2D> calculateCoordinatesFromXY(int x, int y);

    void reifyGridDelta(MessageBus::Data gridDelta);

    void fillTile(size_t j, size_t i, size_t value);

    void pickUpTileByPosition(int x, int y);
    void dragHeldTile(int x, int y, int initialX, int initialY);
    void releaseHeldTile(int x, int y, int initialX, int initialY);

    MessageBus::CallbackPtr onGridModelChanged_;

    MessageBus::CallbackPtr onMouseLeftStarted_;
    MessageBus::CallbackPtr onMouseLeftStopped_;
    MessageBus::CallbackPtr onMouseLeftMoved_;

    size_t cellsX_;
    size_t cellsY_;

    int layoutCellWidth_;
    int layoutCellHeight_;
    int layoutCellMarginX_;
    int layoutCellMarginY_;

    int layoutOffsetX_;
    int layoutOffsetY_;

    json tileImageNames_;

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

    Util::Array2D<std::shared_ptr<Cell>> tileImageByCoordinates_;

    std::shared_ptr<Cell> heldTile_;
    Int2D heldTileOriginalPosition_;
    Size2D heldTileOriginalCoordinates_;

    std::shared_ptr<Cell> interactedTile_;
    Int2D interactedTileOriginalPosition_;
    Size2D interactedTileOriginalCoordinates_;

    Int2D lastDragDelta_;
    Int2D dragMovementVelocity_;
};

} // namespace Match3
