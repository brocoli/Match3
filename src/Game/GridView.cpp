#include "GridView.h"

#include <fstream>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "Engine/AtlasImage.h"
#include "Engine/Engine.h"


namespace Match3 {

extern Engine* _engine_;

GridView::GridView() {
    json configuration_;
    {
        std::ifstream configurationStream(_engine_->GetCurrentDirectory() / "config" / "grid.json");
        configurationStream >> configuration_;
    }

    json layout = configuration_["layout"];

    int cellsX = layout["cellsX"];
    int cellsY = layout["cellsY"];
        //"cellWidth" : 70,
        //"cellHeight" : 70,
        //"cellMarginX" : 4,
        //"cellMarginY" : 4

    int totalCells = cellsX * cellsY;
    for (int i = 0; i < totalCells; ++i) {
        std::shared_ptr<AtlasImage> img = std::make_shared<AtlasImage>(
            "Pieces.png", "Color-1",
            0, 0,
            false
        );
        _engine_->InsertRenderable(img);
        tilePool_.emplace(img);
    }
}


} // namespace Match3
