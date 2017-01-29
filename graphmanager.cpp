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
        m_graph[m_ngraph]->loadNodes("://data/skeleton_astrocyte_m3/skeleton_astro_nodes.csv");
        m_graph[m_ngraph]->loadEdges("://data/skeleton_astrocyte_m3/skeleton_astro_segments.csv");
        m_ngraph++;
    }

}

GraphManager::~GraphManager()
{
    qDebug() << "~GraphManager()";
    if (m_layout_thread1.joinable()) {
        m_layout_thread1.join();
    }

    // destroy all vbo and vao and programs and graphs
}

void GraphManager::startForceDirectedLayout(int graphIdx)
{
    // update the 2D node position at the start and whenever the m_mvp change, -> when m_value == 1.0 and m_mvp changed
    if (m_layout_thread1.joinable()) {
        m_layout_thread1.join();
    }
    // reset graph
    m_graph[graphIdx]->resetCoordinates(m_uniforms.rMatrix);
    m_layout_thread1 = std::thread(&Graph::runforceDirectedLayout, m_graph[graphIdx]);
  //  m_FDL_running = true;

}

bool GraphManager::initOpenGLFunctions()
{
    m_glFunctionsSet = true;
    initializeOpenGLFunctions();

    return true;
}


// we initialize the vbos for drawing
bool GraphManager::initVBO(struct GraphUniforms graph_uniforms, int graphIdx)
{
    if (m_ngraph < graphIdx) {
        qDebug() << "graph index out of range";
        return false;
    }

    if (m_glFunctionsSet == false)
        return false;

    qDebug() << "graph->initVBO";
    m_uniforms = graph_uniforms;

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

    updateUniforms();

    // initialize uniforms

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
    updateUniforms();


    m_IndexVBO.release();
    m_IndexVAO.release();

    return true;
}

void GraphManager::drawNodes(struct GraphUniforms graph_uniforms, int graphIdx)
{
    if (m_ngraph < graphIdx) {
        qDebug() << "graph index out of range";
        return;
    }


    if (m_glFunctionsSet == false)
        return;

    m_uniforms = graph_uniforms;

    m_NodesVAO.bind();
    m_NodesVBO.bind();
    m_graph[graphIdx]->allocateBVertices(m_NodesVBO);

    glUseProgram(m_program_nodes);
    updateUniforms();

    glDrawArrays(GL_POINTS, 0, m_graph[graphIdx]->vertexBufferSize() );
    m_NodesVBO.release();
    m_NodesVAO.release();
}

void GraphManager::drawEdges(struct GraphUniforms graph_uniforms, int graphIdx)
{
    if (m_ngraph < graphIdx) {
        qDebug() << "graph index out of range";
        return;
    }


    if (m_glFunctionsSet == false)
        return;

    m_uniforms = graph_uniforms;
    m_IndexVAO.bind();
    m_NodesVBO.bind();
    m_IndexVBO.bind();

    glUseProgram(m_program_Index);
    updateUniforms();

    glDrawElements(GL_LINES, m_graph[graphIdx]->indexBufferSize(), GL_UNSIGNED_SHORT, 0 );
    m_NodesVBO.release();
    m_IndexVBO.release();
    m_IndexVAO.release();
}

void GraphManager::updateUniforms()
{

    if (m_glFunctionsSet == false)
        return;

    // initialize uniforms
    GLuint mMatrix = glGetUniformLocation(m_program_nodes, "mMatrix");
    if (m_FDL_running) // force directed layout started, them use model without rotation
        glUniformMatrix4fv(mMatrix, 1, GL_FALSE, m_uniforms.modelNoRotMatrix);
    else
        glUniformMatrix4fv(mMatrix, 1, GL_FALSE, m_uniforms.mMatrix);

    GLuint vMatrix = glGetUniformLocation(m_program_nodes, "vMatrix");
    glUniformMatrix4fv(vMatrix, 1, GL_FALSE, m_uniforms.vMatrix);

    GLuint pMatrix = glGetUniformLocation(m_program_nodes, "pMatrix");
    glUniformMatrix4fv(pMatrix, 1, GL_FALSE, m_uniforms.pMatrix);


}
