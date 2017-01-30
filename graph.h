#ifndef GRAPH_H
#define GRAPH_H

#include "node.h"
#include "edge.h"
#include "spatialhash.h"

#include <QDebug>
#include <QFile>
#include <QVector2D>
#include <QVector3D>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

// store graph per object?

struct BufferNode
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
    Edge* addEdge(int eID, int nID1, int nID2);

    Node* getNode(int nID);
    Edge* getEdge(int eID);


    std::map<int, Node*> getNodes() { return m_nodes; }
    std::map<int, Edge*> getEdges() { return m_edges; }

    int getNodesCount(){ return m_nodesCounter; }
    int getEdgesCount() { return m_edgesCounter; }
    int getDupEdgesCount() { return m_dupEdges; }

    std::map<int, Node*>::iterator getNodesBegin()  { return m_nodes.begin(); }
    std::map<int, Node*>::iterator getNodesEnd()    { return m_nodes.end(); }
    std::map<int, Edge*>::iterator getEdgesBegin()  { return m_edges.begin(); }
    std::map<int, Edge*>::iterator getEdgesEnd()    { return m_edges.end(); }

    // force directed layout functions
    void runforceDirectedLayout();
    void attractConnectedNodes(Edge *edge, float k);
    void repulseNodes(Node *node1, Node *node2, float k);
    QVector2D attractionForce(float x1, float y1, float x2, float y2, float k);
    QVector2D repulsiveForce(float x1, float y1, float x2, float y2, float k);
    void resetCoordinates(QMatrix4x4 rotationMatrix);

    // opengl related functions
    size_t vertexBufferSize() { return m_bufferNodes.size(); }
    size_t indexBufferSize() { return m_bufferIndices.size(); }

    void allocateBVertices(QOpenGLBuffer vertexVbo);
    void allocateBIndices(QOpenGLBuffer indexVbo);

    void terminateFDL()  { m_FDL_terminate = true; }

protected:
    std::map<int, Node*>            m_nodes;
    std::map<int, Edge*>            m_edges;

    int m_nodesCounter;
    int m_edgesCounter;
    int m_dupEdges;

    // for opengl buffer
    std::vector<struct BufferNode>  m_bufferNodes;
    std::vector<GLuint>           m_bufferIndices;

    // force directed laout
    float                           m_MAX_DISTANCE;
    float                           m_AABBdim;
    float                           m_Cr;
    float                           m_Ca;
    int                             m_ITERATIONS;
    float                           m_MAX_VERTEX_MOVEMENT;
    float                           m_SLOW_FACTOR;
    float                           m_MAX_FORCE;

    bool                            m_FDL_terminate;

};

#endif // GRAPH_H
