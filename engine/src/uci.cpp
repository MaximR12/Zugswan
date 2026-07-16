#include <thread>
#include <format>
#include "uci.hpp"
#include "perft.hpp"
#include "search.hpp"
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
        m_worker = std::thread{[state = m_state, this, depth]() {
            Perft::run<Perft::divide>(state, depth);
            m_working = false;
        }};
    } else if(token == "depth") {
        int depth;
        args >> depth;
        m_worker = std::thread{[state = m_state, this, depth]() {
            Search::Search<SearchType::depth>(state, depth);
            m_working = false;
        }};
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

void UCI::bench(std::istringstream& args) {
    if(!m_working && m_worker.joinable())
        m_worker.join();
    else if(m_working)
        return;
    m_working = true;

    int hashSize, threads, limit;
    std::string position, type;
    args >> hashSize >> threads >> limit >> position >> type;

    if(type == "perft") {
        m_worker = std::thread{[state = m_state, this, limit]() {
            Perft::run<Perft::bench>(state, limit);
            m_working = false;
        }};
    }
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
        else if(token == "bench")
            bench(is);
        else if(token == "isready")
            std::cout << "readyok\n";

    } while(token != "quit");

    if(m_worker.joinable())
        m_worker.detach();
}