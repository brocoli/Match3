#pragma once

#include <list>
#include <random>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "Util/Array2D.h"
#include "Engine/MessageBus.h"


namespace Match3 {

class GridLogic {
public:
    GridLogic();
    ~GridLogic();

    void InitialFillGrid();

private:
    static constexpr int EMPTY = -1;
    static constexpr int NOT_A_COLOR = -2;

    void trySwapTiles(int j, int i, int dj, int di);

    std::list<json> solveGridUntilStable();

    bool solvePatternsInGrid(std::list<json>& actionLogDelta);
    void applyGravityToTiles(std::list<json>& actionLogDelta);
    bool fillEmptyTilesInGrid(std::list<json>& actionLogDelta);

    MessageBus::CallbackPtr onGridInputSwap_;

    std::uniform_int_distribution<int> randomTileGeneratorDistribution_;

    size_t cellsX_;
    size_t cellsY_;
    size_t cellTypes_;

    std::forward_list<Util::Array2D<int>> patterns_; // Util::Array2D<bool> doesn't work...

    Util::Array2D<int> grid_;
};

} // namespace Match3
