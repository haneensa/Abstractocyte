#include "graphmanager.h"

GraphManager::GraphManager()
    : m_IndexVBO( QOpenGLBuffer::IndexBuffer ),
      m_NodesVBO( QOpenGLBuffer::VertexBuffer ),
      m_glFunctionsSet(false),
      m_FDL_running(false),
      m_2D(false)
{
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

void GraphManager::update2Dflag(bool is2D)
{
    m_2D = is2D;
    if (!m_2D) {
        // reset the cooridnates of the graphs
        QMatrix4x4 identitiy;
        m_graph[0]->resetCoordinates(identitiy); // for now later I will have two separate variables one for 2D one for 3D

    } else {
        // reset graph
        m_graph[0]->resetCoordinates(m_uniforms.rMatrix);

    }
    updateUniformsLocation(m_program_nodes);
    updateUniformsLocation(m_program_Index);
}
// I have 4 graphs:
// so here extract the object nodes in a list because more than a graph use them
// extract skeletons graph in another list
// extract connectivity between neurites in a list
// extract connectivity between astrocyte and neurties in another list
// 1) all skeleton with astrocyte
    // (nodes are object skeleton nodes, and edges are the edges that connect them)
    // + connection points between astrocyte and neurites if that vertex is close to the astrocyte
// 2) skeletons without astrocyte
    // only nods of object and what connect the object nodes together
// 3) astrocyte skeleton and object nodes
    // object nodes
    // astrocyte graph
    // connectivity edges between them
// 4) object nodes and their connectivity information
    // object nodes
    // connectivity info from them
void GraphManager::ExtractGraphFromMesh(ObjectManager *objectManager)
{

     // render skeletons then interpolate to nodes by accessing the nodes positions from ssbo
     // use object manager to initialize them

     // init the edges and they never change except when to display them

     m_graph[0] = new Graph( Graph_t::NODE_NODE ); // neurite-neurite
     m_graph[1] = new Graph( Graph_t::NODE_SKELETON ); // neurite-astrocyte skeleton
     m_graph[2] = new Graph( Graph_t::ALL_SKELETONS ); //  neurites skeletons - astrocyte skeleton
     m_graph[3] = new Graph( Graph_t::NEURITE_SKELETONS ); // neuries skeletons

     m_graph[0]->createGraph(objectManager);
     m_graph[1]->createGraph(objectManager);
     m_graph[2]->createGraph(objectManager);
     m_graph[3]->createGraph(objectManager);
}

void GraphManager::stopForceDirectedLayout(int graphIdx)
{
    // problem the rotation is still on the nodes

    m_graph[graphIdx]->terminateFDL();
    m_FDL_running = false; // todo: get this from the graph itself, since it is per graph, even the thread?

    if (m_layout_thread1.joinable()) {
        m_layout_thread1.join();
    }

}

void GraphManager::startForceDirectedLayout(int graphIdx)
{
    // update the 2D node position at the start and whenever the m_mvp change, -> when m_value == 1.0 and m_mvp changed
    stopForceDirectedLayout(graphIdx);



    m_FDL_running = true;
    m_layout_thread1 = std::thread(&Graph::runforceDirectedLayout, m_graph[graphIdx]);

}

bool GraphManager::initOpenGLFunctions()
{
    m_glFunctionsSet = true;
    initializeOpenGLFunctions();

    return true;
}

void GraphManager::initVertexAttribPointer()
{
    int offset = 0;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(struct BufferNode),  0);
    offset += sizeof(QVector3D);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          sizeof(struct BufferNode),  (void*)offset);
    offset += sizeof(QVector2D);
    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(2, 1, GL_INT, sizeof(BufferNode), (void*)offset);
}

// we initialize the vbos for drawing
bool GraphManager::initVBO(int graphIdx)
{
    if (max_graphs < graphIdx) {
        qDebug() << "graph index out of range";
        return false;
    }

    if (m_glFunctionsSet == false)
        return false;

    qDebug() << "graph->initVBO";

    // 1) initialize shaders
    m_program_nodes = glCreateProgram();
    bool res = initShader(m_program_nodes,  ":/shaders/nodes_vert.glsl", ":/shaders/nodes_geom.glsl", ":/shaders/nodes_frag.glsl");
    if (res == false)
        return res;

    m_program_Index = glCreateProgram();
    res = initShader(m_program_Index,  ":/shaders/nodes_vert.glsl", ":/shaders/lines_geom.glsl", ":/shaders/lines_frag.glsl");
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

    initVertexAttribPointer();

    GL_Error();

    // initialize uniforms
    updateUniformsLocation(m_program_nodes);

    m_NodesVBO.release();
    m_NodesVAO.release();


    m_IndexVAO.create();
    m_IndexVAO.bind();
    m_NodesVBO.bind();

    initVertexAttribPointer();

    m_NodesVBO.release();


    m_IndexVBO.create();
    m_IndexVBO.bind();
    m_graph[graphIdx]->allocateBIndices(m_IndexVBO);

    glUseProgram(m_program_Index);
    updateUniformsLocation(m_program_Index);


    m_IndexVBO.release();
    m_IndexVAO.release();

    return true;
}

