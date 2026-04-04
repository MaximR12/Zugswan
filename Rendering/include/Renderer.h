#pragma once

#include "GameState.h"
#include "Window.h"
#include <iostream>

namespace GUI {
    constexpr int CIRCLE_RES = 100;

    class Renderer {
    private:
        const GameState* m_state;

    public:
        Renderer(const GameState& state) : m_state {&state} { }

        void RenderFrame(Window& window);
    };
}