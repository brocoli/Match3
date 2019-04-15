#include <iostream>
#include <filesystem>
#include <fstream>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;


#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char* argv[]) {
    // Set current directory and load initial window configuration file //

    std::filesystem::path currentDirectory = std::filesystem::path();

    if (!std::filesystem::exists(currentDirectory / "config" / "window.json")) {
        // Try the path from std::filesystem::current_path() if relative paths are not working.
        currentDirectory = std::filesystem::current_path();

        if (!std::filesystem::exists(currentDirectory / "config" / "window.json")) {
            // Try the path from argv[0] if std::filesystem::current_path() is being masked.
            currentDirectory = std::filesystem::path(argv[0]).parent_path();

            if (!std::filesystem::exists(currentDirectory / "config" / "window.json")) {
                std::cout << "Failed to find config directory." << std::endl
                    << "  std::filesystem::current_path() returns " << std::filesystem::current_path() << std::endl
                    << "  argv[0] is " << argv[0] << std::endl;
                return EXIT_FAILURE;
            }
        }
    }

    json windowConfig;

    std::ifstream windowConfigStream(currentDirectory / "config" / "window.json");
    windowConfigStream >> windowConfig;
    windowConfigStream.close();


    // Initialise SDL //

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cout << "Failed to init SDL2: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    int SDL_imageInitFlags = IMG_INIT_PNG | IMG_INIT_JPG;

    if (!(IMG_Init(SDL_imageInitFlags) & SDL_imageInitFlags)) {
        std::cout << "Failed to init SDL2_image: " << IMG_GetError() << std::endl;
        return EXIT_FAILURE;
    }


    // Create window //

    auto* const window = SDL_CreateWindow(
        "Match3",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        windowConfig["width"], windowConfig["height"],
        SDL_WINDOW_ALLOW_HIGHDPI
    );

    if (window == nullptr) {
        std::cout << "Failed to create window: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    // TEST //
    SDL_Surface* windowSurface = SDL_GetWindowSurface(window);
    SDL_Surface* imageSurface = IMG_Load((currentDirectory / "img" / "Backdrop13.jpg").generic_string().c_str());

    if (imageSurface == nullptr) {
        std::cout << "Failed to load texture img/Backdrop13.jpg: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }
    // /TEST //


    // Main loop //

    SDL_Event windowEvent;

    while (true) {
        if (SDL_PollEvent(&windowEvent)) {
            if (windowEvent.type == SDL_QUIT) {
                break;
            }
        }

        // TEST //
        SDL_BlitSurface(imageSurface, nullptr, windowSurface, nullptr);
        SDL_UpdateWindowSurface(window);
        // /TEST //
    }


    // Cleanup //

    SDL_FreeSurface(imageSurface);

    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
