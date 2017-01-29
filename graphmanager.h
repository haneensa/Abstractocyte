#ifndef GRAPHMANAGER_H
#define GRAPHMANAGER_H

#include "mainopengl.h"
#include "graph.h"

#define max_graphs 1

struct GraphUniforms {
    GLint y_axis;
    GLint x_axis;
    float* mMatrix;
    float* vMatrix;
    float* pMatrix;
};


class GraphManager : protected MainOpenGL
{
public:
    GraphManager();
    ~GraphManager();

    void drawNodes(struct GraphUniforms graph_uniforms, int graphIdx);
    void drawEdges(struct GraphUniforms graph_uniforms, int graphIdx);
    bool initVBO(struct GraphUniforms graph_uniforms, int graphIdx);
    void updateUniforms();

protected:
    // later if I need more independent graphs
    Graph                           *m_graph[max_graphs];
    int                             m_ngraph;

    // one for the 2D space
    QOpenGLVertexArrayObject        m_IndexVAO;
    QOpenGLBuffer                   m_IndexVBO;

    QOpenGLVertexArrayObject        m_NodesVAO;
    QOpenGLBuffer                   m_NodesVBO;

    // shaders
    GLuint                          m_program_nodes;
    GLuint                          m_program_Index;

    struct GraphUniforms            m_uniforms;

};

#endif // GRAPHMANAGER_H
