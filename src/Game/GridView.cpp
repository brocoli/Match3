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

GridView::GridView() : tileImageByGridPosition_(0, 0, nullptr) {
    json configuration;
    {
        std::ifstream configurationStream(_engine_->GetCurrentDirectory() / "config" / "grid.json");
        configurationStream >> configuration;
    }

    size_t cellsX = configuration["cellsX"];
    size_t cellsY = configuration["cellsY"];

    json layout = configuration["layout"];

    layoutCellWidth_ = layout["cellWidth"];
    layoutCellHeight_ = layout["cellHeight"];
    layoutCellMarginX_ = layout["cellMarginX"];
    layoutCellMarginY_ = layout["cellMarginY"];

    tileImageNames_ = configuration["cellNames"];

    const json& window = _engine_->GetWindowConfiguration();

    layoutOffsetX_ = (window["width"] - (layoutCellWidth_ + layoutCellMarginX_) * (int)cellsX) / 2;
    layoutOffsetY_ = (window["height"] - (layoutCellHeight_ + layoutCellMarginY_) * (int)cellsY) / 2;

    size_t totalCells = cellsX * cellsY;
    for (size_t i = 0; i < totalCells; ++i) {
        std::shared_ptr<Cell> img = std::make_shared<Cell>(
            "Pieces.png", "Color-1",
            0, 0,
            false
        );
        _engine_->InsertRenderable(img);
        tilePool_.emplace(img);
    }

    tileImageByGridPosition_ = Util::Array2D<std::shared_ptr<Cell>>(cellsY, cellsX, nullptr);

    onGridModelChanged_ = std::make_shared<MessageBus::Callback>(
        [this](const MessageBus::Key&, MessageBus::Data gridDelta) -> void {
            reifyGridDelta(gridDelta);
        }
    );

    _messageBus_->Attach("/Game/Grid/Model/Changed", onGridModelChanged_);
}

Int2D GridView::calculateXYFromCoordinates(size_t j, size_t i) {
    int x = ((layoutCellWidth_ + layoutCellMarginX_) * (int)j) + layoutOffsetX_;
    int y = ((layoutCellHeight_ + layoutCellMarginY_) * (int)i) + layoutOffsetY_;

    return std::pair<int, int>(x, y);
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
}

} // namespace Match3
