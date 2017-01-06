#ifndef EDGE_H
#define EDGE_H

#include "node.h"

enum class Edge_t { ASTRO_SKELETON, NEURITE_SKELETON, NEURITE_NEURITE };

class Edge
{
public:
    Edge(int eID, Node *node1 = 0, Node *node2 = 0, Edge_t etype = Edge_t::ASTRO_SKELETON);
    ~Edge();
    int getID();
    Node* getNode1();
    Node* getNode2();
    int getNode2Id();
    int getNode1Id();
    Edge_t getEdgeType();

protected:
    int     m_eID;
    Edge_t  m_edge_t;
    Node*   m_node1;
    Node*   m_node2;
};

#endif // EDGE_H
