#include "Application.h"
#include "GraphicsIncludes.h"

using namespace GUI;

void Application::run() {
    GLFWwindow* window = m_window.getWindow();

    while (!glfwWindowShouldClose(window)) {
        m_renderer.RenderFrame(m_window);
    }

    glfwDestroyWindow(window);
 
    glfwTerminate();
    exit(EXIT_SUCCESS);
}