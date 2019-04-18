#include "GridLogic.h"

#include <fstream>

#include "Engine/Engine.h"
#include "Engine/MessageBus.h"


namespace Match3 {

extern MessageBus* _messageBus_;
extern Engine* _engine_;

GridLogic::GridLogic() : grid_(0, 0, 0) {
    json configuration;
    {
        std::ifstream configurationStream(_engine_->GetCurrentDirectory() / "config" / "grid.json");
        configurationStream >> configuration;
    }

    cellsX_ = configuration["cellsX"];
    cellsY_ = configuration["cellsY"];

    json cellNames = configuration["cellNames"];
    cellTypes_ = cellNames.size();

    grid_ = Util::Array2D<int> (cellsY_, cellsX_, EMPTY);

    initialFillGrid();
}

void GridLogic::initialFillGrid() {
    std::uniform_int_distribution<int> randomDistribution(0, (int)cellTypes_ - 1);
    std::mt19937 & randomGenerator = _engine_->GetRandomGenerator();

    std::shared_ptr<json> gridDelta = std::make_shared<json>(
        json({
            {
                {"command", "fill"},
                {"tiles", json::array()}
            }
        })
    );

    for (size_t j = 0; j < cellsX_; ++j) {
        for (size_t i = 0; i < cellsY_; ++i) {
            int value = randomDistribution(randomGenerator);
            grid_[i][j] = value;
        }
    }

    for (size_t j = 0; j < cellsX_; ++j) {
        for (size_t i = 0; i < cellsY_; ++i) {
            (*gridDelta)[0]["tiles"].emplace_back(json({
                {"x", j},
                {"y", i},
                {"value", grid_[i][j]}
            }));
        }
    }

    _messageBus_->Notify("/Game/Grid/Model/Changed", gridDelta);
}

} // namespace Match3
