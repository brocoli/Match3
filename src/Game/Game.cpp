#include "Game.h"

#include "Engine/AtlasImage.h"
#include "Engine/Engine.h"
#include "Engine/MessageBus.h"


namespace Match3 {

extern MessageBus* _messageBus_;
extern Engine* _engine_;

Game::Game() {
    onEngineStarted_ = std::make_shared<MessageBus::Callback>(
        [this](const MessageBus::Key&, MessageBus::Data) -> void {
            loadAllGameTextures();
            insertBackgroundImage();
            createGridView();
        }
    );

    _messageBus_->Attach("/Engine/Started", onEngineStarted_);
}

Game::~Game() {
    _messageBus_->Detach("/Engine/Started", onEngineStarted_);
}

void Game::loadAllGameTextures() {
    auto atlasHandler = _engine_->GetResources()->GetAtlasHandler();
    atlasHandler->Load("Backdrop13.jpg");
    atlasHandler->Load("Pieces.png");
}

void Game::insertBackgroundImage() {
    std::shared_ptr<Renderable> background = std::make_shared<AtlasImage>(
        "Backdrop13.jpg", "background", 0, 0, true
    );
    _engine_->InsertRenderable(background);
}

void Game::createGridView() {
    gridView_ = std::make_shared<GridView>();
}

} // namespace Match3
