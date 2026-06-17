#include <thread>
#include <format>
#include "uci.hpp"
#include "perft.hpp"
#include "sstream"

void UCI::go(std::istringstream& args) {
    if(!m_working && m_worker.joinable())
        m_worker.join();
    else if(m_working)
        return;
    m_working = true;
    
    std::string token;
    args >> token;

    if(token == "perft") {
        int depth;
        args >> depth;
        GameState* stateCopy = new GameState{*m_state};
        m_worker = std::thread{&UCI::perft, this, stateCopy, depth};
    } 
}

void UCI::position(std::istringstream& args) {
    std::string token;
    args >> token;

    if(token == "fen") {
        std::string pos, side, castleRights, epTarget, halfMove, fullMove;  
        args >> pos >> side >> castleRights >> epTarget >> halfMove >> fullMove;
        m_state->loadPosition(std::format("{} {} {} {} {} {}", pos, side, castleRights, epTarget, halfMove, fullMove));
    } else if(token == "startpos") 
        m_state->loadStartPos();

    std::string moves;
    if(!(args >> moves) || moves != "moves")
        return;

    std::vector<std::string> moveList;
    moveList.reserve(64);
    while(args >> moveList.emplace_back());

    moveList.pop_back();
    m_state->moveFromList(moveList);
}

void UCI::perft(GameState* stateCopy, int depth) {
    Perft::run(stateCopy, depth);
    delete stateCopy;
    m_working = false;
}

void UCI::run() {
    std::string command, token;

    do {
        std::getline(std::cin, command);
        std::istringstream is(command);
        
        token.clear();
        is >> token;

        if(token == "go")
            go(is);
        else if(token == "position")
            position(is);
        else if(token == "isready")
            if(!m_working) std::cout << "readyok\n";

    } while(token != "quit");

    if(m_worker.joinable())
        m_worker.detach();
}