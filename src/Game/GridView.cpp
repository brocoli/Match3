#include "GridView.h"

#include <fstream>
#include <random>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "Engine/AtlasImage.h"
#include "Engine/Engine.h"


namespace Match3 {

extern Engine* _engine_;

static const std::vector<std::string> tileImageNames{
    "Color-1",
    "Color-2",
    "Color-3",
    "Color-4",
    "Color-5",
};

GridView::GridView() : tileImageByGridPosition_(0, 0) {
    json configuration;
    {
        std::ifstream configurationStream(_engine_->GetCurrentDirectory() / "config" / "grid.json");
        configurationStream >> configuration;
    }

    json layout = configuration["layout"];

    layoutCellsX_ = layout["cellsX"];
    layoutCellsY_ = layout["cellsY"];
    layoutCellWidth_ = layout["cellWidth"];
    layoutCellHeight_ = layout["cellHeight"];
    layoutCellMarginX_ = layout["cellMarginX"];
    layoutCellMarginY_ = layout["cellMarginY"];

    const json& window = _engine_->GetWindowConfiguration();

    layoutOffsetX_ = (window["width"] - (layoutCellWidth_ + layoutCellMarginX_) * (int)layoutCellsX_) / 2;
    layoutOffsetY_ = (window["height"] - (layoutCellHeight_ + layoutCellMarginY_) * (int)layoutCellsY_) / 2;

    size_t totalCells = layoutCellsX_ * layoutCellsY_;
    for (size_t i = 0; i < totalCells; ++i) {
        std::shared_ptr<Cell> img = std::make_shared<Cell>(
            "Pieces.png", "Color-1",
            0, 0,
            false
        );
        _engine_->InsertRenderable(img);
        tilePool_.emplace(img);
    }

    tileImageByGridPosition_ = Util::Array2D<std::shared_ptr<Cell>>(layoutCellsY_, layoutCellsX_);

    for (size_t i = 0; i < layoutCellsY_; ++i) {
        for (size_t j = 0; j < layoutCellsX_; ++j) {
            tileImageByGridPosition_[i][j] = nullptr;
        }
    }
}

// TODO: this is not a View function. Move to logic.
void GridView::FillGrid() {
    std::uniform_int_distribution<size_t> randomDistribution(0, tileImageNames.size() - 1);
    std::mt19937& randomGenerator = _engine_->GetRandomGenerator();

    for (size_t i = 0; i < layoutCellsY_; ++i) {
        for (size_t j = 0; j < layoutCellsX_; ++j) {
            if (tileImageByGridPosition_[i][j] == nullptr) {
                std::shared_ptr<Cell> tile = tilePool_.top();
                tilePool_.pop();

                tile->SetImage(tileImageNames[randomDistribution(randomGenerator)].c_str());
                tile->SetXY(calculateXYFromCoordinates(j, i));
                tile->SetVisible(true);

                tileImageByGridPosition_[i][j] = tile;
            }
        }
    }
}

Int2D GridView::calculateXYFromCoordinates(size_t j, size_t i) {
    int x = ((layoutCellWidth_ + layoutCellMarginX_) * (int)j) + layoutOffsetX_;
    int y = ((layoutCellHeight_ + layoutCellMarginY_) * (int)i) + layoutOffsetY_;

    return std::pair<int, int>(x, y);
}

} // namespace Match3
