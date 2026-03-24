#pragma once

#include "Window.h"
#include "Renderer.h"

namespace GUI { 
    constexpr static int WINDOW_WIDTH = 640, WINDOW_HEIGHT = 640;

    class Application {
    private:
        Window m_window;
        Game::GameState m_gameState;
        Renderer m_renderer;

    public:
        Application(const char* title) : 
            m_window{title, WINDOW_WIDTH, WINDOW_HEIGHT, &m_gameState}, m_gameState{}, m_renderer{m_gameState} { }
        
        void run();
    };
}