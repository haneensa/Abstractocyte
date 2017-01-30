#ifndef GRAPHMANAGER_H
#define GRAPHMANAGER_H

#include "mainopengl.h"
#include "graph.h"

#include <thread>



#define max_graphs 1

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

    bool initOpenGLFunctions();
    void drawNodes(int graphIdx);
    void drawEdges(int graphIdx);
    bool initVBO(int graphIdx);
    void updateUniformsLocation();
    void updateUniforms(struct GraphUniforms graph_uniforms);

    // force directed layout
    void startForceDirectedLayout(int graphIdx);
    void stopForceDirectedLayout(int graphIdx);

protected:
    // later if I need more independent graphs
    Graph                           *m_graph[max_graphs];
    int                             m_ngraph;
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
};

#endif // GRAPHMANAGER_H
