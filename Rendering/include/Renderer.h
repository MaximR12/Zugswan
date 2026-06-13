#pragma once

#include "GameState.h"
#include "Window.h"
#include <iostream>

namespace GUI {
    constexpr int CIRCLE_RES = 100;

    class Renderer {
    private:
        RenderState* m_state;

    public:
        Renderer(RenderState* state) : m_state {state} { }

        void RenderFrame(GLFWwindow* window);
    };
}