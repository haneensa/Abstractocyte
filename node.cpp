#include "node.h"

Node::Node(int nID, float x, float y, float z, Node_t node_type)
{
    m_nID = nID;
    m_x = x;
    m_y = y;
    m_z = z;
    m_node_t = node_type;
}

Node::~Node()
{

}
