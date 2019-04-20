#include "GridView.h"

#include <fstream>
#include <random>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "Util/Signum.h"

#include "Engine/AtlasImage.h"
#include "Engine/Engine.h"
#include "Engine/MessageBus.h"
#include "Engine/Tween.h"


namespace Match3 {

extern MessageBus* _messageBus_;
extern Engine* _engine_;

GridView::GridView() :
    tileImageByCoordinates_(0, 0, nullptr),
    heldTile_(nullptr), heldTileOriginalCoordinates_{ 0, 0 }, heldTileOriginalPosition_{ 0, 0 },
    interactedTile_(nullptr), interactedTileOriginalCoordinates_{ 0, 0 }, interactedTileOriginalPosition_{ 0, 0 },
    currentDragDelta_{ 0, 0 }, dragMovementVelocity_{ 0, 0 },
    inputSensitivityPosition_(0), inputSensitivityVelocityFactor_(0), tileMovementVelocity_(0),
    busy_(false)
{
    json configuration;
    {
        std::ifstream configurationStream((_engine_->GetCurrentDirectory() / "config" / "grid.json").generic_string());
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


    const json& inputSensitivity = configuration["inputSensitivity"];
    inputSensitivityPosition_ = inputSensitivity["position"];
    inputSensitivityVelocityFactor_ = inputSensitivity["velocityFactor"];


    tileMovementVelocity_ = configuration["tileMovementVelocity"];


    const json& window = _engine_->GetWindowConfiguration();

    layoutOffsetX_ = ((int)window["width"] - (int)(layoutCellWidth_ + layoutCellMarginX_) * (int)cellsX_) / 2;
    layoutOffsetY_ = ((int)window["height"] - (int)(layoutCellHeight_ + layoutCellMarginY_) * (int)cellsY_) / 2;

    size_t totalCells = cellsX_ * cellsY_;
    for (size_t i = 0; i < totalCells; ++i) {
        std::shared_ptr<Cell> img = std::make_shared<Cell>(
            "Pieces.png", tileImageNames_[0],
            0, 0,
            false
        );
        _engine_->InsertRenderable(img);
        tilePool_.push_front(img);
    }

    tileImageByCoordinates_ = Util::Array2D<std::shared_ptr<Cell>>(cellsX_, cellsY_, nullptr);


    onGridActionLogChanged_ = std::make_shared<MessageBus::Callback>(
        [this](const MessageBus::Key&, MessageBus::Data actionLogDelta) -> void {
            enqueueActionLogDelta(actionLogDelta);
        }
    );
    _messageBus_->Attach("/Game/Grid/Logic/ActionLog/Changed", onGridActionLogChanged_);


    onMouseLeftStarted_ = std::make_shared<MessageBus::Callback>(
        [this](const MessageBus::Key&, MessageBus::Data mouseEvent) -> void {
            if (!busy_) {
                pickUpTileByPosition((*mouseEvent)["x"], (*mouseEvent)["y"]);
            }
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
    _messageBus_->Detach("/Engine/Input/Mouse/Left/Stopped", onMouseLeftStopped_);
    _messageBus_->Detach("/Engine/Input/Mouse/Left/Moved", onMouseLeftMoved_);
    _messageBus_->Detach("/Engine/Input/Mouse/Left/Started", onMouseLeftStarted_);

    _messageBus_->Detach("/Game/Grid/Logic/ActionLog/Changed", onGridActionLogChanged_);
}

Int2D GridView::calculateXYFromCoordinates(size_t j, size_t i) {
    int x = ((layoutCellWidth_ + layoutCellMarginX_) * (int)j) + layoutOffsetX_;
    int y = ((layoutCellHeight_ + layoutCellMarginY_) * (int)i) + layoutOffsetY_;

    return Int2D(x, y);
}

stdoptional<Size2D> GridView::calculateCoordinatesFromXY(int x, int y) {
    x -= layoutOffsetX_;
    y -= layoutOffsetY_;

    int j = x / (layoutCellWidth_ + layoutCellMarginX_);
    int i = y / (layoutCellHeight_ + layoutCellMarginY_);

    if (j < 0 || cellsX_ <= j || i < 0 || cellsY_ <= i) {
        // index out of range
        return stdoptional<Size2D>();
    }

    x %= layoutCellWidth_ + layoutCellMarginX_;
    y %= layoutCellHeight_ + layoutCellMarginY_;

    if (x >= layoutCellWidth_ || y >= layoutCellHeight_) {
        // position in margin between tiles
        return stdoptional<Size2D>();
    }

    return stdoptional<Size2D>({(size_t)j, (size_t)i});
}

void GridView::enqueueActionLogDelta(MessageBus::Data actionLogDelta) {
    auto logDelta = *actionLogDelta;
    for (auto dt = logDelta.begin(); dt != logDelta.end(); ++dt) {
        auto delta = *dt;
        pendingActionLogDeltas_.emplace_front(delta);
    }
    enactActionLogDelta();
}

void GridView::enactActionLogDelta() {
    while (!busy_ && !pendingActionLogDeltas_.empty()) {
        auto delta = pendingActionLogDeltas_.back();
        pendingActionLogDeltas_.pop_back();

        if (delta["action"] == "fill") {
            for (size_t i = 0; i < delta["tilesCoordinates"].size(); ++i) {
                auto tile = delta["tilesCoordinates"][i];
                fillTile(tile[0], tile[1], delta["tilesColors"][i]);
            }
        } else if (delta["action"] == "tileSwap") {
            swapTiles(delta["tiles"][0][0], delta["tiles"][0][1], delta["tiles"][1][0], delta["tiles"][1][1]);
        } else if (delta["action"] == "failedSwapAttempt") {
            returnTiles(delta["tiles"][0][0], delta["tiles"][0][1], delta["tiles"][1][0], delta["tiles"][1][1]);
        } else if (delta["action"] == "merge") {
            mergeTiles(delta["groups"]);
        } else if (delta["action"] == "gravity") {
            dropTiles(delta["tilesFallen"], delta["tileGrounds"]);
        }
    }
}

void GridView::fillTile(size_t j, size_t i, size_t tileType) {
    std::shared_ptr<Cell> tile = tilePool_.front();
    tilePool_.pop_front();

    int height = (int)(_engine_->GetWindowConfiguration()["height"]) / (int)2;

    tile->SetImage(tileImageNames_[tileType]);
    Int2D xy = calculateXYFromCoordinates(j, i);
    int endY = xy.second;
    xy.second -= height;
    int startY = xy.second;
    tile->SetXY(xy);
    tile->SetVisible(true);

    tileImageByCoordinates_[i][j] = tile;

    busy_ = true;
    tweenRunner_.StartTween(
        1000 * height / tileMovementVelocity_, startY,
        Easing::InQuad(startY, endY - startY),
        [tile](const int& data) { tile->SetY(data); },
        [this]() {
            if (tweenRunner_.GetLivingTweenCount() <= 1) {
                busy_ = false;
                enactActionLogDelta();
            }
        }
    );
}

void GridView::emptyTile(size_t j, size_t i) {
    std::shared_ptr<Cell> tile = tileImageByCoordinates_[i][j];
    tileImageByCoordinates_[i][j] = nullptr;
    tile->SetVisible(false);
    tilePool_.push_front(tile);
}

void GridView::swapTiles(size_t j, size_t i, size_t oj, size_t oi) {
    std::shared_ptr<Cell> tile = tileImageByCoordinates_[i][j];
    std::shared_ptr<Cell> otherTile = tileImageByCoordinates_[oi][oj];

    tileImageByCoordinates_[i][j] = otherTile;
    tileImageByCoordinates_[oi][oj] = tile;

    tile->SetXY(calculateXYFromCoordinates(oj, oi));
    otherTile->SetXY(calculateXYFromCoordinates(j, i));
}

void GridView::returnTiles(size_t j, size_t i, size_t oj, size_t oi) {
    std::shared_ptr<Cell> tile = tileImageByCoordinates_[i][j];
    std::shared_ptr<Cell> otherTile = tileImageByCoordinates_[oi][oj];

    tile->SetXY(calculateXYFromCoordinates(j, i));
    otherTile->SetXY(calculateXYFromCoordinates(oj, oi));
}

void GridView::mergeTiles(std::list<std::list<std::array<size_t, 2>>> groups) {
    groupsToMergeTempCopy_ = std::list<std::list<std::array<size_t, 2>>>(groups);
    tweenCountTemp_ = 0;

    for (auto gt = groups.begin(); gt != groups.end(); ++gt) {
        auto mergedGroup = *gt;
        size_t minJ = std::numeric_limits<size_t>::max(), maxJ = std::numeric_limits<size_t>::min();
        size_t minI = std::numeric_limits<size_t>::max(), maxI = std::numeric_limits<size_t>::min();

        for (auto ct = mergedGroup.begin(); ct != mergedGroup.end(); ++ct) {
            auto coordinates = *ct;
            minJ = std::min(minJ, coordinates[0]);
            maxJ = std::max(maxJ, coordinates[0]);
            minI = std::min(minI, coordinates[1]);
            maxI = std::max(maxI, coordinates[1]);
        }

        size_t halfJ = (minJ + maxJ) / 2, halfI = (minI + maxI) / 2;

        for (auto ct = mergedGroup.begin(); ct != mergedGroup.end(); ++ct) {
            auto coordinates = *ct;
            size_t j = coordinates[0], i = coordinates[1];

            std::shared_ptr<Cell> tile = tileImageByCoordinates_[i][j];

            Int2D startXY = tile->GetXY();
            Int2D endXY = calculateXYFromCoordinates(halfJ, halfI);

            int startX = startXY.first, startY = startXY.second;
            int endX = endXY.first, endY = endXY.second;
            int deltaX = endX - startX;
            int deltaY = endY - startY;

            busy_ = true;
            if (deltaX != 0) {
                ++tweenCountTemp_;
                tweenRunner_.StartTween(
                    1000 * std::abs(deltaX) / tileMovementVelocity_, startX,
                    Easing::InQuad(startX, deltaX),
                    [tile](const int& data) { tile->SetX(data); },
                    [this]() {
                        --tweenCountTemp_;
                        if (tweenCountTemp_ <= 0) {
                            for (auto gt = groupsToMergeTempCopy_.begin(); gt != groupsToMergeTempCopy_.end(); ++gt) {
                                auto mergedGroup = *gt;
                                for (auto rt = mergedGroup.begin(); rt != mergedGroup.end(); ++rt) {
                                    auto coords = *rt;
                                    emptyTile(coords[0], coords[1]);
                                }
                            }
                            if (tweenRunner_.GetLivingTweenCount() <= 1) {
                                busy_ = false;
                                enactActionLogDelta();
                            }
                        }
                    }
                );
            }
            if (deltaY != 0) {
                ++tweenCountTemp_;
                tweenRunner_.StartTween(
                    1000 * std::abs(deltaY) / tileMovementVelocity_, startY,
                    Easing::InQuad(startY, deltaY),
                    [tile](const int& data) { tile->SetY(data); },
                    [this]() {
                        --tweenCountTemp_;
                        if (tweenCountTemp_ <= 0) {
                            for (auto gt = groupsToMergeTempCopy_.begin(); gt != groupsToMergeTempCopy_.end(); ++gt) {
                                auto mergedGroup = *gt;
                                for (auto rt = mergedGroup.begin(); rt != mergedGroup.end(); ++rt) {
                                    auto coords = *rt;
                                    emptyTile(coords[0], coords[1]);
                                }
                            }
                            if (tweenRunner_.GetLivingTweenCount() <= 1) {
                                busy_ = false;
                                enactActionLogDelta();
                            }
                        }
                    }
                );
            }
        }
    }
}

void GridView::dropTiles(std::vector<std::array<size_t, 2>> tilesFallen, std::vector<std::array<size_t, 2>> tileGrounds) {
    // Note that these lists are assumed to be ordered in a way that avoids collisions
    for (size_t k = 0; k < tilesFallen.size(); ++k) {
        std::array<size_t, 2> tileFallen = tilesFallen[k];
        std::array<size_t, 2> tileGround = tileGrounds[k];

        size_t j = tileFallen[0], i = tileFallen[1], oj = tileGround[0], oi = tileGround[1];

        std::shared_ptr<Cell> tile = tileImageByCoordinates_[i][j];
        std::shared_ptr<Cell> otherTile = tileImageByCoordinates_[oi][oj];

        tileImageByCoordinates_[i][j] = nullptr;
        tileImageByCoordinates_[oi][oj] = tile;

        Int2D startXY = tile->GetXY();
        Int2D endXY = calculateXYFromCoordinates(oj, oi);

        int startX = startXY.first, startY = startXY.second;
        int endX = endXY.first, endY = endXY.second;
        int deltaX = endX - startX;
        int deltaY = endY - startY;

        busy_ = true;
        if (deltaX != 0) {
            tweenRunner_.StartTween(
                1000 * std::abs(deltaX) / tileMovementVelocity_, startX,
                Easing::InQuad(startX, deltaX),
                [tile](const int& data) { tile->SetX(data); },
                [this]() {
                    if (tweenRunner_.GetLivingTweenCount() <= 1) {
                        busy_ = false;
                        enactActionLogDelta();
                    }
                }
            );
        }
        if (deltaY != 0) {
            tweenRunner_.StartTween(
                1000 * std::abs(deltaY) / tileMovementVelocity_, startY,
                Easing::InQuad(startY, deltaY),
                [tile](const int& data) { tile->SetY(data); },
                [this]() {
                    if (tweenRunner_.GetLivingTweenCount() <= 1) {
                        busy_ = false;
                        enactActionLogDelta();
                    }
                }
            );
        }
    }
}

void GridView::pickUpTileByPosition(int x, int y) {
    stdoptional<Size2D> maybeCoords = calculateCoordinatesFromXY(x, y);

    if (maybeCoords.has_value()) {
        Size2D coords = maybeCoords.value();
        size_t j = coords.first;
        size_t i = coords.second;

        heldTile_ = tileImageByCoordinates_[i][j];
        heldTileOriginalCoordinates_ = coords;
        heldTileOriginalPosition_ = calculateXYFromCoordinates(j, i);

        _engine_->ToFrontRenderable(heldTile_);

        dragHeldTile(x, y, x, y);
    }
}

void GridView::dragHeldTile(int x, int y, int initialX, int initialY) {
    if (heldTile_ != nullptr) {
        int deltaX = x - initialX;
        int deltaY = y - initialY;

        size_t j = heldTileOriginalCoordinates_.first;
        size_t i = heldTileOriginalCoordinates_.second;

        // Clamp deltas to avoid values that correspond to moves outside of the grid
        if (heldTileOriginalCoordinates_.first <= 0) {
            deltaX = std::max(0, deltaX);
        }
        else if (cellsX_ - 1 <= heldTileOriginalCoordinates_.first) {
            deltaX = std::min(deltaX, 0);
        }

        if (heldTileOriginalCoordinates_.second <= 0) {
            deltaY = std::max(0, deltaY);
        }
        else if (cellsX_ - 1 <= heldTileOriginalCoordinates_.second) {
            deltaY = std::min(deltaY, 0);
        }

        // Find the most significant delta, zero out the other
        int absDeltaX = std::abs(deltaX);
        int absDeltaY = std::abs(deltaY);

        if (absDeltaX == absDeltaY) {
            // Just reset positions and return if they're the same
            dragMovementVelocity_ = Int2D{ 0, 0 };
            currentDragDelta_ = Int2D{ 0, 0 };

            heldTile_->SetXY(heldTileOriginalPosition_);
            if (interactedTile_ != nullptr) {
                interactedTile_->SetXY(interactedTileOriginalPosition_);
            }
            return;
        }

        if (absDeltaX < absDeltaY) {
            deltaX = 0;
            absDeltaX = 0;
        }
        else {
            deltaY = 0;
            absDeltaY = 0;
        }

        // Cap movement at one tile distance
        int signJ = Util::signum(deltaX);
        int signI = Util::signum(deltaY);

        deltaX = signJ * std::min(absDeltaX, layoutCellWidth_ + layoutCellMarginX_);
        deltaY = signI * std::min(absDeltaY, layoutCellHeight_ + layoutCellMarginY_);

        // Find interacted tile and track it if necessary
        int otherI = (int)i + signI;
        int otherJ = (int)j + signJ;

        std::shared_ptr<Cell> otherTile = tileImageByCoordinates_[otherI][otherJ];
        if (interactedTile_ != otherTile) {
            if (interactedTile_ != nullptr) {
                interactedTile_->SetXY(interactedTileOriginalPosition_);
            }

            interactedTile_ = otherTile;
            interactedTileOriginalCoordinates_.first = otherJ;
            interactedTileOriginalCoordinates_.second = otherI;
            interactedTileOriginalPosition_ = calculateXYFromCoordinates(otherJ, otherI);
        }

        // Update positions
        dragMovementVelocity_.first = deltaX - currentDragDelta_.first;
        dragMovementVelocity_.second = deltaY - currentDragDelta_.second;

        currentDragDelta_.first = deltaX;
        currentDragDelta_.second = deltaY;

        heldTile_->SetXY(Int2D{ heldTileOriginalPosition_.first + deltaX, heldTileOriginalPosition_.second + deltaY });
        interactedTile_->SetXY(Int2D{ interactedTileOriginalPosition_.first - deltaX, interactedTileOriginalPosition_.second - deltaY });
    }
}

void GridView::releaseHeldTile(int x, int y, int initialX, int initialY) {
    if (heldTile_ != nullptr) {
        // Calculate whether this release will cause a logic input event
        int velX = dragMovementVelocity_.first;
        int velY = dragMovementVelocity_.second;
        int deltaX = currentDragDelta_.first;
        int deltaY = currentDragDelta_.second;

        int valueToCheck = 0;
        char* axisOfMovement;

        if (deltaX != 0 && deltaY == 0) {
            valueToCheck = deltaX + velX * inputSensitivityVelocityFactor_;
            axisOfMovement = "x";
        } else if (deltaY != 0 && deltaX == 0) {
            valueToCheck = deltaY + velY * inputSensitivityVelocityFactor_;
            axisOfMovement = "y";
        }

        bool shouldNotifyLogicInput = std::abs(valueToCheck) > inputSensitivityPosition_;

        // Clean held tile pointers
        if (!shouldNotifyLogicInput) {
            heldTile_->SetXY(heldTileOriginalPosition_);
        }
        heldTile_ = nullptr;

        if (interactedTile_ != nullptr) {
            if (!shouldNotifyLogicInput) {
                interactedTile_->SetXY(interactedTileOriginalPosition_);
            }
            interactedTile_ = nullptr;
        }

        // Notify input event if appropriate
        if (shouldNotifyLogicInput) {
            _messageBus_->Notify("/Game/Grid/View/Input/Swap", std::make_shared<const json>(json{
                { "axis", axisOfMovement },
                { "sign", Util::signum(valueToCheck) },
                { "j", heldTileOriginalCoordinates_.first },
                { "i", heldTileOriginalCoordinates_.second },
            }));
        };
    }
}

} // namespace Match3
