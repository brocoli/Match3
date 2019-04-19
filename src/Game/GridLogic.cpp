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

    randomTileGeneratorDistribution_ = std::uniform_int_distribution<int>(0, (int)cellTypes_ - 1);

    grid_ = Util::Array2D<int>(cellsX_, cellsY_, EMPTY);


    for each (auto pattern in configuration["colorPatterns"]) {
        size_t cols = pattern["cols"];
        size_t rows = pattern["rows"];
        Util::Array2D<int> patternArray(cols, rows, 0);

        std::vector<std::vector<int>> patternGrid = pattern["pattern"];

        for (size_t j = 0; j < cols; ++j) {
            for (size_t i = 0; i < rows; ++i) {
                patternArray[i][j] = patternGrid[i][j];
            }
        }

        patterns_.emplace_front(patternArray);
    }


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
}

GridLogic::~GridLogic() {
    _messageBus_->Detach("/Game/Grid/View/Input/Swap", onGridInputSwap_);
}

void GridLogic::InitialFillGrid() {
    std::mt19937& randomGenerator = _engine_->GetRandomGenerator();

    std::list<json> actionLogDelta = json::array();
    std::list<std::array<size_t, 2>> filledTilesCoordinates;
    std::list<int> filledTilesColors;

    for (size_t j = 0; j < cellsX_; ++j) {
        for (size_t i = 0; i < cellsY_; ++i) {
            grid_[i][j] = randomTileGeneratorDistribution_(randomGenerator);
        }
    }

    solveGridUntilStable();

    for (size_t j = 0; j < cellsX_; ++j) {
        for (size_t i = 0; i < cellsY_; ++i) {
            filledTilesCoordinates.emplace_back(std::array<size_t, 2>{ j, i });
            filledTilesColors.emplace_back(grid_[i][j]);
        }
    }

    actionLogDelta.emplace_back(json({
        {"action", "fill"},
        {"tilesCoordinates", filledTilesCoordinates},
        {"tilesColors", filledTilesColors},
    }));
    _messageBus_->Notify("/Game/Grid/Logic/ActionLog/Changed", std::make_shared<const json>(actionLogDelta));
}

void GridLogic::trySwapTiles(int j, int i, int dj, int di) {
    int temp = grid_[i][j];
    grid_[i][j] = grid_[i + di][j + dj];
    grid_[i + di][j + dj] = temp;

    std::list<json> actionLogDelta = solveGridUntilStable();

    actionLogDelta.emplace_front(json({
        {"action", actionLogDelta.empty() ? "failedSwapAttempt" : "tileSwap"},
        {"tiles", json::array({
            json::array({ j, i }),
            json::array({ j + dj, i + di }),
        })}
    }));
    _messageBus_->Notify("/Game/Grid/Logic/ActionLog/Changed", std::make_shared<const json>(actionLogDelta));
}

std::list<json> GridLogic::solveGridUntilStable() {
    std::list<json> actionLogDelta;

    while (solvePatternsInGrid(actionLogDelta)) {
        applyGravityToTiles(actionLogDelta);
        if (!fillEmptyTilesInGrid(actionLogDelta)) {
            break;
        }
    }

    return actionLogDelta;
}

bool GridLogic::solvePatternsInGrid(std::list<json>& actionLogDelta) {
    std::list<std::list<std::array<size_t, 2>>> mergersFound;

    for (int color = 0; color < cellTypes_; ++color) {
        std::unordered_set<size_t> collectedTiles;

        for each (auto pattern in patterns_) {
            size_t cols = pattern.GetCols();
            size_t rows = pattern.GetRows();

            // Build a colored version of the pattern
            auto coloredPattern = Util::Array2D<int>(pattern);

            for (size_t j = 0; j < cols; ++j) {
                for (size_t i = 0; i < rows; ++i) {
                    if (pattern[i][j]) {
                        coloredPattern[i][j] = color;
                    }
                    else {
                        coloredPattern[i][j] = NOT_A_COLOR;
                    }
                }
            }

            // Find all pattern instances
            auto patternInstances = grid_.FindPatternInstances(coloredPattern);

            // Collect all tiles affected by these patterns
            // TODO: the correct thing to do would be to search
            //   for expanded dependent patterns whenever they overlap
            //   i.e. a line of 3 could expand to a line of 5 or a cross.
            for (size_t pIdx = 0; pIdx < patternInstances.size(); pIdx += 2) {
                size_t j = patternInstances[pIdx];
                size_t i = patternInstances[pIdx + 1];

                for (size_t pj = 0; pj < cols; ++pj) {
                    for (size_t pi = 0; pi < rows; ++pi) {
                        if (pattern[pi][pj]) {
                            collectedTiles.emplace((j + pj) * cellsY_ + (i + pi));
                        }
                    }
                }
            }
        }

        // Record the merger
        if (!collectedTiles.empty()) {
            std::list<std::array<size_t, 2>> merger;
            for each (size_t coords in collectedTiles) {
                size_t j = coords / cellsY_;
                size_t i = coords % cellsY_;
                grid_[i][j] = EMPTY;
                merger.emplace_back(std::array<size_t, 2>{ j, i });
            }
            mergersFound.emplace_back(merger);
        }
    }

    if (!mergersFound.empty()) {
        actionLogDelta.emplace_back(json({
            {"action", "merge"},
            {"groups", mergersFound},
        }));

        return true;
    }

    return false;
}

void GridLogic::applyGravityToTiles(std::list<json>& actionLogDelta) {
    std::list<std::array<size_t, 2>> tilesFallen;
    std::list<std::array<size_t, 2>> tileGrounds;

    for (size_t j = 0; j < cellsX_; ++j) {
        for (int i = (int)cellsY_ - 1, groundI = (int)cellsY_ - 1; 0 <= i; --i) {
            size_t _i = (size_t)i;
            if (grid_[_i][j] != EMPTY) {
                size_t _groundI = (size_t)groundI;
                if (grid_[_groundI][j] == EMPTY) {
                    grid_[_groundI][j] = grid_[_i][j];
                    grid_[_i][j] = EMPTY;

                    tilesFallen.emplace_back(std::array<size_t, 2>{ j, _i });
                    tileGrounds.emplace_back(std::array<size_t, 2>{ j, _groundI });
                }
                --groundI;
            }
        }
    }

    if (!tilesFallen.empty()) {
        actionLogDelta.emplace_back(json({
            {"action", "gravity"},
            {"tilesFallen", tilesFallen},
            {"tileGrounds", tileGrounds},
        }));
    }
}

bool GridLogic::fillEmptyTilesInGrid(std::list<json>& actionLogDelta) {
    std::mt19937& randomGenerator = _engine_->GetRandomGenerator();

    std::list<std::array<size_t, 2>> filledTilesCoordinates;
    std::list<int> filledTilesColors;

    for (size_t j = 0; j < cellsX_; ++j) {
        for (size_t i = 0; i < cellsY_; ++i) {
            if (grid_[i][j] == EMPTY) {
                int value = randomTileGeneratorDistribution_(randomGenerator);
                grid_[i][j] = value;
                filledTilesCoordinates.emplace_back(std::array<size_t, 2>({ j, i }));
                filledTilesColors.emplace_back(value);
            }
        }
    }

    if (!filledTilesCoordinates.empty()) {
        actionLogDelta.emplace_back(json({
            {"action", "fill"},
            {"tilesCoordinates", filledTilesCoordinates},
            {"tilesColors", filledTilesColors},
        }));

        return true;
    }

    return false;
}

} // namespace Match3
