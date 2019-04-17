#include <iostream>
#include <filesystem>

#include "Engine.h"
#include "MessageBus.h"


namespace Match3 {

// Globals //
Engine* _engine_ = nullptr;
MessageBus* _messageBus_ = nullptr;

} // namespace Match3


// Main //

#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char* argv[]) {
    // Set current directory path //

    std::filesystem::path currentDirectory = std::filesystem::path();

    if (!std::filesystem::exists(currentDirectory / "config" / "engine.json")) {
        // Try the path from std::filesystem::current_path() if relative paths are not working.
        currentDirectory = std::filesystem::current_path();

        if (!std::filesystem::exists(currentDirectory / "config" / "engine.json")) {
            // Try the path from argv[0] if std::filesystem::current_path() is being masked.
            currentDirectory = std::filesystem::path(argv[0]).parent_path();

            if (!std::filesystem::exists(currentDirectory / "config" / "engine.json")) {
                std::cout << "Failed to find config directory." << std::endl
                    << "  std::filesystem::current_path() returns " << std::filesystem::current_path() << std::endl
                    << "  argv[0] is " << argv[0] << std::endl;
                return EXIT_FAILURE;
            }
        }
    }

    {
        Match3::MessageBus messageBus;
        Match3::_messageBus_ = &messageBus;

        Match3::Engine engine(currentDirectory);
        Match3::_engine_ = &engine;
        engine.Run();

        if (engine.GetFinishState() != Match3::Engine::FinishState::WindowClosed) {
            std::cout << "The engine finished with a problem. " << engine.GetFinishState() << std::endl;
            return EXIT_FAILURE;
        }
    }
    Match3::_messageBus_ = nullptr;
    Match3::_engine_ = nullptr;

    return EXIT_SUCCESS;
}
