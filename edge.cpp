#include "edge.h"

Edge::Edge(int eID, Node *node1, Node *node2, Edge_t etype)
{
    m_eID = eID;
    m_node1 = node1;
    m_node2 = node2;
    m_edge_t = etype;
}

Edge::~Edge()
{
    delete m_node1;
    delete m_node2;
}

int Edge::getID()
{
    return m_eID;
}

Node* Edge::getNode1()
{
    return m_node1;
}

Node* Edge::getNode2()
{
    return m_node2;
}

int Edge::getNode1Id()
{
    return m_node1->getID();
}

int Edge::getNode2Id()
{
    return m_node2->getID();
}

Edge_t Edge::getEdgeType()
{
    return m_edge_t;
}
