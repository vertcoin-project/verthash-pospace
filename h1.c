#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define NODE_SIZE 32

struct Graph {
    FILE* db;
    int64_t log2;
    int64_t pow2;
};

int64_t Log2(int64_t x) {
    int64_t r = 0;
    for(; x > 1; x >>= 1) {
        r++;
    }
    
    return r;
}

int64_t subtree(struct Graph* g, const int64_t node) {
    int64_t level = (g->log2 + 1) - Log2(node);
    return (int64_t)((1 << (uint64_t)level) - 1);
}

int64_t bfsToPost(struct Graph* g, const int64_t node) {
    if(node == 0) {
        return 0;
    }
    
    int64_t cur = node;
    int64_t res = 0;
    
    while(cur != 1) {
        if(cur % 2 == 0) {
            res -= subtree(g, cur) + 1;
        } else {
            res--;
        }
        cur /= 2;
    }
    res += 2*g->pow2 - 1;
    
    return res;
}

void WriteId(struct Graph* g, uint8_t* Node, const int64_t id) {
    fseek(g->db, id*NODE_SIZE, SEEK_SET);
    fwrite(Node, 1, NODE_SIZE, g->db);
}

void WriteNode(struct Graph* g, uint8_t* Node, const int64_t id) {
    const int64_t idx = bfsToPost(g, id);
    WriteId(g, Node, idx);
}

void NewNode(struct Graph* g, const int64_t id, uint8_t* hash) {
    WriteNode(g, hash, id);
}

uint8_t* GetId(struct Graph* g, const int64_t id) {
    fseek(g->db, id*NODE_SIZE, SEEK_SET);
    uint8_t* node = malloc(NODE_SIZE);
    fread(node, 1, NODE_SIZE, g->db);
    return node;
}

uint8_t* GetNode(struct Graph* g, const int64_t id) {
    const int64_t idx = bfsToPost(g, id);
    return GetId(g, idx);
}

/*
func (g *Graph) ButterflyGraph(index int64, count *int64) {
	if index == 0 {
		index = 1
	}
	numLevel := 2 * index
	perLevel := int64(1 << uint64(index))
	begin := *count - perLevel // level 0 created outside
	// no parents at level 0
	var level, i int64
	for level = 1; level < numLevel; level++ {
		for i = 0; i < perLevel; i++ {
			var prev int64
			shift := index - level
			if level > numLevel/2 {
				shift = level - numLevel/2
			}
			if (i>>uint64(shift))&1 == 0 {
				prev = i + (1 << uint64(shift))
			} else {
				prev = i - (1 << uint64(shift))
			}
			parent0 := g.GetNode(begin + (level-1)*perLevel + prev)
			parent1 := g.GetNode(*count - perLevel)

			ph := append(parent0.H, parent1.H...)
			buf := make([]byte, hashSize)
			binary.PutVarint(buf, *count)
			val := append(g.pk, buf...)
			val = append(val, ph...)
			hash := sha3.Sum256(val)

			g.NewNode(*count, hash[:])
			*count++
		}
	}
}
*/

void ButterflyGraph(struct Graph* g, int64_t index, int64_t* count) {
    if(index == 0) {
        index = 1;
    }
    
    int64_t numLevel = 2*index;
    int64_t perLevel = (int64_t)(1 << (uint64_t)index);
    int64_t begin = *count - perLevel;
    int64_t level, i;
  
    for(level = 1; level < numLevel; level++) {
        for(i = 0; i < perLevel; i++) {
            int64_t prev;
            int64_t shift = index - level;
            if(level > numLevel/2) {
                shift = level - numLevel/2;
            }
            if(((i>>(uint64_t)shift)&1) == 0) {
                prev = i + (1 << (uint64_t)shift);
            } else {
                prev = i - (1 << (uint64_t)shift);
            }
            
            uint8_t* parent0 = GetNode(g, begin+(level-1)*perLevel + prev);
            uint8_t* parent1 = GetNode(g, *count - perLevel);
            
            
        }
    }
}
