#ifndef EDGE_H
#define EDGE_H

#include "node.h"

// store the points of a segment, starting from the knots and the points in between
// here we get the indices that connect these points
// slowly lower the resolution until we go to 2D skeleton

enum class Edge_t { ASTRO_SKELETON, NEURITE_SKELETON, SYNAPSE, PARENT_CHILD };

class Node;
class Edge
{
public:
    Edge(int eID, int idxID, Node *node1 = 0, Node *node2 = 0, Edge_t etype = Edge_t::ASTRO_SKELETON);
    ~Edge();
    int getID()                 { return m_eID; }
    int getIdxID()              { return m_idxID; }
    Node* getNode1()            { return m_node1; }
    Node* getNode2()            { return m_node2; }
    Edge_t getEdgeType()        { return m_edge_t; }
    int getNode2Id();
    int getNode1Id();


protected:
    int     m_eID;
    int     m_idxID;
    Edge_t  m_edge_t;
    Node*   m_node1;
    Node*   m_node2;
    // list of nodes in between
    std::vector<Node*> nodesList;
};

#endif // EDGE_H
