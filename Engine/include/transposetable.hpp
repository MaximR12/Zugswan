#pragma once

#include "board.hpp"

constexpr size_t DEFAULT_HASH_SIZE = 64 * 1024 * 1024; //64MB

enum class NodeType : uint8_t {
    exact, upper, lower
};

struct TransposeEntry {
    uint64_t zobrist;
    Move best;
    int16_t score;
    uint8_t depth;
    NodeType type;
    bool used;
};

constexpr size_t CLUSTER_SIZE = 4; //each cluster fits in one 64 byte cache line

struct Cluster {
    TransposeEntry entries[CLUSTER_SIZE];
};

constexpr size_t TT_SIZE = DEFAULT_HASH_SIZE/sizeof(Cluster); 
constexpr size_t INDEX_MASK = TT_SIZE-1;

class TranspositionTable {
private:
    Cluster m_table[TT_SIZE];

public:
    TranspositionTable() { clear(); }

    TransposeEntry* probe(uint64_t zobrist);
    void insert(uint64_t zobrist, NodeType type, Move best, uint8_t depth, int16_t score);
    void clear();

    static size_t getIndex(uint64_t zobrist) { return zobrist&INDEX_MASK; }
};