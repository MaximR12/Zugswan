#pragma once

#include <mutex>
#include "Window.h"
#include "Renderer.h"
#include "GameState.h"

constexpr static int WINDOW_WIDTH = 640, WINDOW_HEIGHT = 640;

class Game {
private:
    GUI::Window m_window;
    GUI::Renderer m_renderer;
    GameState m_gameState;
    RenderState m_renderState;

    std::mutex m_stateLock;
    std::mutex m_renderLock;

public:
    Game(const char* title) : m_window{title, WINDOW_WIDTH, WINDOW_HEIGHT, this}, m_gameState{}, m_renderer{&m_renderState} { 
        std::lock_guard<std::mutex> guard(m_stateLock); 
        m_gameState.updateRenderState(m_renderState);
    }

    void perftCommand(std::stringstream& ss);
    void perftBulkCommand(std::stringstream& ss);
    void divideCommand(std::stringstream& ss);
    void loadPositionCommand(std::stringstream& ss);
    
    void handleClick(int square);
    void unMakeMove();
    void run();
};