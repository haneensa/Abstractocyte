#ifndef GRAPHMANAGER_H
#define GRAPHMANAGER_H

#include "mainopengl.h"
#include "graph.h"
#include "objectmanager.h"

#include <thread>


// (1) neurite-neurite graph (2) neurite-astrocyte skeleton (3) neurites skeletons - astrocyte skeleton (4) neuries skeletons
#define max_graphs 4

struct GraphUniforms {
    GLint y_axis;
    GLint x_axis;
    float* mMatrix;
    float* vMatrix;
    float* pMatrix;
    float* modelNoRotMatrix;
    QMatrix4x4 rMatrix;
};


class GraphManager : public MainOpenGL
{
public:
    GraphManager();
    ~GraphManager();

    void ExtractGraphFromMesh(ObjectManager *instance);

    bool initOpenGLFunctions();
    void drawNodes(int graphIdx);
    void drawEdges(int graphIdx);
    bool initVBO(int graphIdx);
    void updateUniformsLocation(GLuint program);
    void updateUniforms(struct GraphUniforms graph_uniforms);

    void initVertexAttribPointer();

    // force directed layout
    void startForceDirectedLayout(int graphIdx);
    void stopForceDirectedLayout(int graphIdx);

    void drawGrid(struct GridUniforms grid_uniforms);
    void initGrid();

protected:
    // later if I need more independent graphs
    Graph                           *m_graph[max_graphs];
    bool                            m_glFunctionsSet;

    // one for the 2D space
    QOpenGLVertexArrayObject        m_IndexVAO;
    QOpenGLBuffer                   m_IndexVBO;

    QOpenGLVertexArrayObject        m_NodesVAO;
    QOpenGLBuffer                   m_NodesVBO;

    // shaders
    GLuint                          m_program_nodes;
    GLuint                          m_program_Index;

    struct GraphUniforms            m_uniforms;

    // thread management
    std::thread                     m_layout_thread1;
    bool                            m_FDL_running;

    // todo: flag that stops the FDL and kill the thread
    // todo: ?

    // graphs elements
    std::map<int, Object*>            m_neurites_nodes_info;
    std::vector<QVector2D>            m_nerites_edges_info;

};

#endif // GRAPHMANAGER_H
