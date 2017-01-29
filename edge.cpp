#include "edge.h"

Edge::Edge(int eID, int idxID, Node *node1, Node *node2, Edge_t etype)
{
    // 1) m_eID local to the file, not unique restricted
    // 2) m_idxID global to the graph class that is sequential and unique
    m_eID = eID;
    m_idxID = idxID;
    m_node1 = node1;
    m_node2 = node2;
    m_edge_t = etype;
}

Edge::~Edge()
{
    delete m_node1;
    delete m_node2;
}


int Edge::getNode1Id()
{
    return m_node1->getID();
}

int Edge::getNode2Id()
{
    return m_node2->getID();
}
