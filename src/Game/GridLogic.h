#pragma once

#include <queue>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "Util/Array2D.h"


namespace Match3 {

class GridLogic {
public:
    GridLogic();

private:
    void initialFillGrid();

    size_t cellsX_;
    size_t cellsY_;
    size_t cellTypes_;

    static const int EMPTY = -1;

    Util::Array2D<int> grid_;
};

} // namespace Match3
