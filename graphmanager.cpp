#include "graphmanager.h"

GraphManager::GraphManager()
    : m_IndexVBO(QOpenGLBuffer::IndexBuffer),
      m_NodesVBO( QOpenGLBuffer::VertexBuffer ),
      m_ngraph(0),
      m_glFunctionsSet(false),
      m_FDL_running(false)
{
    if (m_ngraph < max_graphs)  {
        m_graph[m_ngraph] = new Graph();

        // todo: load per segment the segments points defining the curve, and find a way to render them as part of the sekeleton
//        m_graph[m_ngraph]->loadNodes("://data/skeleton_astrocyte_m3/skeleton_astro_nodes.csv");
//        m_graph[m_ngraph]->loadEdges("://data/skeleton_astrocyte_m3/skeleton_astro_segments.csv");

//        m_graph[m_ngraph]->loadNodes("://data/skeleton_astrocyte_m3/skeleton_astro_points_2000offsets.csv");
      //  m_graph[m_ngraph]->loadNodes("://data/skeleton_astrocyte_m3/skeleton_astro_points.csv");
//        m_graph[m_ngraph]->loadEdges("://data/skeleton_astrocyte_m3/points_segments.csv");

        // test connectivity info
        m_graph[m_ngraph]->loadNodes("://data/originalData/processed_data/nodesList.csv");
       // m_graph[m_ngraph]->loadEdges("://data/originalData/processed_data/connectivityList.csv");

        m_ngraph++;
    }

}

GraphManager::~GraphManager()
{
    qDebug() << "~GraphManager()";
    stopForceDirectedLayout(0);
    if (m_layout_thread1.joinable()) {
        m_layout_thread1.join();
    }

    // destroy all vbo and vao and programs and graphs

    if (m_glFunctionsSet == true) {

        m_NodesVAO.destroy();
        m_NodesVBO.destroy();

        m_IndexVAO.destroy();
        m_IndexVBO.destroy();
    }

    delete m_graph[0]; // todo: if more than one iterate over all

}

void GraphManager::stopForceDirectedLayout(int graphIdx)
{
    m_graph[graphIdx]->terminateFDL();
    m_FDL_running = false; // todo: get this from the graph itself, since it is per graph, even the thread?
    updateUniformsLocation();
    if (m_layout_thread1.joinable()) {
        m_layout_thread1.join();
    }

}

void GraphManager::startForceDirectedLayout(int graphIdx)
{
    // update the 2D node position at the start and whenever the m_mvp change, -> when m_value == 1.0 and m_mvp changed
    stopForceDirectedLayout(graphIdx);

    // reset graph
    m_graph[graphIdx]->resetCoordinates(m_uniforms.rMatrix);


    m_FDL_running = true;
    m_layout_thread1 = std::thread(&Graph::runforceDirectedLayout, m_graph[graphIdx]);

}

bool GraphManager::initOpenGLFunctions()
{
    m_glFunctionsSet = true;
    initializeOpenGLFunctions();

    return true;
}


// we initialize the vbos for drawing
bool GraphManager::initVBO(int graphIdx)
{
    if (m_ngraph < graphIdx) {
        qDebug() << "graph index out of range";
        return false;
    }

    if (m_glFunctionsSet == false)
        return false;

    qDebug() << "graph->initVBO";

    // 1) initialize shaders
    m_program_nodes = glCreateProgram();
    bool res = initShader(m_program_nodes,  ":/shaders/nodes.vert", ":/shaders/nodes.geom", ":/shaders/nodes.frag");
    if (res == false)
        return res;

    m_program_Index = glCreateProgram();
    res = initShader(m_program_Index,  ":/shaders/nodes.vert", ":/shaders/lines.geom", ":/shaders/lines.frag");
    if (res == false)
        return res;

    GL_Error();

    // initialize buffers
    m_NodesVAO.create();
    m_NodesVAO.bind();

    m_NodesVBO.create();
    m_NodesVBO.setUsagePattern( QOpenGLBuffer::DynamicDraw );
    m_NodesVBO.bind();
    m_graph[graphIdx]->allocateBVertices(m_NodesVBO);


    glUseProgram(m_program_nodes);
    GL_Error();


    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(struct BufferNode),  0);
    int offset = sizeof(QVector3D);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          sizeof(struct BufferNode),  (void*)offset);
    GL_Error();

    // initialize uniforms
    updateUniformsLocation();

    m_NodesVBO.release();
    m_NodesVAO.release();


    m_IndexVAO.create();
    m_IndexVAO.bind();
    m_NodesVBO.bind();

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(struct BufferNode),  0);
    offset = sizeof(QVector3D);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          sizeof(struct BufferNode),  (void*)offset);

    m_NodesVBO.release();


    m_IndexVBO.create();
    m_IndexVBO.bind();
    m_graph[graphIdx]->allocateBIndices(m_IndexVBO);

    glUseProgram(m_program_Index);
    updateUniformsLocation();


    m_IndexVBO.release();
    m_IndexVAO.release();

    return true;
}

void GraphManager::initGrid()
{
    m_graph[0]->initGridBuffers();
}

void GraphManager::drawGrid(struct GridUniforms grid_uniforms)
{
    m_graph[0]->drawGrid(grid_uniforms);
}


void GraphManager::drawNodes(int graphIdx)
{

    if (m_ngraph < graphIdx) {
        qDebug() << "graph index out of range";
        return;
    }

    if (m_glFunctionsSet == false)
        return;

    m_NodesVAO.bind();
    m_NodesVBO.bind();
    m_graph[graphIdx]->allocateBVertices(m_NodesVBO);

    glUseProgram(m_program_nodes);
    updateUniformsLocation();

    glDrawArrays(GL_POINTS, 0, m_graph[graphIdx]->vertexBufferSize() );
    m_NodesVBO.release();
    m_NodesVAO.release();
}

void GraphManager::drawEdges(int graphIdx)
{
    if (m_ngraph < graphIdx) {
        qDebug() << "graph index out of range";
        return;
    }

    if (m_glFunctionsSet == false)
        return;

    m_IndexVAO.bind();
    m_NodesVBO.bind();
    m_IndexVBO.bind();

    glUseProgram(m_program_Index);
    updateUniformsLocation();

    glDrawElements(GL_LINES, m_graph[graphIdx]->indexBufferSize(), GL_UNSIGNED_INT, 0 );
    m_NodesVBO.release();
    m_IndexVBO.release();
    m_IndexVAO.release();
}

void GraphManager::updateUniformsLocation()
{
    if (m_glFunctionsSet == false)
        return;

    // initialize uniforms
    GLuint mMatrix = glGetUniformLocation(m_program_nodes, "mMatrix");
    if (m_FDL_running) { // force directed layout started, them use model without rotation
        glUniformMatrix4fv(mMatrix, 1, GL_FALSE, m_uniforms.modelNoRotMatrix);
    } else {
        glUniformMatrix4fv(mMatrix, 1, GL_FALSE, m_uniforms.mMatrix);
    }

    GLuint vMatrix = glGetUniformLocation(m_program_nodes, "vMatrix");
    glUniformMatrix4fv(vMatrix, 1, GL_FALSE, m_uniforms.vMatrix);

    GLuint pMatrix = glGetUniformLocation(m_program_nodes, "pMatrix");
    glUniformMatrix4fv(pMatrix, 1, GL_FALSE, m_uniforms.pMatrix);


}

void GraphManager::updateUniforms(struct GraphUniforms graph_uniforms)
{
    m_uniforms = graph_uniforms;
}
