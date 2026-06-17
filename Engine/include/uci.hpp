#include <thread>
#include "gamestate.hpp"
#include "engine.hpp"

class UCI {
private:
    GameState* m_state;
    Engine* m_engine;
    std::thread m_worker;
    std::atomic<bool> m_working;

    void go(std::istringstream& args);
    void position(std::istringstream& args);
    void perft(GameState* stateCopy, int depth);

public:
    UCI(GameState* state, Engine* engine) : m_state{state}, m_engine{engine}, m_worker{}, m_working{false} { }

    void run();
};