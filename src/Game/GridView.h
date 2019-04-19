#pragma once

#include <deque>
#include <forward_list>
#include <functional>
#include <optional>
#include <set>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "Util/Array2D.h"
#include "Engine/AtlasImage.h"
#include "Engine/MessageBus.h"
#include "Engine/TweenRunner.h"


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

    void enqueueActionLogDelta(MessageBus::Data actionLogDelta);
    void enactActionLogDelta();

    void fillTile(size_t j, size_t i, size_t value);
    void emptyTile(size_t j, size_t i);
    void swapTiles(size_t j, size_t i, size_t oj, size_t oi);
    void returnTiles(size_t j, size_t i, size_t oj, size_t oi);
    void mergeTiles(std::list<std::list<std::array<size_t, 2>>> groups);
    void dropTiles(std::vector<std::array<size_t, 2>> tilesFallen, std::vector<std::array<size_t, 2>> tileGrounds);

    void pickUpTileByPosition(int x, int y);
    void dragHeldTile(int x, int y, int initialX, int initialY);
    void releaseHeldTile(int x, int y, int initialX, int initialY);

    MessageBus::CallbackPtr onGridActionLogChanged_;

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

    std::forward_list<std::shared_ptr<Cell>> tilePool_;

    Util::Array2D<std::shared_ptr<Cell>> tileImageByCoordinates_;

    std::shared_ptr<Cell> heldTile_;
    Int2D heldTileOriginalPosition_;
    Size2D heldTileOriginalCoordinates_;

    std::shared_ptr<Cell> interactedTile_;
    Int2D interactedTileOriginalPosition_;
    Size2D interactedTileOriginalCoordinates_;

    Int2D currentDragDelta_;
    Int2D dragMovementVelocity_;

    int inputSensitivityPosition_;
    int inputSensitivityVelocityFactor_;

    int tileMovementVelocity_;

    bool busy_;
    std::deque<json> pendingActionLogDeltas_;

    TweenRunner<int> tweenRunner_;

    int tweenCountTemp_;
    std::list<std::list<std::array<size_t, 2>>> groupsToMergeTempCopy_;
};

} // namespace Match3
