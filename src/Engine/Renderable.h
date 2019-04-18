#pragma once

#include <SDL.h>


namespace Match3 {

class Renderable {
public:
    virtual void Render(SDL_Renderer* renderer) = 0;
};


} // namespace Match3
