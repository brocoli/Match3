#include <iostream>
#include <experimental/filesystem>
#include <random>

#include "Engine/Engine.h"
#include "Engine/MessageBus.h"
#include "Game/Game.h"


namespace Match3 {

// Globals //
MessageBus* _messageBus_ = nullptr;
Engine* _engine_ = nullptr;

} // namespace Match3


// Main //

#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char* argv[]) {
    // Set current directory path //

    std::experimental::filesystem::path currentDirectory = std::experimental::filesystem::path();

    if (!std::experimental::filesystem::exists(currentDirectory / "config" / "engine.json")) {
        // Try the path from std::experimental::filesystem::current_path() if relative paths are not working.
        currentDirectory = std::experimental::filesystem::current_path();

        if (!std::experimental::filesystem::exists(currentDirectory / "config" / "engine.json")) {
            // Try the path from argv[0] if std::experimental::filesystem::current_path() is being masked.
            currentDirectory = std::experimental::filesystem::path(argv[0]).parent_path();

            if (!std::experimental::filesystem::exists(currentDirectory / "config" / "engine.json")) {
                std::cout << "Failed to find config directory." << std::endl
                    << "  std::experimental::filesystem::current_path() returns " << std::experimental::filesystem::current_path() << std::endl
                    << "  argv[0] is " << argv[0] << std::endl;
                return EXIT_FAILURE;
            }
        }
    }

    {
        Match3::MessageBus messageBus;
        Match3::_messageBus_ = &messageBus;

        Match3::Engine engine(currentDirectory, std::random_device()());
        Match3::_engine_ = &engine;

        Match3::Game game;

        engine.Run();

        if (engine.GetFinishState() != Match3::Engine::FinishState::WindowClosed) {
            std::cout << "The engine finished with a problem. " << engine.GetFinishState() << std::endl;
            return EXIT_FAILURE;
        }
    }
    Match3::_messageBus_ = nullptr;

    return EXIT_SUCCESS;
}
