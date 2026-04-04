#pragma once

#include "Window.h"
#include "Renderer.h"
#include "GameState.h"

constexpr static int WINDOW_WIDTH = 640, WINDOW_HEIGHT = 640;

class Game {
private:
    GUI::Window m_window;
    GameState m_gameState;
    GUI::Renderer m_renderer;

public:
    Game(const char* title) : 
        m_window{title, WINDOW_WIDTH, WINDOW_HEIGHT, &m_gameState}, m_gameState{}, m_renderer{m_gameState} { }
    
    void run();
};