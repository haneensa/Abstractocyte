#ifndef NODE_H
#define NODE_H

#include <QVector3D>

#include "edge.h"

enum class Node_t { AXON, DENDRITE, BOUTON, SPINE, MITO, SYNAPSE, ASTROCYTE, GLYCOGEN };

class Edge;

class Node
{
public:
    Node(int nID, float x, float y, float z, Node_t node_type = Node_t::ASTROCYTE);
    ~Node();
    int getID()                 { return m_nID; }
    Node_t getNodeType()        { return m_node_t; }
    QVector3D get3DPosition()   {return QVector3D(m_x, m_y, m_z); }

    void addEdge(Edge *e);

protected:
    int     m_nID;

    // original 3D position
    float   m_x;
    float   m_y;
    float   m_z;

    Node_t m_node_t;

    std::map<int, Edge*> m_adjEdges;
};

#endif // NODE_H