void GraphManager::initGrid()
{
    m_graph[0]->initGridBuffers();
}

void GraphManager::drawGrid(struct GlobalUniforms grid_uniforms)
{
    m_graph[0]->drawGrid(grid_uniforms);
}


void GraphManager::drawNodes(int graphIdx)
{

    if (max_graphs < graphIdx) {
        qDebug() << "graph index out of range";
        return;
    }

    if (m_glFunctionsSet == false)
        return;

    m_NodesVAO.bind();
    m_NodesVBO.bind();
    m_graph[graphIdx]->allocateBVertices(m_NodesVBO);

    glUseProgram(m_program_nodes);
    updateUniformsLocation(m_program_nodes);

    glDrawArrays(GL_POINTS, 0, m_graph[graphIdx]->vertexBufferSize() );
    m_NodesVBO.release();
    m_NodesVAO.release();
}

void GraphManager::drawEdges(int graphIdx)
{
    if (max_graphs < graphIdx) {
        qDebug() << "graph index out of range";
        return;
    }

    if (m_glFunctionsSet == false)
        return;

    m_IndexVAO.bind();
    m_NodesVBO.bind();
    m_IndexVBO.bind();

    glUseProgram(m_program_Index);
    updateUniformsLocation(m_program_Index);

    glLineWidth(10.0f);
    glDrawElements(GL_LINES, m_graph[graphIdx]->indexBufferSize(), GL_UNSIGNED_INT, 0 );
    m_NodesVBO.release();
    m_IndexVBO.release();
    m_IndexVAO.release();
}

void GraphManager::updateUniformsLocation(GLuint program)
{
    if (m_glFunctionsSet == false)
        return;

    // initialize uniforms
    GLuint mMatrix = glGetUniformLocation(program, "mMatrix");
    GLuint vMatrix = glGetUniformLocation(program, "vMatrix");

    if (m_2D) { // force directed layout started, them use model without rotation
        glUniformMatrix4fv(mMatrix, 1, GL_FALSE, m_uniforms.modelNoRotMatrix);
     } else {
        glUniformMatrix4fv(mMatrix, 1, GL_FALSE, m_uniforms.mMatrix);
    }

    glUniformMatrix4fv(vMatrix, 1, GL_FALSE, m_uniforms.vMatrix);

    GLuint pMatrix = glGetUniformLocation(program, "pMatrix");
    glUniformMatrix4fv(pMatrix, 1, GL_FALSE, m_uniforms.pMatrix);

    GLint y_axis = glGetUniformLocation(program, "y_axis");
    glUniform1iv(y_axis, 1, &m_uniforms.y_axis);

    GLint x_axis = glGetUniformLocation(program, "x_axis");
    glUniform1iv(x_axis, 1, &m_uniforms.x_axis);

}

void GraphManager::updateUniforms(struct GlobalUniforms graph_uniforms)
{
    m_uniforms = graph_uniforms;
}

void GraphManager::updateGraphParam1(double value)
{
    m_graph[0]->updateGraphParam1(value);
}

void GraphManager::updateGraphParam2(double value)
{
    m_graph[0]->updateGraphParam2(value);
}

void GraphManager::updateGraphParam3(double value)
{
    m_graph[0]->updateGraphParam3(value);
}

void GraphManager::updateGraphParam4(double value)
{
    m_graph[0]->updateGraphParam4(value);
}

void GraphManager::updateGraphParam5(double value)
{
    m_graph[0]->updateGraphParam5(value);
}

void GraphManager::updateGraphParam6(double value)
{
    m_graph[0]->updateGraphParam6(value);
}

void GraphManager::updateGraphParam7(double value)
{
    m_graph[0]->updateGraphParam7(value);
}
