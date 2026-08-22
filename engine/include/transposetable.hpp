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

constexpr size_t TT_SIZE = DEFAULT_HASH_SIZE / sizeof(Cluster); 
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

constexpr size_t PAWN_HASH_SIZE = 4 * 1024 * 1024; //4 MB

struct PawnTableEntry {
    uint64_t pawnHash;
    uint16_t ply;
    int16_t pawnScore;
    int16_t kingSafetyBonus;
    bool used;
};

constexpr size_t PAWN_CLUSTER_SIZE = std::hardware_constructive_interference_size / sizeof(PawnTableEntry);

struct PawnCluster {
    PawnTableEntry entries[PAWN_CLUSTER_SIZE];
};

constexpr size_t PT_SIZE = PAWN_HASH_SIZE / sizeof(PawnCluster);
constexpr size_t PAWN_INDEX_MASK = PT_SIZE-1;

class PawnTable {
private:
    PawnCluster m_table[PT_SIZE];

public:
    PawnTable() { clear(); }

    PawnTableEntry* probe(uint64_t hash, uint16_t ply);
    void insert(uint64_t hash, uint16_t ply, int16_t score, int16_t kingBonus);
    void clear();

    static size_t getIndex(uint64_t hash) { return hash&PAWN_INDEX_MASK; }
};