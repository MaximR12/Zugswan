#include "Window.h"
#include "GraphicsIncludes.h"

using namespace GUI;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    switch(key) {
        case(GLFW_KEY_ESCAPE):
            if(action == GLFW_PRESS)
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
    }
}

static void mouse_callback(GLFWwindow* window, int button, int action, int mods) {
    GameState* state = static_cast<GameState*>(glfwGetWindowUserPointer(window)); 

    if(button != GLFW_MOUSE_BUTTON_1) {
        return;
    }

    if(action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        const int squares_per_side = 8;
        const float boardX = GameDimensions.xPos, boardY = GameDimensions.yPos, boardLen = GameDimensions.len, squareLen = boardLen / squares_per_side;
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        const int col = int(xpos / (squareLen * width / boardLen/2.0f)) - 4, row = int(ypos / (squareLen * height / boardLen/2.0f)) - 4;
        const int square = (7 - row) * 8 + col;
        
        if(col >= 0 && row >= 0 && col < squares_per_side && row < squares_per_side) {
            state->handleClick(square);
        }
    }
}

static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
} 

Window::Window(const char* title, int width, int height, GameState* state) : m_windowInfo {title, width, height} {
    glfwSetErrorCallback(error_callback);

    if(!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(m_windowInfo.width, m_windowInfo.height, m_windowInfo.title, NULL, NULL);

    if(!m_window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);  
    glfwSetWindowUserPointer(m_window, state);
    glfwSetKeyCallback(m_window, key_callback);
    glfwSetMouseButtonCallback(m_window, mouse_callback);

    glfwMakeContextCurrent(m_window);
    gladLoadGL();
    glfwSwapInterval(1);
}

Window::~Window() {
    glfwDestroyWindow(m_window);
}