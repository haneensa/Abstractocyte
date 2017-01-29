#include "node.h"

Node::Node(int nID, int idxID, float x, float y, float z, Node_t node_type)
{
    m_nID = nID;
    m_idxID = idxID;
    m_x = x;
    m_y = y;
    m_z = z;
    m_node_t = node_type;
}

Node::~Node()
{

}

void Node::addEdge(Edge *e)
{
    if (e == NULL)
        return;

    // check if and end of this edge is this node
    if (e->getNode1Id() == m_nID) {
        if (m_adjEdges.count(e->getNode2Id()) == 0)
            m_adjEdges[e->getNode2Id()] = e;
    } else {
        if (m_adjEdges.count(e->getNode1Id()) == 0)
            m_adjEdges[e->getNode1Id()] = e;
    }
}
