#ifndef GRAPH_H
#define GRAPH_H

#include "node.h"
#include "edge.h"
#include <QDebug>
#include <QFile>

// store graph per object?

struct GraphNode
{
    QVector3D coord3D;
    QVector2D coord2D;
};

class Graph
{
public:
    Graph();
    ~Graph();

    bool loadNodes(QString filename);
    bool loadEdges(QString filename);

    Node* addNode(int nID, float x, float y, float z);
    Edge* addEdge(int nID1, int nID2);
    Node* getNode(int nID);

    std::map<int, Node*> getNodes() { return m_nodes; }
    std::map<int, Edge*> getEdges() { return m_edges; }

    int getNodesCount(){ return m_nodesCounter; }
    int getEdgesCount() { return m_edgesCounter; }
    int getDupEdgesCount() { return m_dupEdges; }

    std::map<int, Node*>::iterator getNodesBegin()  { return m_nodes.begin(); }
    std::map<int, Node*>::iterator getNodesEnd()    { return m_nodes.end(); }
    std::map<int, Edge*>::iterator getEdgesBegin()  { return m_edges.begin(); }
    std::map<int, Edge*>::iterator getEdgesEnd()    { return m_edges.end(); }



protected:
    std::map<int, Node*> m_nodes;
    std::map<int, Edge*> m_edges;

    // for opengl buffer
    std::vector<struct GraphNode> m_graphNodes;

    int m_nodesCounter;
    int m_edgesCounter;
    int m_dupEdges;

};

#endif // GRAPH_H
