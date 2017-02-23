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
    GraphManager(ObjectManager *objectManager);
    ~GraphManager();

    void ExtractGraphFromMesh();

    bool initOpenGLFunctions();

    void drawNeuritesGraph();
    void drawSkeletonsGraph();

    bool initVBO();
    void updateUniformsLocation(GLuint program);
    void updateUniforms(struct GlobalUniforms graph_uniforms);

    void initNeuritesVertexAttribPointer();
    void initSkeletonsVertexAttribPointer();

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
    ObjectManager                       *m_obj_mngr;
    Graph                               *m_graph[max_graphs];

    bool                                m_glFunctionsSet;

    // one for the 2D Neurites node abstraction space
    QOpenGLVertexArrayObject            m_NeuritesGraphVAO;
    QOpenGLBuffer                       m_NeuritesIndexVBO;
    QOpenGLBuffer                       m_NeuritesNodesVBO;

    // shaders
    GLuint                              m_program_neurites_nodes;
    GLuint                              m_program_neurites_index;

    // one for the 2D Neurites node abstraction space
    QOpenGLVertexArrayObject            m_SkeletonsGraphVAO;
    QOpenGLBuffer                       m_SkeletonsIndexVBO;
    QOpenGLBuffer                       m_SkeletonsNodesVBO;

    // shaders
    GLuint                              m_program_skeletons_nodes;
    GLuint                              m_program_skeletons_index;

    struct GlobalUniforms               m_uniforms;

    // thread management
    std::thread                         m_layout_thread1;
    bool                                m_FDL_running;
    bool                                m_2D;
};

#endif // GRAPHMANAGER_H
