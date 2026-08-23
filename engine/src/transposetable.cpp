#include "transposetable.hpp"

void TranspositionTable::clear() {
    for(size_t i = 0; i < TT_SIZE; ++i) {
        Cluster& cluster = m_table[i];
        for(int j = 0; j < CLUSTER_SIZE; ++j) {
            cluster.entries[j].used = false;
            cluster.entries[j].eval = VALUE_UNDEFINED;
        }
    }
}

TransposeEntry* TranspositionTable::probe(uint64_t zobrist) {
    size_t index = TranspositionTable::getIndex(zobrist);
    assert(index < TT_SIZE);

    uint32_t key = TranspositionTable::getKey(zobrist);
    Cluster& cluster = m_table[index];
    for(int i = 0; i < CLUSTER_SIZE; ++i) {
        TransposeEntry& currEntry = cluster.entries[i];
        
        if(!currEntry.used)
            return nullptr;

        if(currEntry.key == key)
            return &currEntry; 
    }

    return nullptr;
}

void TranspositionTable::insert(uint64_t zobrist, NodeType type, Move best, uint8_t depth, int16_t score, int16_t eval) {
    size_t index = TranspositionTable::getIndex(zobrist);
    assert(index < TT_SIZE);

    int cIndex = -1, minDepth = INT_MAX, minDepthInd = 0;
    uint32_t key = TranspositionTable::getKey(zobrist);
    Cluster& cluster = m_table[index];
    for(int i = 0; i < CLUSTER_SIZE; ++i) {
        TransposeEntry& currEntry = cluster.entries[i];

        if(!currEntry.used) {
            cIndex = i;
            break;
        }

        if(key == currEntry.key) { //prioritize higher depth
            if(currEntry.depth >= depth)
                return; 
            else {
                cIndex = i;
                break;
            }
        }

        if(currEntry.depth < minDepth) {
            minDepth = currEntry.depth;
            minDepthInd = i;
        }
    }

    TransposeEntry& entry = cIndex == -1 ? cluster.entries[minDepthInd] : cluster.entries[cIndex]; //overwrite min depth node if cluster is full
    entry = {
        .key = key,
        .best = best,
        .score = score,
        .eval = eval,
        .depth = depth,
        .type = type,
        .used = true
    };
}

void PawnTable::clear() {
    for(int i = 0; i < PT_SIZE; ++i) {
        PawnCluster& cluster = m_table[i];
        for(int j = 0; j < PAWN_CLUSTER_SIZE; ++j)
            cluster.entries[j].used = false;
    }
}

PawnTableEntry* PawnTable::probe(uint64_t hash, uint16_t ply) {
    size_t index = PawnTable::getIndex(hash);
    assert(index < TT_SIZE);

    PawnCluster& cluster = m_table[index];
    for(int i = 0; i < PAWN_CLUSTER_SIZE; ++i) {
        PawnTableEntry& currEntry = cluster.entries[i];
        
        if(!currEntry.used)
            return nullptr;

        if(currEntry.pawnHash == hash) {
            currEntry.ply = std::max(ply, currEntry.ply);
            return &currEntry; 
        }
    }

    return nullptr;
}

void PawnTable::insert(uint64_t hash, uint16_t ply, int16_t score, int16_t kingBonus) {
    size_t index = getIndex(hash);
    PawnCluster& cluster = m_table[index];

    uint16_t greatestAge = -VALUE_INFINITE;
    PawnTableEntry& replace = cluster.entries[0];
    for(int i = 0; i < PAWN_CLUSTER_SIZE; ++i) {
        if(!cluster.entries[i].used) {
            cluster.entries[i] = {
                .pawnHash = hash,
                .ply = ply,
                .pawnScore = score,
                .kingSafetyBonus = kingBonus
            };

            return;
        }

        uint16_t currAge = ply - cluster.entries[i].ply; 
        if(currAge > greatestAge) {
            greatestAge = currAge;
            replace = cluster.entries[i]; 
        }
    }

    replace = {
        .pawnHash = hash,
        .ply = ply,
        .pawnScore = score,
        .kingSafetyBonus = kingBonus
    };
}