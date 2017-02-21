// how to represent a whole skeleton in a graph?
// all the nodes of a skeleton should have the same hvgx id
// so I cant index the map with it

#ifndef GRAPH_H
#define GRAPH_H


#define PI 3.14159265358979f

#include "node.h"
#include "edge.h"
#include "spatialhash.h"
#include "objectmanager.h"

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
    QVector3D coord3D; // vertex center, todo: w: ID
    QVector2D coord2D; // layouted coordinate
    int ID;
};

/*
    5-----1----2
    |     |    |
    |     3----4
    |          |
    |          |
    6----------7

  5: (N) Skeleton_Node->vertex, (A) alpha = 0
  6: (N) Skeleton_Node->vertex, (A) Skeleton_Node->vertex
  7: (N) struct ssbo_mesh->center, (A) Skeleton_Node->vertex
  1: (N) Skeleton_Node->layout3, (A) alpha = 0
  2: (N) struct ssbo_mesh->layout 1, (A) alpha = 0
  3: (N) Skeleton_Node->layout 1, (A) Skeleton_Node->layout 1
  4: (N) struct ssbo_mesh->layout 2, (A) Skeleton_Node->layout 2
*/



// allocate vbo with enough space for all skeletons using this struct
// use this for all skeleton stages (simplified one)
struct Skeleton_Node {
    QVector4D vertex;   // original position  (simplified)  -> w: hvgx ID
    QVector2D layout1;  // layouted position (all skeletons)
    QVector2D layout2;  // layouted position (no neurites)
    QVector2D layout3;  // layouted position (no astrocyte)
};

// edge info is always present. only display it in certain states

/*
 * NODE_NODE:
 * nodes IDs are hvgx ID
 * edges simple edge between nodes using hvgx ID
 * update: struct ssbo_mesh->layout 1
 *
 * NODE_SKELETON:
 * update: struct ssbo_mesh->layout 2
 * update: Skeleton_Node->layout 2
 *
 * ALL_SKELETONS:
 * update: Skeleton_Node->layout 1
 *
 * NEURITE_SKELETONS:
 * update: Skeleton_Node->layout 3
 *
 */
enum class Graph_t { NODE_NODE = 0, NODE_SKELETON = 1,  ALL_SKELETONS = 2, NEURITE_SKELETONS =3};


class Graph
{
public:
    Graph(Graph_t graphType);
    ~Graph();

    bool createGraph(ObjectManager *objectManager);
    bool parseNODE_NODE(ObjectManager *objectManager);
    bool parseSKELETON(ObjectManager *objectManager);

    Node* addNode(std::pair<int, int> id_tuple, float x, float y, float z);
    Edge* addEdge(int eID, int hvgxID, int nID1, int nID2);

    Node* getNode(std::pair<int, int> id_tuple);
    Edge* getEdge(int eID);


    std::map<std::pair<int, int>, Node*> getNodes() { return m_nodes; }
    std::map<int, Edge*> getEdges() { return m_edges; }

    int getNodesCount(){ return m_nodesCounter; }
    int getEdgesCount() { return m_edgesCounter; }
    int getDupEdgesCount() { return m_dupEdges; }

    std::map<std::pair<int, int>, Node*>::iterator getNodesBegin()  { return m_nodes.begin(); }
    std::map<std::pair<int, int>, Node*>::iterator getNodesEnd()    { return m_nodes.end(); }
    std::map<int, Edge*>::iterator getEdgesBegin()  { return m_edges.begin(); }
    std::map<int, Edge*>::iterator getEdgesEnd()    { return m_edges.end(); }

    // force directed layout functions
    void runforceDirectedLayout();
    void attractConnectedNodes(Edge *edge, float k);
    void repulseNodes(Node *node1, Node *node2, float k);
    QVector2D attractionForce(float x1, float y1, float x2, float y2, float k);
    QVector2D repulsiveForce(float x1, float y1, float x2, float y2, float k);
    void resetCoordinates(QMatrix4x4 rotationMatrix);

    void update_node_data(Node* node);

    // opengl related functions
    size_t vertexBufferSize() { return m_bufferNodes.size(); }
    size_t indexBufferSize() { return m_bufferIndices.size(); }

    void allocateBVertices(QOpenGLBuffer vertexVbo);
    void allocateBIndices(QOpenGLBuffer indexVbo);

    void terminateFDL()  { m_FDL_terminate = true; }

    // spatial hashing
    void updateNode(Node *node);
    void drawGrid(struct GlobalUniforms grid_uniforms);
    void initGridBuffers();
    void updateGridUniforms(struct GlobalUniforms grid_uniforms);

    void updateGraphParam1(double value);
    void updateGraphParam2(double value);
    void updateGraphParam3(double value);
    void updateGraphParam4(double value);
    void updateGraphParam5(double value);
    void updateGraphParam6(double value);
    void updateGraphParam7(double value);


    // GEM
    void initGemParameters();
    void runGEM();
    double computeImpulse(/*vertex*/);
    double function_growing(/*vertex*/);
    void update_node(/*vertex and impulse*/);
    double get_angle(/*vertex 1, vertex 2*/);

protected:
    struct pair_hash {
        template <class T1, class T2>
        std::size_t operator () (const std::pair<T1,T2> &p) const {
            auto h1 = std::hash<T1>{}(p.first);
            auto h2 = std::hash<T2>{}(p.second);
            return h1 ^ h2;
        }
    };

    Graph_t                                         m_gType;

    ObjectManager                                   *m_obj_mngr;

    std::map<std::pair<int, int>, Node*>            m_nodes;    // IDs are unique to identify nodes
                                                // if more than a skeleton
                                                // IDs for skeleton are offsets, that is later
                                                // stored in the skeleton itself

    std::map<int, Edge*>            m_edges;    // IDs refer to m_nodes IDs

    std::map<int, Skeleton*>        m_skeletons; // ID are hvgx

    int m_nodesCounter;
    int m_edgesCounter;
    int m_dupEdges;

    // for opengl buffer
    std::vector<struct BufferNode>  m_bufferNodes;
    std::vector<GLuint>             m_bufferIndices;

    bool                            m_FDL_terminate;

    // spatial hashing
    SpatialHash                     *hashGrid;


    // force directed laout
    float                           m_MAX_DISTANCE;
    float                           m_AABBdim;
    float                           m_Cr;
    float                           m_Ca;
    int                             m_ITERATIONS;
    float                           m_MAX_VERTEX_MOVEMENT;
    float                           m_SLOW_FACTOR;
    float                           m_MAX_FORCE;

    // GEM constants
    float                           m_nrm;  // normalization factor
    float                           m_gravity;  // gravitational constant
    float                           m_edge_size;    // desired edge size
    int                             m_Tmin;         // min temperature
    int                             m_Tmax;
    float                           m_a_r;          // PI/6
    float                           m_a_o;          // PI/2
    float                           m_s_r;          // 1/2n
    float                           m_s_o;          // 1/3
    float                           m_Tinit;        // initial temperature for a vertex
    float                           m_Tglobal;      // Tinit * n (temperature sum)
    int                             m_rounds;       // max number of rounds


};

#endif // GRAPH_H
