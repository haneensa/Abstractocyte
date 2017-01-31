#ifndef NODE_H
#define NODE_H

#include <QVector3D>
#include <QVector2D>
#include <QMatrix4x4>
#include "edge.h"

enum class Node_t { AXON, DENDRITE, BOUTON, SPINE, MITO, SYNAPSE, ASTROCYTE, GLYCOGEN };

class Edge;

class Node
{
public:
    Node(int nID, int idxID, float x, float y, float z, Node_t node_type = Node_t::ASTROCYTE);
    ~Node();
    int getID()                 { return m_nID; }
    int getIdxID()              { return m_idxID; }

    Node_t getNodeType()        { return m_node_t; }
    QVector3D get3DPosition()   {return m_nodeXYZ; }
    void addEdge(Edge *e);


    // force based layout related functions
    QVector3D getLayoutedPosition();
    void addToLayoutedPosition(QVector2D newPos);
    void resetLayout(QMatrix4x4 rotationMatrix);

    void addToForceSum(QVector2D newForce);
    QVector2D getForceSum()     { return m_forceSum; }

    void resetForce();

    // hashmap
    void setHashMap(std::pair<int, int> cell);
    std::pair<int, int>  getHashMapCell();

private:
    int                     m_nID;
    int                     m_idxID;
    // original 3D position
    QVector3D               m_nodeXYZ;

    Node_t                  m_node_t;

    std::map<int, Edge*>    m_adjEdges;

    // force directed layout
    QVector2D               m_forceSum; // force to be applied on 2D position
    QVector3D               m_layouted; // 2D position after applying force

    // hashmap
    std::pair<int, int>     m_hashMapCell;

};

#endif // NODE_H
