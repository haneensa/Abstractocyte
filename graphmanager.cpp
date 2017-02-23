// this should only handle the 4 graphs, their construction, layouting algorithms
// and updating their data, and drawing
#include "graphmanager.h"

GraphManager::GraphManager(ObjectManager *objectManager)
    : m_NeuritesIndexVBO( QOpenGLBuffer::IndexBuffer ),
      m_NeuritesNodesVBO( QOpenGLBuffer::VertexBuffer ),
      m_SkeletonsIndexVBO( QOpenGLBuffer::IndexBuffer ),
      m_SkeletonsNodesVBO( QOpenGLBuffer::VertexBuffer ),
      m_glFunctionsSet(false),
      m_FDL_running(false),
      m_2D(false)
{
    m_obj_mngr = objectManager;
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
        m_NeuritesGraphVAO.destroy();
        m_NeuritesNodesVBO.destroy();
        m_NeuritesIndexVBO.destroy();
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
    updateUniformsLocation(m_program_neurites_nodes);
    updateUniformsLocation(m_program_neurites_index);
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
void GraphManager::ExtractGraphFromMesh()
{

     // render skeletons then interpolate to nodes by accessing the nodes positions from ssbo
     // use object manager to initialize them

     // init the edges and they never change except when to display them

     m_graph[0] = new Graph( Graph_t::NODE_SKELETON ); // neurite-neurite
//     m_graph[1] = new Graph( Graph_t::NODE_SKELETON ); // neurite-astrocyte skeleton
//     m_graph[2] = new Graph( Graph_t::ALL_SKELETONS ); //  neurites skeletons - astrocyte skeleton
//     m_graph[3] = new Graph( Graph_t::NEURITE_SKELETONS ); // neuries skeletons

     m_graph[0]->createGraph(m_obj_mngr);
//     m_graph[1]->createGraph(m_obj_mngr);
//     m_graph[2]->createGraph(m_obj_mngr);
//     m_graph[3]->createGraph(m_obj_mngr);
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

void GraphManager::initNeuritesVertexAttribPointer()
{
    int offset = 0;
    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, 1, GL_INT, 0, (void*)offset);

}
void GraphManager::initSkeletonsVertexAttribPointer()
{
   // QVector4D vertex;   // original position  (simplified)  -> w: hvgx ID
   // QVector2D layout1;  // layouted position (all skeletons)
   // QVector2D layout2;  // layouted position (no neurites)
   // QVector2D layout3;  // layouted position (no astrocyte)
    int offset = 0;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,
                          sizeof(struct AbstractSkelNode),  0);


    offset +=  sizeof(QVector4D);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          sizeof(AbstractSkelNode), (GLvoid*)offset);

    offset +=  sizeof(QVector2D);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                          sizeof(AbstractSkelNode), (GLvoid*)offset);

    offset +=  sizeof(QVector2D);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE,
                          sizeof(AbstractSkelNode), (GLvoid*)offset);

}

