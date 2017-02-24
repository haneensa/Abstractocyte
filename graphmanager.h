// this should only handle the 4 graphs, their construction, layouting algorithms
// and updating their data, and drawing

#ifndef GRAPHMANAGER_H
#define GRAPHMANAGER_H

#include "graph.h"
#include "datacontainer.h"
#include "openglmanager.h"
#include "glsluniform_structs.h"
#include <thread>


// (1) neurite-neurite graph
// (2) neurite-astrocyte skeleton
// (3) neurites skeletons - astrocyte skeleton
// (4) neuries skeletons
#define max_graphs 4

class GraphManager
{
public:
    GraphManager(DataContainer *objectManager, OpenGLManager *opengl_mnger);
    ~GraphManager();

    void ExtractGraphFromMesh();

    void drawNeuritesGraph();
    void drawSkeletonsGraph();

    void updateUniformsLocation(GLuint program);
    void updateUniforms(struct GlobalUniforms graph_uniforms);

    void initNeuritesVertexAttribPointer();
    void initSkeletonsVertexAttribPointer();

    // force directed layout
    void startForceDirectedLayout(int graphIdx);
    void stopForceDirectedLayout(int graphIdx);

    void update2Dflag(bool is2D, struct GlobalUniforms uniforms);

    void updateGraphParam1(double value);
    void updateGraphParam2(double value);
    void updateGraphParam3(double value);
    void updateGraphParam4(double value);
    void updateGraphParam5(double value);
    void updateGraphParam6(double value);
    void updateGraphParam7(double value);

protected:
    DataContainer                       *m_data_containter;
    OpenGLManager                       *m_opengl_mngr;
    Graph                               *m_graph[max_graphs];

    // thread management
    std::thread                         m_layout_thread1;
    bool                                m_FDL_running;
    bool                                m_2D;
};

#endif // GRAPHMANAGER_H
