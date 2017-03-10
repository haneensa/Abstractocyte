// TODO: draw the boundry of grid

#ifndef SPATIALHASH_H
#define SPATIALHASH_H

#include <map>
#include <vector>
#include <set>
#include <unordered_map>

#include "mainopengl.h"
#include "node.h"
#include "glsluniform_structs.h"

class SpatialHash
{
public:
    SpatialHash(int col, float min, float max);
    ~SpatialHash();


    // insert point in grid
    std::pair<int, int> hash(float x, float y);
    std::set< std::pair<int, int> > hashAABB(float x, float y, float r);
    void insert(Node *node);
    void queryAABB(Node *node, float r, std::vector<Node*> &dataCell);
    void updateNode(Node *node);
    void clear();

protected:
     struct pair_hash {
         template <class T1, class T2>
         std::size_t operator () (const std::pair<T1,T2> &p) const {
             auto h1 = std::hash<T1>{}(p.first);
             auto h2 = std::hash<T2>{}(p.second);
             return h1 ^ h2;
         }
     };

    float   m_cellSize;
    float   m_min;
    float   m_max;
    float   m_col;
    std::unordered_map< std::pair<int, int>, std::vector<Node*>, pair_hash > m_hashMap;
};

#endif // SPATIALHASH_H
