#ifndef GRAPHMANAGER_H
#define GRAPHMANAGER_H

#include "mainopengl.h"
#include "graph.h"
#include "objectmanager.h"
#include "glsluniform_structs.h"
#include <thread>


// (1) neurite-neurite graph (2) neurite-astrocyte skeleton (3) neurites skeletons - astrocyte skeleton (4) neuries skeletons
#define max_graphs 4

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
    void updateUniforms(struct GlobalUniforms graph_uniforms);

    void initVertexAttribPointer();

    // force directed layout
    void startForceDirectedLayout(int graphIdx);
    void stopForceDirectedLayout(int graphIdx);

    void drawGrid(struct GlobalUniforms grid_uniforms);
    void initGrid();
    void update2Dflag(bool is2D);

    void updateGraphParam1(double value);
    void updateGraphParam2(double value);
    void updateGraphParam3(double value);
    void updateGraphParam4(double value);
    void updateGraphParam5(double value);
    void updateGraphParam6(double value);
    void updateGraphParam7(double value);

protected:
    // later if I need more independent graphs
    Graph                           *m_graph[max_graphs];
    std::vector< struct Skeleton_Node > m_skeletons_data; // all skeletons here
    std::vector< struct Neurite_Node > m_neurites_nodes; // all neurites centers init here


    bool                            m_glFunctionsSet;

    // one for the 2D space
    QOpenGLVertexArrayObject        m_IndexVAO;
    QOpenGLBuffer                   m_IndexVBO;

    QOpenGLVertexArrayObject        m_NodesVAO;
    QOpenGLBuffer                   m_NodesVBO;

    // shaders
    GLuint                          m_program_nodes;
    GLuint                          m_program_Index;

    struct GlobalUniforms            m_uniforms;

    // thread management
    std::thread                     m_layout_thread1;
    bool                            m_FDL_running;
    bool                            m_2D;

    // todo: flag that stops the FDL and kill the thread
    // todo: ?
};

#endif // GRAPHMANAGER_H
