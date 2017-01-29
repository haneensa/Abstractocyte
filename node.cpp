#include "node.h"

Node::Node(int nID, int idxID, float x, float y, float z, Node_t node_type)
{
    m_nID = nID;
    m_idxID = idxID;
    m_nodeXYZ = QVector3D(x, y, z);
    m_node_t = node_type;

    // force directed layout
    m_forceSum = QVector2D(0.0, 0.0);
    m_layouted = QVector2D(x, y);
}

Node::~Node()
{
    // delete the edges?
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

// force based layout related functions
QVector2D Node::getLayoutedPosition()
{
    return m_layouted;
}


void Node::addToLayoutedPosition(QVector2D newPos)
{
    m_layouted += newPos;
}

void Node::resetLayout(QMatrix4x4 rotationMatrix)
{
    QVector3D vec = rotationMatrix * m_nodeXYZ;
    m_layouted = vec.toVector2D();
    resetForce();
}

void Node::addToForceSum(QVector2D newForce)
{
    m_forceSum += newForce;
}

void Node::resetForce()
{
    m_forceSum = QVector2D(0.0, 0.0);
}
