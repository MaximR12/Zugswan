#include "Game.h"
#include "GraphicsIncludes.h"
#include "Perft.h"
#include "sstream"
#include <thread>
#include <unordered_map>

void Game::handleClick(int square) {
    std::lock_guard<std::mutex> stateGuard(m_stateLock);
    m_gameState.handleClick(square);
    std::lock_guard<std::mutex> renderGuard(m_renderLock);
    m_gameState.updateRenderState(m_renderState);
}

void Game::unMakeMove() {
    std::lock_guard<std::mutex> stateGuard(m_stateLock);
    m_gameState.unMakeMove();
    m_gameState.updateLegalMoves();
    std::lock_guard<std::mutex> renderGuard(m_renderLock);
    m_gameState.updateRenderState(m_renderState);
}

void renderLoop(GLFWwindow* window, GUI::Renderer* renderer) {
    glfwMakeContextCurrent(window);

    while(!glfwWindowShouldClose(window)) {
        renderer->RenderFrame(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

GameState copyState(GameState& state, std::mutex& stateLock) {
    std::lock_guard<std::mutex> guard{stateLock};
    return state;
}

void Game::divideCommand(std::stringstream& ss) {
    int depth;
    ss >> depth;
    GameState stateCopy = copyState(m_gameState, m_stateLock);
    runDivide(&stateCopy, depth);
}

void Game::perftBulkCommand(std::stringstream& ss) {
    int depth;
    ss >> depth;
    GameState stateCopy = copyState(m_gameState, m_stateLock);
    runPerftBulk(&stateCopy, depth);
}

void Game::perftCommand(std::stringstream& ss) {
    int depth;
    ss >> depth;
    GameState stateCopy = copyState(m_gameState, m_stateLock);
    runPerft(&stateCopy, depth);
}

void Game::loadPositionCommand(std::stringstream& ss) {
    std::string FEN;
    std::getline(ss, FEN);
    std::lock_guard<std::mutex> stateGuard(m_stateLock);
    m_gameState.loadPosition(FEN);
    m_gameState.updateRenderState(m_renderState);
}

void parseCommand(std::string command, Game* game) {
    std::stringstream ss(command);
    std::string type;
    ss >> type;

    std::unordered_map<std::string, void (Game::*)(std::stringstream&)> commandMap {
        {"perft", &Game::perftCommand}, {"perftbulk", &Game::perftBulkCommand}, {"divide", &Game::divideCommand}, {"position", &Game::loadPositionCommand}
    };

    if(commandMap.find(type) == commandMap.end()) {
        std::cout << "Invalid command.\n";
        return;
    }

    void (Game::*commandFunc)(std::stringstream&) = commandMap[type];
    (game->*commandFunc)(ss);
}

void commandLoop(GLFWwindow* window, Game* game) {
    std::string command;
    while(!glfwWindowShouldClose(window)) {
        std::getline(std::cin, command);
        parseCommand(command, game);
    }
}

void Game::run() {
    GLFWwindow* window = m_window.getWindow();
    glfwMakeContextCurrent(nullptr);
    std::thread renderThread(renderLoop, window, &m_renderer);
    std::thread commandThread(commandLoop, window, this);
    renderThread.detach(), commandThread.detach();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

int main() {
    Game game("Chess Engine");
    game.run(); 

    return 0;
}