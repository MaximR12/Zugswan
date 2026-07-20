#include "transposetable.hpp"

TranspositionTable::TranspositionTable() {
    for(size_t i = 0; i < TT_SIZE; ++i) {
        Cluster& cluster = m_table[i];
        for(int j = 0; j < CLUSTER_SIZE; ++j)
            cluster.entries[j].used = false;
    }
}

TransposeEntry* TranspositionTable::probe(uint64_t zobrist) {
    size_t index = TranspositionTable::getIndex(zobrist);
    assert(index < TT_SIZE);

    Cluster& cluster = m_table[index];
    for(int i = 0; i < CLUSTER_SIZE; ++i) {
        TransposeEntry& currEntry = cluster.entries[i];
        
        if(!currEntry.used)
            return nullptr;

        if(currEntry.zobrist == zobrist)
            return &currEntry; 
    }

    return nullptr;
}

void TranspositionTable::insert(uint64_t zobrist, NodeType type, Move best, uint8_t depth, uint8_t score) {
    size_t index = TranspositionTable::getIndex(zobrist);
    assert(index < TT_SIZE);

    int cIndex = -1, minDepth = INT_MAX, minDepthInd = 0;
    Cluster& cluster = m_table[index];
    for(int i = 0; i < CLUSTER_SIZE; ++i) {
        TransposeEntry& currEntry = cluster.entries[i];

        if(!currEntry.used) {
            cIndex = i;
            break;
        }

        if(zobrist == currEntry.zobrist) { //prioritize higher depth
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
        .zobrist = zobrist,
        .best = best,
        .depth = depth,
        .score = score,
        .type = type,
        .used = true
    };
}