// we initialize the vbos for drawing
bool GraphManager::initVBO()
{
    if (m_glFunctionsSet == false)
        return false;

    qDebug() << "graph->initVBO";

    /* neurites nodes */
    // 1) initialize shaders
    m_program_neurites_nodes = glCreateProgram();
    bool res = initShader(m_program_neurites_nodes, ":/shaders/nodes_vert.glsl",
                                                    ":/shaders/nodes_geom.glsl",
                                                    ":/shaders/nodes_frag.glsl");

    if (res == false)
        return res;

    m_program_neurites_index = glCreateProgram();
    res = initShader(m_program_neurites_index,  ":/shaders/nodes_vert.glsl",
                                       ":/shaders/lines_geom.glsl",
                                       ":/shaders/lines_frag.glsl");
    if (res == false)
        return res;

    GL_Error();

    // initialize buffers
    m_NeuritesGraphVAO.create();
    m_NeuritesGraphVAO.bind();

    m_NeuritesNodesVBO.create();
    m_NeuritesNodesVBO.setUsagePattern( QOpenGLBuffer::DynamicDraw );
    m_NeuritesNodesVBO.bind();

    m_obj_mngr->allocate_neurites_nodes(m_NeuritesNodesVBO);

    glUseProgram(m_program_neurites_nodes);
    GL_Error();

    initNeuritesVertexAttribPointer();

    GL_Error();

    // initialize uniforms
    updateUniformsLocation(m_program_neurites_nodes);

    m_NeuritesNodesVBO.release();

    m_NeuritesIndexVBO.create();
    m_NeuritesIndexVBO.bind();

    m_obj_mngr->allocate_neurites_edges(m_NeuritesNodesVBO);

    glUseProgram(m_program_neurites_index);
    updateUniformsLocation(m_program_neurites_index);


    m_NeuritesIndexVBO.release();
    m_NeuritesGraphVAO.release();



    /* skeletons */

    m_program_skeletons_index = glCreateProgram();
    res = initShader(m_program_skeletons_index, ":/shaders/abstract_skeleton_node_vert.glsl",
                                                ":/shaders/nodes_geom.glsl",
                                                ":/shaders/nodes_frag.glsl");
    if (res == false)
        return res;

    m_program_neurites_index = glCreateProgram();
    res = initShader(m_program_neurites_index,  ":/shaders/abstract_skeleton_node_vert.glsl",
                                       ":/shaders/lines_geom.glsl",
                                       ":/shaders/lines_frag.glsl");
    if (res == false)
        return res;

    m_SkeletonsGraphVAO.create();
    m_SkeletonsGraphVAO.bind();

    m_SkeletonsNodesVBO.create();
    m_SkeletonsNodesVBO.setUsagePattern( QOpenGLBuffer::DynamicDraw );
    m_SkeletonsNodesVBO.bind();

    m_obj_mngr->allocate_abs_skel_nodes(m_SkeletonsNodesVBO);

    glUseProgram(m_program_neurites_nodes);
    GL_Error();

    initSkeletonsVertexAttribPointer();

    GL_Error();

    // initialize uniforms
    updateUniformsLocation(m_program_neurites_nodes);

    m_SkeletonsNodesVBO.release();

    m_SkeletonsIndexVBO.create();
    m_SkeletonsIndexVBO.bind();

    m_obj_mngr->allocate_abs_skel_edges(m_SkeletonsIndexVBO);

    glUseProgram(m_program_neurites_index);
    updateUniformsLocation(m_program_neurites_index);


    m_SkeletonsIndexVBO.release();
    m_SkeletonsGraphVAO.release();


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


void GraphManager::drawNeuritesGraph()
{
    if (m_glFunctionsSet == false)
        return;

    m_obj_mngr->write_ssbo_data();

    m_NeuritesGraphVAO.bind();
    m_NeuritesNodesVBO.bind();

    glUseProgram(m_program_neurites_nodes);
    updateUniformsLocation(m_program_neurites_nodes);

    glDrawArrays(GL_POINTS, 0, m_obj_mngr->get_neurites_nodes_size() );

    m_NeuritesIndexVBO.bind();

    glUseProgram(m_program_neurites_index);
    updateUniformsLocation(m_program_neurites_index);

    glLineWidth(10.0f);
    glDrawElements(GL_LINES,  m_obj_mngr->get_neurites_edges_size(), GL_UNSIGNED_INT, 0 );

    m_NeuritesIndexVBO.release();

    m_NeuritesNodesVBO.release();
    m_NeuritesGraphVAO.release();
}

void GraphManager::drawSkeletonsGraph()
{
    if (m_glFunctionsSet == false)
        return;

    // update skeleton data from object manager

    m_SkeletonsGraphVAO.bind();
    m_SkeletonsNodesVBO.bind();

    glUseProgram(m_program_neurites_nodes);
    updateUniformsLocation(m_program_neurites_nodes);

    glDrawArrays(GL_POINTS, 0,  m_obj_mngr->get_abs_skel_nodes_size() );

    m_SkeletonsIndexVBO.bind();

    glUseProgram(m_program_neurites_index);
    updateUniformsLocation(m_program_neurites_index);

    glLineWidth(10.0f);
    glDrawElements(GL_LINES, m_obj_mngr->get_abs_skel_edges_size(), GL_UNSIGNED_INT, 0 );
    m_SkeletonsIndexVBO.release();


    m_SkeletonsNodesVBO.release();
    m_SkeletonsGraphVAO.release();
}

void GraphManager::updateUniformsLocation(GLuint program)
{
    if (m_glFunctionsSet == false)
        return;

    // initialize uniforms
    GLuint mMatrix = glGetUniformLocation(program, "mMatrix");
    GLuint vMatrix = glGetUniformLocation(program, "vMatrix");
    GLint is2D = glGetUniformLocation(program, "is2D");
    int is2D_value;

    if (m_2D) { // force directed layout started, them use model without rotation
        glUniformMatrix4fv(mMatrix, 1, GL_FALSE, m_uniforms.modelNoRotMatrix);
        is2D_value = 1;
     } else {
        glUniformMatrix4fv(mMatrix, 1, GL_FALSE, m_uniforms.mMatrix);
        is2D_value = 0;
    }
    glUniform1iv(is2D, 1, &is2D_value);

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
