#include "Game.h"
#include "GraphicsIncludes.h"

void Game::run() {
    GLFWwindow* window = m_window.getWindow();

    while (!glfwWindowShouldClose(window)) {
        m_renderer.RenderFrame(m_window);
        if(m_gameState.getTurn() != m_gameState.playerTurn()) {
            m_gameState.playEngineMove();
            m_gameState.updateLegalMoves();
        }
    }

    glfwDestroyWindow(window);
 
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

int main() {
    Game game("Chess Engine");
    game.run();    

    return 0;
}