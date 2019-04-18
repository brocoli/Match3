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


    onGridInputSwap_ = std::make_shared<MessageBus::Callback>(
        [this](const MessageBus::Key&, MessageBus::Data mouseEvent) -> void {
            const std::string axis = (*mouseEvent)["axis"];
            int sign = (*mouseEvent)["sign"];

            int dj = 0;
            int di = 0;

            if (axis == "x") {
                dj = sign;
            } else {
                di = sign;
            }

            trySwapTiles((*mouseEvent)["j"], (*mouseEvent)["i"], dj, di);
        }
    );
    _messageBus_->Attach("/Game/Grid/View/Input/Swap", onGridInputSwap_);


    initialFillGrid();
}

GridLogic::~GridLogic() {
    _messageBus_->Detach("/Game/Grid/View/Input/Swap", onGridInputSwap_);
}

void GridLogic::initialFillGrid() {
    std::uniform_int_distribution<int> randomDistribution(0, (int)cellTypes_ - 1);
    std::mt19937 & randomGenerator = _engine_->GetRandomGenerator();

    std::shared_ptr<json> actionLogDelta = std::make_shared<json>(
        json({
            {
                {"action", "fill"},
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

    //TODO: solve board until stabilizes

    for (size_t j = 0; j < cellsX_; ++j) {
        for (size_t i = 0; i < cellsY_; ++i) {
            (*actionLogDelta)[0]["tiles"].emplace_back(json({
                {"x", j},
                {"y", i},
                {"value", grid_[i][j]}
            }));
        }
    }

    _messageBus_->Notify("/Game/Grid/Logic/ActionLog/Changed", actionLogDelta);
}

void GridLogic::trySwapTiles(int j, int i, int dj, int di) {
    int temp = grid_[i][j];
    grid_[i][j] = grid_[i + di][j + dj];
    grid_[i + di][j + dj] = temp;

    //TODO: search for new matches
    //  if found, solve board until board stabilizes
    //  otherwise, revert change and just notify the attempt

    std::shared_ptr<json> actionLogDelta = std::make_shared<json>(
        json({
            {
                {"action", "tileSwap"},
                {"tiles", json::array({
                    json::array({ j, i }),
                    json::array({ j+dj, i+di }),
                })}
            }
        })
    );

    _messageBus_->Notify("/Game/Grid/Logic/ActionLog/Changed", actionLogDelta);
}

} // namespace Match3
