#include <thread>
#include "gamestate.hpp"

class UCI {
private:
    GameState* m_state;
    std::thread m_worker;
    std::atomic<bool> m_working;

    void go(std::istringstream& args);
    void position(std::istringstream& args);
    void bench(std::istringstream& args);

public:
    UCI(GameState* state) : m_state{state}, m_worker{}, m_working{false} { }

    void run();
};