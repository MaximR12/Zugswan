#pragma once

#include "GameState.h"
#include "Window.h"
#include <iostream>

namespace GUI {
    class Renderer {
    private:
        const Game::GameState* m_state;

    public:
        Renderer(const Game::GameState& state) : m_state {&state} { }

        void RenderFrame(Window& window);
    };
}