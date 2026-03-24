#pragma once

#include <iostream>
#include "GameState.h"

struct GLFWwindow;

namespace GUI {
    struct WindowInfo {
        const char* title;
        const int width;
        const int height;
    };

    struct GameDimensions {
        const float xPos;
        const float yPos;
        const float len;
    };
    constexpr struct GameDimensions GameDimensions {-0.5f, 0.5f, 1.0f};

    class Window {
    private:
        WindowInfo m_windowInfo;
        GLFWwindow* m_window;

    public:
        Window(const char* title, int width, int height, Game::GameState* state);
        ~Window();

        GLFWwindow* getWindow() { return m_window; }
        const int getWidth() const { return m_windowInfo.width; }
        const int getHeight() const { return m_windowInfo.height; }
    };
}