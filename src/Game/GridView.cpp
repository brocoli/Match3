#include "GridView.h"

#include <fstream>
#include <random>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "Engine/AtlasImage.h"
#include "Engine/Engine.h"
#include "Engine/MessageBus.h"


namespace Match3 {

extern MessageBus* _messageBus_;
extern Engine* _engine_;

GridView::GridView() : tileImageByGridPosition_(0, 0, nullptr), heldTile_(nullptr) {
    json configuration;
    {
        std::ifstream configurationStream(_engine_->GetCurrentDirectory() / "config" / "grid.json");
        configurationStream >> configuration;
    }

    cellsX_ = configuration["cellsX"];
    cellsY_ = configuration["cellsY"];

    json layout = configuration["layout"];

    layoutCellWidth_ = layout["cellWidth"];
    layoutCellHeight_ = layout["cellHeight"];
    layoutCellMarginX_ = layout["cellMarginX"];
    layoutCellMarginY_ = layout["cellMarginY"];

    tileImageNames_ = configuration["cellNames"];

    const json& window = _engine_->GetWindowConfiguration();

    layoutOffsetX_ = (window["width"] - (layoutCellWidth_ + layoutCellMarginX_) * (int)cellsX_) / 2;
    layoutOffsetY_ = (window["height"] - (layoutCellHeight_ + layoutCellMarginY_) * (int)cellsY_) / 2;

    size_t totalCells = cellsX_ * cellsY_;
    for (size_t i = 0; i < totalCells; ++i) {
        std::shared_ptr<Cell> img = std::make_shared<Cell>(
            "Pieces.png", tileImageNames_[0],
            0, 0,
            false
        );
        _engine_->InsertRenderable(img);
        tilePool_.emplace(img);
    }

    tileImageByGridPosition_ = Util::Array2D<std::shared_ptr<Cell>>(cellsY_, cellsX_, nullptr);


    onGridModelChanged_ = std::make_shared<MessageBus::Callback>(
        [this](const MessageBus::Key&, MessageBus::Data gridDelta) -> void {
            reifyGridDelta(gridDelta);
        }
    );
    _messageBus_->Attach("/Game/Grid/Model/Changed", onGridModelChanged_);


    onMouseLeftStarted_ = std::make_shared<MessageBus::Callback>(
        [this](const MessageBus::Key&, MessageBus::Data mouseEvent) -> void {
            pickUpTileByPosition((*mouseEvent)["x"], (*mouseEvent)["y"]);
        }
    );
    _messageBus_->Attach("/Engine/Input/Mouse/Left/Started", onMouseLeftStarted_);

    onMouseLeftMoved_ = std::make_shared<MessageBus::Callback>(
        [this](const MessageBus::Key&, MessageBus::Data mouseEvent) -> void {
            dragHeldTile((*mouseEvent)["x"], (*mouseEvent)["y"], (*mouseEvent)["initialX"], (*mouseEvent)["initialY"]);
        }
    );
    _messageBus_->Attach("/Engine/Input/Mouse/Left/Moved", onMouseLeftMoved_);

    onMouseLeftStopped_ = std::make_shared<MessageBus::Callback>(
        [this](const MessageBus::Key&, MessageBus::Data mouseEvent) -> void {
            releaseHeldTile((*mouseEvent)["x"], (*mouseEvent)["y"], (*mouseEvent)["initialX"], (*mouseEvent)["initialY"]);
        }
    );
    _messageBus_->Attach("/Engine/Input/Mouse/Left/Stopped", onMouseLeftStopped_);
}

GridView::~GridView() {
    _messageBus_->Detach("/Game/Grid/Model/Changed", onGridModelChanged_);

    _messageBus_->Detach("/Engine/Input/Mouse/Left/Moved", onMouseLeftMoved_);
    _messageBus_->Detach("/Engine/Input/Mouse/Left/Stopped", onMouseLeftStopped_);
    _messageBus_->Detach("/Engine/Input/Mouse/Left/Started", onMouseLeftStarted_);
}

Int2D GridView::calculateXYFromCoordinates(size_t j, size_t i) {
    int x = ((layoutCellWidth_ + layoutCellMarginX_) * (int)j) + layoutOffsetX_;
    int y = ((layoutCellHeight_ + layoutCellMarginY_) * (int)i) + layoutOffsetY_;

    return Int2D(x, y);
}

std::optional<Size2D> GridView::calculateCoordinatesFromXY(int x, int y) {
    x -= layoutOffsetX_;
    y -= layoutOffsetY_;

    int j = x / (layoutCellWidth_ + layoutCellMarginX_);
    int i = y / (layoutCellHeight_ + layoutCellMarginY_);

    if (j < 0 || cellsX_ <= j || i < 0 || cellsY_ <= i) {
        // index out of range
        return std::nullopt;
    }

    x %= layoutCellWidth_ + layoutCellMarginX_;
    y %= layoutCellHeight_ + layoutCellMarginY_;

    if (x >= layoutCellWidth_ || y >= layoutCellHeight_) {
        // position in margin between tiles
        return std::nullopt;
    }

    return std::optional<Size2D>({(size_t)j, (size_t)i});
}

void GridView::reifyGridDelta(MessageBus::Data gridDelta) {
    for each (json delta in (*gridDelta)) {
        if (delta["command"] == "fill") {
            for each (json tile in delta["tiles"]) {
                fillTile(tile["x"], tile["y"], tile["value"]);
            }
        }
    }
}

void GridView::fillTile(size_t j, size_t i, size_t tileType) {
    std::shared_ptr<Cell> tile = tilePool_.top();
    tilePool_.pop();

    tile->SetImage(tileImageNames_[tileType]);
    tile->SetXY(calculateXYFromCoordinates(j, i));
    tile->SetVisible(true);

    tileImageByGridPosition_[i][j] = tile;
}

void GridView::pickUpTileByPosition(int x, int y) {
    std::optional<Size2D> maybeCoords = calculateCoordinatesFromXY(x, y);

    if (maybeCoords.has_value()) {
        Size2D coords = maybeCoords.value();
        size_t j = coords.first;
        size_t i = coords.second;

        heldTile_ = tileImageByGridPosition_[i][j];
        _engine_->ToFrontRenderable(heldTile_);

        dragHeldTile(x, y, x, y);
    }
}

void GridView::dragHeldTile(int x, int y, int initialX, int initialY) {
    if (heldTile_ != nullptr) {
        heldTile_->SetXY(Int2D{ x, y });
    }
}

void GridView::releaseHeldTile(int x, int y, int initialX, int initialY) {
    if (heldTile_ != nullptr) {
        dragHeldTile(x, y, initialX, initialY);
        heldTile_ = nullptr;
    }
}

} // namespace Match3
