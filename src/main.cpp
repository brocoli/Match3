#include <iostream>
#ifdef __APPLE__
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif
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

    fs::path currentDirectory = fs::path();

    if (!fs::exists(currentDirectory / "config" / "engine.json")) {
        // Try the path from fs::current_path() if relative paths are not working.
        currentDirectory = fs::current_path();

        if (!fs::exists(currentDirectory / "config" / "engine.json")) {
            // Try the path from argv[0] if fs::current_path() is being masked.
            currentDirectory = fs::path(argv[0]).parent_path();

            if (!fs::exists(currentDirectory / "config" / "engine.json")) {
                std::cout << "Failed to find config directory." << std::endl
                    << "  fs::current_path() returns " << fs::current_path() << std::endl
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
