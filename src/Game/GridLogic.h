#pragma once

#include <queue>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "Util/Array2D.h"
#include "Engine/MessageBus.h"


namespace Match3 {

class GridLogic {
public:
    GridLogic();
    ~GridLogic();

private:
    void initialFillGrid();
    void trySwapTiles(int j, int i, int dj, int di);

    MessageBus::CallbackPtr onGridInputSwap_;

    size_t cellsX_;
    size_t cellsY_;
    size_t cellTypes_;

    static const int EMPTY = -1;

    Util::Array2D<int> grid_;
};

} // namespace Match3
