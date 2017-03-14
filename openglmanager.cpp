/*
Just to make it easier.. this code is copied from HVR_AbstractGraphManager


//Render the selection frame

glBindFramebufferEXT(GL_FRAMEBUFFER, m_selectionFramebuffer);
//clear
glClear(GL_COLOR_BUFFER_BIT);
//disable dithering -- important
glDisable(GL_DITHER);
//render graph
m_abstract_graph->drawAbstract(HVR_GRAPH_DRAW_SELECTION_MODE, HVR_ABSTRACT_GRAPH_DRAW_CLUSTERED_EDGES_OPTION);
//enable dithering again
glEnable(GL_DITHER);
//process click
_processSelection();

glBindFramebufferEXT(GL_FRAMEBUFFER, 0);


in process selection:

GLint viewport[4];
GLubyte pixel[3];
glGetIntegerv(GL_VIEWPORT, viewport);
glReadPixels(m_mouse_x, viewport[3] - m_mouse_y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, (void *)pixel);

int id = pixel[0] + pixel[1] * 256 + pixel[2] * 65536;

 */

#include "openglmanager.h"

OpenGLManager::OpenGLManager(DataContainer *obj_mnger, AbstractionSpace  *absSpace)
    : m_2D(false),
      m_bindIdx(2) // ssbo biding point
{
    m_dataContainer = obj_mnger;
    m_2dspace = absSpace;
    m_glFunctionsSet = false;

    m_display_child = false;
    m_display_parent = false;
    m_display_synapses = false;
	m_zoom = 1.0f;
    m_depth = 1;

    m_color_encoding = Color_e::TYPE;
    m_size_encoding = Size_e::VOLUME;

}

OpenGLManager::~OpenGLManager()
{
    if (m_glFunctionsSet == false)
        return;

    m_vao_glycogen.destroy();
    m_vbo_glycogen.destroy();
}

bool OpenGLManager::initOpenGLFunctions()
{
    m_glFunctionsSet = true;
    initializeOpenGLFunctions();

    m_GSkeleton.initOpenGLFunctions();
    m_TMesh.initOpenGLFunctions();
    m_SkeletonPoints.initOpenGLFunctions();
    m_GNeurites.initOpenGLFunctions();
    m_GlycogenPoints.initOpenGLFunctions();

    fillVBOsData();

    initSSBO();

    // *********** 3) Skeleton Points    ***********
    initSkeletonShaders();
    initMeshTrianglesShaders();
    initAbstractSkeletonShaders();
    initNeuritesGraphShaders();
    initGlycogenPointsShaders();
    return true;
}

void OpenGLManager::updateCanvasDim(int w, int h, int retianScale)
{
    if (m_canvas_h != h || m_canvas_w != w){
        m_canvas_h = h * retianScale;
        m_canvas_w = w * retianScale;
        m_retinaScale = retianScale;
        initSelectionFrameBuffer();
        GL_Error();
    }

}

// ----------------------------------------------------------------------------
//
void OpenGLManager::initSelectionFrameBuffer()
{
    qDebug() << "initSelectionFrameBuffer";

    // create FBO
    glGenFramebuffers(1, &m_selectionFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_selectionFrameBuffer);
    glGenRenderbuffers(1, &m_selectionRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_selectionRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, m_canvas_w, m_canvas_h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_selectionRenderBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// ----------------------------------------------------------------------------
//
int OpenGLManager::processSelection(float x, float y)
{
    qDebug() << "Draw Selection!";

    glBindFramebuffer(GL_FRAMEBUFFER, m_selectionFrameBuffer);
    glEnable(GL_DEPTH_TEST);

    GLubyte pixel[3];

    qDebug() << x << " " << y;
    //glReadBuffer(GL_BACK);
    glReadPixels(x, y, 1, 1, GL_RGB,GL_UNSIGNED_BYTE, (void *)pixel);
    int pickedID = pixel[0] + pixel[1] * 256 + pixel[2] * 65536;
    qDebug() << pixel[0] << " " << pixel[1] << " " << pixel[2];

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    qDebug() << "Picked ID: " << pickedID;

    return pickedID;
 }

// ----------------------------------------------------------------------------
//
bool OpenGLManager::initSSBO()
{
    if (m_glFunctionsSet == false)
        return false;

    qDebug() <<  " m_ssbo_data.size() : " << m_ssbo_data.size() ;
    int bufferSize =  m_ssbo_data.size() * sizeof(struct ssbo_mesh);

    glGenBuffers(1, &m_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize , NULL, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bindIdx, m_ssbo);
    qDebug() << "m_ssbo_data buffer size: " << bufferSize;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    write_ssbo_data();

    return true;
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::write_ssbo_data()
{
    int bufferSize =  m_ssbo_data.size() * sizeof(struct ssbo_mesh);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
    GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
    memcpy(p,   m_ssbo_data.data(),  bufferSize);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}


// ----------------------------------------------------------------------------
//
/*
 * allocates: m_vbo_skeleton with skeleton points
 * allocates: m_vbo_IndexMesh with mesh indices
 * allocates: m_ssbo_data with object data
 * allocates: m_neurites_nodes with neurites IDs
 * allocates: m_neurites_edges with connectivity info between m_neurites_nodes using their index in the list
 * allocates: m_abstract_skel_nodes with abstract skeleton nodes
 * allocates: m_abstract_skel_edges of abstract skeleton
 * */
void OpenGLManager::fillVBOsData()
{
    m_ssbo_data.resize(1000);

    m_SkeletonPoints.vboCreate("SkeletonPoints", Buffer_t::VERTEX, Buffer_USAGE_t::STATIC);
    m_SkeletonPoints.vboBind("SkeletonPoints");
    m_SkeletonPoints.vboAllocate("SkeletonPoints", NULL,
                            m_dataContainer->getSkeletonPointsSize()  * sizeof(SkeletonPoint));

    int neurtite_vbo_IndexMesh_offset = 0;

    // initialize index buffers
    m_TMesh.vboCreate("MeshIndices", Buffer_t::INDEX, Buffer_USAGE_t::STATIC);
    m_TMesh.vboBind("MeshIndices");
    m_TMesh.vboAllocate("MeshIndices", NULL, m_dataContainer->getMeshIndicesSize() * sizeof(GLuint) );


    int vbo_skeleton_offset = 0;

    std::map<int, Object*> objects_map = m_dataContainer->getObjectsMap();
    for ( auto iter = objects_map.begin(); iter != objects_map.end(); iter++) {
        Object *object_p = (*iter).second;
        int ID = object_p->getHVGXID();

        /* fill ssbo data per object */
        if (m_ssbo_data.size() <= ID) {
            qDebug() << "resizing m_ssbo_data.";
            m_ssbo_data.resize(ID + 100);
        }

        m_ssbo_data[ID] = object_p->getSSBOData();
        float volume =  object_p->getVolume() / m_dataContainer->getMaxVolume();
        m_ssbo_data[ID].info.setX( 20 *  volume);

        qDebug() << " allocating: " << object_p->getName().data();


        // allocating mesh indices
        int vbo_IndexMesh_count = object_p->get_indices_Size() * sizeof(GLuint);
        // write only neurites, if astrocyte then write in m_Astro_vbo_IndexMesh
        m_TMesh.vboWrite("MeshIndices", neurtite_vbo_IndexMesh_offset, object_p->get_indices(), vbo_IndexMesh_count);

        neurtite_vbo_IndexMesh_offset += vbo_IndexMesh_count;


        // allocating skeleton vertices, if this object hash no skeleton, then this will return and wont write anything
        int vbo_skeleton_count = object_p->writeSkeletontoVBO(m_SkeletonPoints.getVBO("SkeletonPoints"), vbo_skeleton_offset);
        vbo_skeleton_offset += vbo_skeleton_count;

        // allocate neurites nodes place holders
        if (object_p->getObjectType() != Object_t::ASTROCYTE
                && object_p->getObjectType() != Object_t::MITO
                && object_p->getObjectType()  != Object_t::SYNAPSE) {
            object_p->setNodeIdx(m_neurites_nodes.size());
            m_neurites_nodes.push_back(ID);
        }

        // get skeleton of the object
        // if no skeleton, this will be skipped, thus no graph for this object
        Skeleton *skeleton = object_p->getSkeleton();
        // if (spine/bouton) then get their skeleton from their parents
        // get the actual nodes from their parents
        // only show them if parent is filtered

        // I do want the substructures graph to be present and use them only if their parents was filtered

        // add all and in the view check if the parent not filtered then dont show it
        std::vector<QVector3D> nodes3D = skeleton->getGraphNodes(); // if child these would be 0
        std::vector<QVector2D> edges2D = skeleton->getGraphEdges();
        qDebug() << "set offset: " << m_abstract_skel_nodes.size();
        object_p->setSkeletonOffset(m_abstract_skel_nodes.size());
        // each skeleton node has local index within its list of nodes
        // and an offset within list of abstract skel nodes
        for ( int i = 0; i < nodes3D.size(); i++) {
            QVector4D vertex = nodes3D[i];
            vertex.setW(ID);
            struct AbstractSkelNode skel_node = {vertex, vertex.toVector2D(), vertex.toVector2D(), vertex.toVector2D() };
            m_abstract_skel_nodes.push_back(skel_node);
        }

        int skeleton_offset = object_p->getSkeletonOffset();
        // add edges
        for (int i = 0; i < edges2D.size(); ++i) {
            int nID1 = edges2D[i].x() + skeleton_offset;
            int nID2 = edges2D[i].y() + skeleton_offset;
            m_abstract_skel_edges.push_back(nID1);
            m_abstract_skel_edges.push_back(nID2);
        }
   }

    // allocate neurites nodes edges
    std::vector<QVector2D> edges_info = m_dataContainer->getNeuritesEdges();
    for (int i = 0; i < edges_info.size(); ++i) {
        int nID1 = edges_info[i].x();
        int nID2 = edges_info[i].y();
        if (objects_map.find(nID1) == objects_map.end()
            || objects_map.find(nID2) == objects_map.end()) {
            continue;
        }
        m_neurites_edges.push_back(objects_map[nID1]->getNodeIdx());
        m_neurites_edges.push_back(objects_map[nID2]->getNodeIdx());
    }

    m_TMesh.vboRelease("MeshIndices");
    m_SkeletonPoints.vboRelease("SkeletonPoints");

    // allocate neurites nodes


    qDebug() << "m_abstract_skel_nodes.size(): " << m_abstract_skel_nodes.size();
    qDebug() << "m_abstract_skel_edges.size(): " << m_abstract_skel_edges.size();
    qDebug() << "m_neurites_edges.size(): " << m_neurites_edges.size();
    qDebug() << "m_neurites_nodes.size(): " << m_neurites_nodes.size();


}

// ----------------------------------------------------------------------------
//
bool OpenGLManager::initMeshVertexAttrib()
{
    if (m_glFunctionsSet == false)
        return false;

    int offset = 0;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,
                          sizeof(VertexData),  0);


    offset +=  sizeof(QVector4D);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE,
                          sizeof(VertexData), (GLvoid*)offset);

    return true;
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::initNeuritesVertexAttribPointer()
{
    int offset = 0;
    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, 1, GL_INT, 0, (void*)offset);

}

void OpenGLManager::initSkeletonsVertexAttribPointer()
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

// ----------------------------------------------------------------------------
//
bool OpenGLManager::initAbstractSkeletonShaders()
{
    qDebug() << "OpenGLManager::initAbstractSkeletonShaders()";

    if (m_glFunctionsSet == false)
        return false;

    qDebug() << "nodes shader";
    m_GSkeleton.createProgram("2D_nodes");
    bool res = m_GSkeleton.compileShader("2D_nodes",
                                      ":/shaders/abstract_skeleton_node_2D_vert.glsl",
                                      ":/shaders/abstract_skeleton_node_geom.glsl",
                                      ":/shaders/points_3d_frag.glsl");
    if (res == false)
        return res;

    qDebug() << "index shader";
    m_GSkeleton.createProgram("2D_index");
    res = m_GSkeleton.compileShader("2D_index",
                                     ":/shaders/abstract_skeleton_node_2D_vert.glsl",
                                     ":/shaders/abstract_skeleton_line_geom.glsl",
                                     ":/shaders/points_3d_frag.glsl");

    if (res == false)
        return res;

    m_GSkeleton.createProgram("23D_nodes");
    res = m_GSkeleton.compileShader("23D_nodes",
                                    ":/shaders/abstract_skeleton_node_transition_vert.glsl",
                                    ":/shaders/abstract_skeleton_node_geom.glsl",
                                    ":/shaders/points_3d_frag.glsl");

    if (res == false)
        return res;

    qDebug() << "index shader";
    m_GSkeleton.createProgram("23D_index");
    res = m_GSkeleton.compileShader("23D_index",
                                    ":/shaders/abstract_skeleton_node_transition_vert.glsl",
                                    ":/shaders/abstract_skeleton_line_geom.glsl",
                                    ":/shaders/points_3d_frag.glsl");

    if (res == false)
        return res;

    // allocate skeleton nodes
    m_GSkeleton.vboCreate("nodes", Buffer_t::VERTEX, Buffer_USAGE_t::DYNAMIC_DRAW);
    m_GSkeleton.vboBind("nodes");

    m_GSkeleton.vboAllocate("nodes",
                            m_abstract_skel_nodes.data(),
                            m_abstract_skel_nodes.size() * sizeof(struct AbstractSkelNode) );

    GL_Error();
    m_GSkeleton.vboRelease("nodes");

    // allocate skeleton edges
    m_GSkeleton.vboCreate("index", Buffer_t::INDEX, Buffer_USAGE_t::STATIC);
    m_GSkeleton.vboBind("index");

    m_GSkeleton.vboAllocate("index",
                            m_abstract_skel_edges.data(),
                            m_abstract_skel_edges.size() * sizeof(GLuint) );

    m_GSkeleton.vboRelease("index");
    GL_Error();


    m_GSkeleton.vaoCreate("SkeletonGraph");
    m_GSkeleton.vaoBind("SkeletonGraph");

    m_GSkeleton.vboBind("nodes");


    m_GSkeleton.useProgram("2D_nodes");
    initSkeletonsVertexAttribPointer();

    m_GSkeleton.useProgram("23D_nodes");
    initSkeletonsVertexAttribPointer();

    GL_Error();


    m_GSkeleton.vboRelease("nodes");

    m_GSkeleton.vboBind("index");
    m_GSkeleton.useProgram("2D_index");
    m_GSkeleton.useProgram("23D_index");

    m_GSkeleton.vboRelease("index");
    m_GSkeleton.vaoRelease();



    /* selectin */
    m_GSkeleton.createProgram("selection_2D_nodes");
    res = m_GSkeleton.compileShader("selection_2D_nodes",
                                    ":/shaders/abstract_skeleton_node_2D_vert.glsl",
                                    ":/shaders/abstract_skeleton_node_geom.glsl",
                                    ":/shaders/hvgx_selection_frag.glsl");
    if (res == false)
        return res;


    m_GSkeleton.createProgram("selection_2D_index");
    res = m_GSkeleton.compileShader("selection_2D_index",
                                    ":/shaders/abstract_skeleton_node_2D_vert.glsl",
                                    ":/shaders/abstract_skeleton_line_geom.glsl",
                                    ":/shaders/hvgx_selection_frag.glsl");

    if (res == false)
        return res;

    m_GSkeleton.createProgram("selection_23D_nodes");
    res = m_GSkeleton.compileShader("selection_23D_nodes",
                                    ":/shaders/abstract_skeleton_node_transition_vert.glsl",
                                    ":/shaders/abstract_skeleton_node_geom.glsl",
                                    ":/shaders/hvgx_selection_frag.glsl");

    if (res == false)
        return res;

    m_GSkeleton.createProgram("selection_23D_index");
    res = m_GSkeleton.compileShader("selection_23D_index",
                                    ":/shaders/abstract_skeleton_node_transition_vert.glsl",
                                    ":/shaders/abstract_skeleton_line_geom.glsl",
                                    ":/shaders/hvgx_selection_frag.glsl");

    if (res == false)
        return res;

    m_GSkeleton.vaoCreate("Selection");
    m_GSkeleton.vaoBind("Selection");

    m_GSkeleton.vboBind("nodes");

    m_GSkeleton.useProgram("selection_2D_nodes");
    initSkeletonsVertexAttribPointer();

    m_GSkeleton.useProgram("selection_23D_nodes");
    initSkeletonsVertexAttribPointer();

    GL_Error();

    m_GSkeleton.vboRelease("nodes");

    m_GSkeleton.vboBind("index");

    m_GSkeleton.useProgram("selection_2D_index");
    m_GSkeleton.useProgram("selection_23D_index");

    m_GSkeleton.vboRelease("index");

    m_GSkeleton.vaoRelease();


}

// ----------------------------------------------------------------------------
//
// only the edges, the nodes itself they are not needed to be visible
// this will collabse into a node for the neurites at the most abstract view
void OpenGLManager::drawSkeletonsGraph(struct GlobalUniforms grid_uniforms, bool selection )
{
    if (m_glFunctionsSet == false)
        return;

    // update skeleton data from object manager
    m_uniforms = grid_uniforms;

    m_GSkeleton.vboBind("nodes");
    m_GSkeleton.vboAllocate("nodes",
                            m_abstract_skel_nodes.data(),
                            m_abstract_skel_nodes.size() * sizeof(struct AbstractSkelNode) );
    m_GSkeleton.vboRelease("nodes");

    if (selection) {
        m_GSkeleton.vaoBind("Selection");
        m_GSkeleton.useProgram("selection_2D_nodes");
        updateAbstractUniformsLocation(m_GSkeleton.getProgram("selection_2D_nodes"));
        glDrawArrays(GL_POINTS, 0,  m_abstract_skel_nodes.size() );

        m_GSkeleton.useProgram("selection_23D_nodes");
        updateAbstractUniformsLocation(m_GSkeleton.getProgram("selection_23D_nodes"));
        glDrawArrays(GL_POINTS, 0,  m_abstract_skel_nodes.size() );

        m_GSkeleton.vboBind("index");

        m_GSkeleton.useProgram("selection_2D_index");
        updateAbstractUniformsLocation(m_GSkeleton.getProgram("selection_2D_index"));
        glLineWidth(20);
        glDrawElements(GL_LINES, m_abstract_skel_edges.size(), GL_UNSIGNED_INT, 0 );

        m_GSkeleton.useProgram("selection_23D_index");
        updateAbstractUniformsLocation(m_GSkeleton.getProgram("selection_23D_index"));
        glLineWidth(20);
        glDrawElements(GL_LINES, m_abstract_skel_edges.size(), GL_UNSIGNED_INT, 0 );

        m_GSkeleton.vboRelease("index");
        m_GSkeleton.vaoRelease();
    } else {
        m_GSkeleton.vaoBind("SkeletonGraph");
        m_GSkeleton.useProgram("2D_nodes");
        updateAbstractUniformsLocation( m_GSkeleton.getProgram("2D_nodes") );
        glDrawArrays(GL_POINTS, 0,  m_abstract_skel_nodes.size() );

        m_GSkeleton.useProgram("23D_nodes");
        updateAbstractUniformsLocation( m_GSkeleton.getProgram("23D_nodes") );
        glDrawArrays(GL_POINTS, 0,  m_abstract_skel_nodes.size() );

        m_GSkeleton.vboBind("index");

        m_GSkeleton.useProgram("2D_index");
        updateAbstractUniformsLocation( m_GSkeleton.getProgram("2D_index"));
        glLineWidth(20);
        glDrawElements(GL_LINES, m_abstract_skel_edges.size(), GL_UNSIGNED_INT, 0 );

        m_GSkeleton.useProgram("23D_index");
        updateAbstractUniformsLocation( m_GSkeleton.getProgram("23D_index"));
        glLineWidth(20);
        glDrawElements(GL_LINES, m_abstract_skel_edges.size(), GL_UNSIGNED_INT, 0 );
        m_GSkeleton.vboRelease("index");
        m_GSkeleton.vaoRelease();
    }
}


// ----------------------------------------------------------------------------
//
// we initialize the vbos for drawing
bool OpenGLManager::initNeuritesGraphShaders()
{
    qDebug() << "OpenGLManager::initNeuritesGraphShaders()";

    if (m_glFunctionsSet == false)
        return false;

    m_GNeurites.createProgram("index");
    bool res = m_GNeurites.compileShader("index",
                                         ":/shaders/nodes_vert.glsl",
                                         ":/shaders/abstract_skeleton_line_geom.glsl",
                                         ":/shaders/points_3d_frag.glsl");
    if (res == false)
        return res;

    GL_Error();

    // allocate skeleton nodes
    m_GNeurites.vboCreate("nodes", Buffer_t::VERTEX, Buffer_USAGE_t::STATIC);
//    m_NeuritesNodesVBO.setUsagePattern( QOpenGLBuffer::DynamicDraw );

    m_GNeurites.vboBind("nodes");

    m_GNeurites.vboAllocate("nodes",
                            m_neurites_nodes.data(),
                            m_neurites_nodes.size() * sizeof(GLuint));

    GL_Error();
    m_GNeurites.vboRelease("nodes");


    // allocate neurites edges
    // allocate skeleton edges
    m_GNeurites.vboCreate("index", Buffer_t::INDEX, Buffer_USAGE_t::STATIC);
    m_GNeurites.vboBind("index");

    m_GNeurites.vboAllocate("index",
                            m_neurites_edges.data(),
                            m_neurites_edges.size() * sizeof(GLuint)  );

    m_GNeurites.vboRelease("index");

    GL_Error();

    // initialize buffers
    m_GNeurites.vaoCreate("ConnectivityGraph");
    m_GNeurites.vaoBind("ConnectivityGraph");

    m_GNeurites.vboBind("nodes");

    GL_Error();

    initNeuritesVertexAttribPointer();

    GL_Error();

    // initialize uniforms

    m_GNeurites.vboRelease("nodes");

    m_GNeurites.vboBind("index");

    m_GNeurites.useProgram("index");

    m_GNeurites.vboRelease("index");
    m_GNeurites.vaoRelease();

    return true;
}

// ----------------------------------------------------------------------------
//
// only the edges, because the skeleton itself will collabse into a node
void OpenGLManager::drawNeuritesGraph(struct GlobalUniforms grid_uniforms)
{
    if (m_glFunctionsSet == false)
        return;

    m_GNeurites.vaoBind("ConnectivityGraph");
    m_GNeurites.vboBind("nodes");

    m_uniforms = grid_uniforms;

    m_GNeurites.vboBind("index");

    m_GNeurites.useProgram("index");
    updateAbstractUniformsLocation( m_GNeurites.getProgram("index") );
    glLineWidth(20);

    glDrawElements(GL_LINES,  m_neurites_edges.size(), GL_UNSIGNED_INT, 0 );

    m_GNeurites.vboRelease("index");

    m_GNeurites.vboRelease("nodes");
    m_GNeurites.vaoRelease();
}

// ----------------------------------------------------------------------------
//
bool OpenGLManager::initMeshTrianglesShaders()
{
    qDebug() << "initMeshTrianglesShaders";
    m_TMesh.createProgram("3Dtriangles");
    bool res = m_TMesh.compileShader("3Dtriangles",
                                    ":/shaders/mesh_vert.glsl",
                                    ":/shaders/mesh_geom.glsl",
                                    ":/shaders/mesh_frag.glsl");
    if (res == false)
        return res;

    // create vbos and vaos
    m_TMesh.vaoCreate("Mesh");
    m_TMesh.vaoBind("Mesh");

    m_TMesh.useProgram("3Dtriangles");
    QVector3D lightDir = QVector3D(-2.5f, -2.5f, -0.9f);
    GLuint lightDir_loc = glGetUniformLocation(m_TMesh.getProgram("3Dtriangles"),
                                               "diffuseLightDirection");

    glUniform3fv(lightDir_loc, 1, &lightDir[0]);

    m_TMesh.vboCreate("MeshVertices", Buffer_t::VERTEX, Buffer_USAGE_t::STATIC);
    m_TMesh.vboBind("MeshVertices");

    Mesh* mesh = m_dataContainer->getMeshPointer();
    mesh->allocateVerticesVBO( m_TMesh.getVBO("MeshVertices") );

    initMeshVertexAttrib();


    m_TMesh.vboRelease("MeshVertices");
    m_TMesh.vaoRelease();


    /*  start selection buffer **/
    m_TMesh.vaoCreate("Selection");
    m_TMesh.vaoBind("Selection");


    m_TMesh.createProgram("selection");
    res = m_TMesh.compileShader("selection",
                                ":/shaders/mesh_vert.glsl",
                                ":/shaders/mesh_geom.glsl",
                                ":/shaders/hvgx_selection_frag.glsl");
    if (res == false)
        return res;

    m_TMesh.useProgram("selection");

    m_TMesh.vboBind("MeshVertices");
    initMeshVertexAttrib();
    m_TMesh.vboRelease("MeshVertices");


    m_TMesh.vaoRelease();

    GL_Error();
    /*  end selection buffer **/

}

// ----------------------------------------------------------------------------
//
void OpenGLManager::drawMeshTriangles(struct GlobalUniforms grid_uniforms, bool selection )
{
   m_uniforms = grid_uniforms;

   if (selection) {
       m_TMesh.vaoBind("Selection");
       m_TMesh.useProgram("selection");
       updateUniformsLocation(m_TMesh.getProgram("selection"));
       m_TMesh.vboBind("MeshIndices");
       glDrawElements(GL_TRIANGLES,  m_dataContainer->getMeshIndicesSize(),  GL_UNSIGNED_INT, 0 );
       m_TMesh.vboRelease("MeshIndices");
       m_TMesh.vaoRelease();
   } else {
       m_TMesh.vaoBind("Mesh");
       m_TMesh.useProgram("3Dtriangles");
       updateUniformsLocation(m_TMesh.getProgram("3Dtriangles"));

       m_TMesh.vboBind("MeshIndices");
       glDrawElements(GL_TRIANGLES,  m_dataContainer->getMeshIndicesSize(),  GL_UNSIGNED_INT, 0 );
       m_TMesh.vboRelease("MeshIndices");

       m_TMesh.vaoRelease();
   }


}

// ----------------------------------------------------------------------------
//
bool OpenGLManager::initSkeletonShaders()
{
    qDebug() << "OpenGLManager::initSkeletonShaders()";

    m_SkeletonPoints.createProgram("3DPoints");
    bool res = m_SkeletonPoints.compileShader("3DPoints",
                                              ":/shaders/skeleton_point_vert.glsl",
                                              ":/shaders/skeleton_point_geom.glsl",
                                              ":/shaders/points_3d_frag.glsl");

    if (res == false)
        return res;

    m_SkeletonPoints.vaoCreate("SkeletonPoints");
    m_SkeletonPoints.vaoBind("SkeletonPoints");

    m_SkeletonPoints.useProgram("3DPoints");

    m_SkeletonPoints.vboBind("SkeletonPoints");

    GL_Error();

    int offset = 0;
    glEnableVertexAttribArray(0); // original position of the skeleton point
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(SkeletonPoint),  0);

    offset += sizeof(QVector4D);
    glEnableVertexAttribArray(1); // simplified skeleton end 1
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(SkeletonPoint),  (GLvoid*)offset);

    offset += sizeof(QVector4D);
    glEnableVertexAttribArray(2);   // simplified skeleton end 2
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(SkeletonPoint),  (GLvoid*)offset);

    GL_Error();

    m_SkeletonPoints.vboRelease("SkeletonPoints");
    m_SkeletonPoints.vaoRelease();


    /*   selection */
    m_SkeletonPoints.createProgram("selection");
    res = m_SkeletonPoints.compileShader("selection",
                                         ":/shaders/skeleton_point_vert.glsl",
                                         ":/shaders/skeleton_point_geom.glsl",
                                         ":/shaders/hvgx_selection_frag.glsl");

    if (res == false)
        return res;


    m_SkeletonPoints.vaoCreate("Selection");
    m_SkeletonPoints.vaoBind("Selection");


    m_SkeletonPoints.useProgram("selection");

    m_SkeletonPoints.vboBind("SkeletonPoints");

    GL_Error();

    offset = 0;
    glEnableVertexAttribArray(0); // original position of the skeleton point
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(SkeletonPoint),  0);

    offset += sizeof(QVector4D);
    glEnableVertexAttribArray(1); // simplified skeleton end 1
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(SkeletonPoint),  (GLvoid*)offset);

    offset += sizeof(QVector4D);
    glEnableVertexAttribArray(2);   // simplified skeleton end 2
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(SkeletonPoint),  (GLvoid*)offset);

    GL_Error();

    m_SkeletonPoints.vboRelease("SkeletonPoints");
    m_SkeletonPoints.vaoRelease();
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::drawSkeletonPoints(struct GlobalUniforms grid_uniforms, bool selection)
{
   // qDebug() << "OpenGLManager::drawSkeletonPoints";
    // I need this because vertex <-> skeleton mapping is not complete
    m_uniforms = grid_uniforms;

    if (selection) {
        m_SkeletonPoints.vaoBind("Selection");
        m_SkeletonPoints.useProgram("selection");
        updateUniformsLocation(m_SkeletonPoints.getProgram("selection"));
        glDrawArrays(GL_POINTS, 0,  m_dataContainer->getSkeletonPointsSize()  );
        m_SkeletonPoints.vaoRelease();
    } else {
        m_SkeletonPoints.vaoBind("SkeletonPoints");
        m_SkeletonPoints.useProgram("3DPoints");
        updateUniformsLocation(m_SkeletonPoints.getProgram("3DPoints"));
        glDrawArrays(GL_POINTS, 0,  m_dataContainer->getSkeletonPointsSize()  );
        m_SkeletonPoints.vaoRelease();
    }
}

// ----------------------------------------------------------------------------
//
bool OpenGLManager::initGlycogenPointsShaders()
{
    qDebug() << "OpenGLManager::initGlycogenPointsShaders()";

    if (m_glFunctionsSet == false)
        return false;

    m_GlycogenPoints.createProgram("3DPoints");
    bool res = m_GlycogenPoints.compileShader("3DPoints",
                                              ":/shaders/glycogen_vert.glsl",
                                              ":/shaders/glycogen_geom.glsl",
                                                ":/shaders/points_3d_frag.glsl");
    if (res == false)
        return false;

    GL_Error();

    // initialize buffers
    m_vao_glycogen.create();
    m_vao_glycogen.bind();

    m_vbo_glycogen.create();
    m_vbo_glycogen.setUsagePattern( QOpenGLBuffer::DynamicDraw );
    m_vbo_glycogen.bind();

    // fill glycogen points

    //std::map<int, Glycogen*> glycogenMap = m_dataContainer->getGlycogenMap();
	std::vector<VertexData*>* glycogenList = m_dataContainer->getGlycogenVertexDataPtr();
    //struct glycogen_datum {
    //    int ID;
    //    QVector4D center_diam;
    //};

    //std::vector<struct glycogen_datum> glycogen_data;
	std::vector<VertexData> glycogen_data;
	//for (auto iter = glycogenMap.begin(); iter != glycogenMap.end(); iter++ ) {
	for (auto iter = glycogenList->begin(); iter != glycogenList->end(); iter++) {
        //Glycogen*gc = (*iter).second;
		VertexData* vd = (*iter);
		//QVector4D center_diam = gc->getCenter();
        //center_diam.setW(gc->getRadius());
        //struct glycogen_datum gc_datum = {gc->getID(), center_diam};
        //glycogen_data.push_back(gc_datum);
		glycogen_data.push_back(*vd);
    }

    //m_vbo_glycogen.allocate( glycogen_data.data(),
    //                              glycogen_data.size() * sizeof(struct glycogen_datum) );

	m_vbo_glycogen.allocate(glycogen_data.data(), glycogen_data.size() * sizeof(struct VertexData));

    GL_Error();

    m_GlycogenPoints.useProgram("3DPoints");

    // init pointers
   /* int offset = 0;
    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, 1, GL_INT, 0, (void*)offset);

    offset +=  sizeof(int);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE,
                          sizeof(glycogen_datum), (GLvoid*)offset);*/

	int offset = 0;
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,
						  sizeof(struct VertexData), 0);


	offset += sizeof(QVector4D);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE,
						  sizeof(VertexData), (GLvoid*)offset);

    GL_Error();

    // initialize uniforms

    m_vbo_glycogen.release();
    m_vao_glycogen.bind();
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::drawGlycogenPoints(struct GlobalUniforms grid_uniforms)
{

    // I need this because transitioning from mesh to skeleton is not smooth
    m_vao_glycogen.bind();

    m_GlycogenPoints.useProgram("3DPoints");
    m_uniforms = grid_uniforms;

    updateUniformsLocation(m_GlycogenPoints.getProgram("3DPoints"));
    glDrawArrays(GL_POINTS, 0,  m_dataContainer->getGlycogenSize() );

    m_vao_glycogen.release();
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::updateGlycogenPoints()
{
	//m_vao_glycogen.bind();
	m_vbo_glycogen.bind();



	// fill glycogen points
	//std::map<int, Glycogen*> glycogenMap = m_dataContainer->getGlycogenMap();
	std::vector<VertexData*>* glycogenList = m_dataContainer->getGlycogenVertexDataPtr();
	//struct glycogen_datum {
	//    int ID;
	//    QVector4D center_diam;
	//};

	//std::vector<struct glycogen_datum> glycogen_data;
	std::vector<VertexData> glycogen_data;
	//for (auto iter = glycogenMap.begin(); iter != glycogenMap.end(); iter++ ) {
	for (auto iter = glycogenList->begin(); iter != glycogenList->end(); iter++) {
		//Glycogen*gc = (*iter).second;
		VertexData* vd = (*iter);
		//QVector4D center_diam = gc->getCenter();
		//center_diam.setW(gc->getRadius());
		//struct glycogen_datum gc_datum = {gc->getID(), center_diam};
		//glycogen_data.push_back(gc_datum);
		glycogen_data.push_back(*vd);
	}

	auto buffer_data = m_vbo_glycogen.map(QOpenGLBuffer::WriteOnly);

	memcpy(buffer_data, glycogen_data.data(), glycogen_data.size() * sizeof(struct VertexData));

	m_vbo_glycogen.unmap();
	//m_vbo_glycogen.allocate(glycogen_data.data(), glycogen_data.size() * sizeof(struct VertexData));

	/*int offset = 0;
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,
		sizeof(struct VertexData), 0);


	offset += sizeof(QVector4D);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
		sizeof(VertexData), (GLvoid*)offset);*/

	m_vbo_glycogen.release();
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::update2Dflag(bool is2D)
{
    m_2D = is2D;
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::drawAll(struct GlobalUniforms grid_uniforms)
{
    m_uniforms = grid_uniforms;
    write_ssbo_data();
    struct ast_neu_properties space_properties = m_2dspace->getSpaceProper();

    drawGlycogenPoints(grid_uniforms);


    if ( (space_properties.ast.render_type.x() == 1 &&  space_properties.neu.render_type.x() == 1) ) {
        glDisable (GL_BLEND);
        glBlendFunc (GL_ONE, GL_ONE);
        drawSkeletonPoints(grid_uniforms, false);
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    if ( (space_properties.ast.render_type.y() == 1 &&  space_properties.ast.render_type.x() == 0) || space_properties.neu.render_type.y() == 1 )
        drawSkeletonPoints(grid_uniforms, false); // transparency is allowed

    if ( space_properties.ast.render_type.z() == 1 ||  space_properties.neu.render_type.z() == 1)
        drawSkeletonsGraph(grid_uniforms, false);

    if ( space_properties.ast.render_type.w() == 1 ||  space_properties.neu.render_type.w() == 1) {
        drawSkeletonsGraph(grid_uniforms, false);
        drawNeuritesGraph(grid_uniforms);
    }

    if ( space_properties.ast.render_type.x() == 1 ||  space_properties.neu.render_type.x() == 1)
        drawMeshTriangles(grid_uniforms, false);

    glBindFramebuffer(GL_FRAMEBUFFER, m_selectionFrameBuffer);
    //clear
    glClear(GL_COLOR_BUFFER_BIT);
    //disable dithering -- important
    glDisable(GL_DITHER);
    glDisable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);

    //render graph
    drawMeshTriangles(m_uniforms, true);
    drawSkeletonPoints(m_uniforms, true);
    drawSkeletonsGraph(m_uniforms, true);

    //enable dithering again
    glEnable(GL_DITHER);
    glEnable(GL_MULTISAMPLE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


}

// ----------------------------------------------------------------------------
//
void OpenGLManager::updateAbstractUniformsLocation(GLuint program)
{
    if (m_glFunctionsSet == false)
        return;

    // initialize uniforms
    GLuint mMatrix = glGetUniformLocation(program, "mMatrix");
    GLuint m_noRartionMatrix = glGetUniformLocation(program, "m_noRartionMatrix");
    GLuint vMatrix = glGetUniformLocation(program, "vMatrix");
    GLint is2D = glGetUniformLocation(program, "is2D");
    int is2D_value;

    if (m_2D) { // force directed layout started, them use model without rotation
        is2D_value = 1;
    } else {
        is2D_value = 0;
    }
    glUniform1iv(is2D, 1, &is2D_value);

    glUniformMatrix4fv(m_noRartionMatrix, 1, GL_FALSE, m_uniforms.modelNoRotMatrix);
    glUniformMatrix4fv(mMatrix, 1, GL_FALSE, m_uniforms.mMatrix);

    glUniformMatrix4fv(vMatrix, 1, GL_FALSE, m_uniforms.vMatrix);

    GLuint pMatrix = glGetUniformLocation(program, "pMatrix");
    glUniformMatrix4fv(pMatrix, 1, GL_FALSE, m_uniforms.pMatrix);

    GLint y_axis = glGetUniformLocation(program, "y_axis");
    glUniform1iv(y_axis, 1, &m_uniforms.y_axis);

    GLint x_axis = glGetUniformLocation(program, "x_axis");
    glUniform1iv(x_axis, 1, &m_uniforms.x_axis);

    GLint viewport = glGetUniformLocation(program, "viewport");
    float viewport_values[4];
    viewport_values[0] = m_uniforms.viewport.x();
    viewport_values[1] = m_uniforms.viewport.y();
    viewport_values[2] = m_uniforms.viewport.z();
    viewport_values[3] = m_uniforms.viewport.w();

    glUniform4fv(viewport, 1,  viewport_values);

    glUniform1iv(8, 1, &m_uniforms.max_volume);

    glUniform1iv(9, 1, &m_uniforms.max_astro_coverage);
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::updateUniformsLocation(GLuint program)
{
    if (m_glFunctionsSet == false)
        return;

   // qDebug() << "OpenGLManager::updateUniformsLocation";
    // initialize uniforms
    GLuint mMatrix = glGetUniformLocation(program, "mMatrix");
    glUniformMatrix4fv(mMatrix, 1, GL_FALSE, m_uniforms.mMatrix);

	GLuint m_noRartionMatrix = glGetUniformLocation(program, "m_noRartionMatrix");
	glUniformMatrix4fv(m_noRartionMatrix, 1, GL_FALSE, m_uniforms.modelNoRotMatrix);

    GLuint vMatrix = glGetUniformLocation(program, "vMatrix");
    glUniformMatrix4fv(vMatrix, 1, GL_FALSE, m_uniforms.vMatrix);

    GLuint pMatrix = glGetUniformLocation(program, "pMatrix");
    glUniformMatrix4fv(pMatrix, 1, GL_FALSE, m_uniforms.pMatrix);

    GLint y_axis = glGetUniformLocation(program, "y_axis");
    glUniform1iv(y_axis, 1, &m_uniforms.y_axis);

    GLint x_axis = glGetUniformLocation(program, "x_axis");
    glUniform1iv(x_axis, 1, &m_uniforms.x_axis);


    glUniform1iv(8, 1, &m_uniforms.max_volume);
    glUniform1iv(9, 1, &m_uniforms.max_astro_coverage);
}


// ----------------------------------------------------------------------------
//
void OpenGLManager::update_ssbo_data_layout1(QVector2D layout1, int hvgxID)
{
    if (m_ssbo_data.size() < hvgxID)
        return;

    m_ssbo_data[hvgxID].layout1 = layout1;
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::update_ssbo_data_layout2(QVector2D layout2, int hvgxID)
{
    if (m_ssbo_data.size() < hvgxID)
        return;

    m_ssbo_data[hvgxID].layout2 = layout2;
}

// ----------------------------------------------------------------------------
//
// Graph_t::ALL_SKELETONS, dont discriminate between types
void OpenGLManager::update_skeleton_layout1(QVector2D layout1,  long node_index, int hvgxID)
{
    // get the object -> get its skeleton -> update the layout
    std::map<int, Object*> objects_map = m_dataContainer->getObjectsMap();

    if (objects_map.find(hvgxID) == objects_map.end()) {
        return;
    }

    Skeleton *skel = objects_map[hvgxID]->getSkeleton();
    if (skel == NULL) {
        qDebug() << "No Skeleton " << hvgxID;
        return;
    }

    if (node_index  > m_abstract_skel_nodes.size()) {
        qDebug() << node_index << " out of range " << m_abstract_skel_nodes.size();
        return;
    }

    m_abstract_skel_nodes[node_index].layout1 = layout1;
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::update_skeleton_layout2(QVector2D layout2,  long node_index, int hvgxID)
{
    // get the object -> get its skeleton -> update the layout
    // get the object -> get its skeleton -> update the layout
    std::map<int, Object*> objects_map = m_dataContainer->getObjectsMap();
    if (objects_map.find(hvgxID) == objects_map.end()) {
        qDebug() << "Object not found";
        return;
    }

    Skeleton *skel = objects_map[hvgxID]->getSkeleton();
    if (skel == NULL) {
        qDebug() << "No Skeleton " << hvgxID;
        return;
    }

    if (node_index  > m_abstract_skel_nodes.size()) {// since no astrpcyte I should never get here
        qDebug() <<node_index << " out of range " << m_abstract_skel_nodes.size();
        return;
    }

    m_abstract_skel_nodes[node_index].layout2 = layout2;
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::update_skeleton_layout3(QVector2D layout3,  long node_index, int hvgxID)
{
    // get the object -> get its skeleton -> update the layout
    // get the object -> get its skeleton -> update the layout
    std::map<int, Object*> objects_map = m_dataContainer->getObjectsMap();
    if (objects_map.find(hvgxID) == objects_map.end()) {
        return;
    }

    Skeleton *skel = objects_map[hvgxID]->getSkeleton();
    if (skel == NULL) {
        qDebug() << "No Skeleton " << hvgxID;
        return;
    }

    if (node_index  > m_abstract_skel_nodes.size()) {
        qDebug() << node_index << " out of range "<< m_abstract_skel_nodes.size();
        return;
    }

    m_abstract_skel_nodes[node_index].layout3 = layout3;
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::multiplyWithRotation(QMatrix4x4 rotationMatrix)
{
    for (int i = 0; i < m_abstract_skel_nodes.size(); ++i) {
        QVector3D rotVertex = rotationMatrix * m_abstract_skel_nodes[i].vertex.toVector3D();
        m_abstract_skel_nodes[i].layout1 = rotVertex.toVector2D();
        m_abstract_skel_nodes[i].layout2 = rotVertex.toVector2D();
        m_abstract_skel_nodes[i].layout3 = rotVertex.toVector2D();

    }
}

// ----------------------------------------------------------------------------
//
Object_t OpenGLManager::getObjectTypeByID(int hvgxID)
{
    if (m_dataContainer == NULL)
        return Object_t::UNKNOWN;

    return m_dataContainer->getObjectTypeByID(hvgxID);
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::FilterByType(Object_t type)
{
    // this should be fast, I should somehow group by IDs for types, and when we filter by type I just get all IDs with this type and set them to 1 which means filtered
    std::vector<Object*> objects_list = m_dataContainer->getObjectsByType(type);
    qDebug() << "Filtering " << objects_list.size() << " objects";

    for (int i = 0; i < objects_list.size(); i++) {
        int hvgxID = objects_list[i]->getHVGXID();
        qDebug() << "Filtering ID " << hvgxID;
        if (m_ssbo_data[hvgxID].info.w() == 1) {
            FilterObject(hvgxID, false);
        } else {
            FilterObject(hvgxID, true);
        }
    }

    m_dataContainer->recomputeMaxVolAstro();
    updateSSBO();}

// ----------------------------------------------------------------------------
//
void OpenGLManager::recursiveFilter(int hvgxID, bool isfilterd)
{
    std::map<int, Object*>  objectMap = m_dataContainer->getObjectsMap();
    if (objectMap.find(hvgxID) == objectMap.end() || objectMap[hvgxID] == NULL) {
        return;
    }

    FilterObject(hvgxID, isfilterd);

    if (isfilterd) // hide
        return;

    Object *obj = objectMap[hvgxID];

    // if it has a children then get them
    // else if has parent get them
    if (m_display_parent) {
        Object *parent = obj->getParent();
        if (parent == NULL) {
            qDebug() << obj->getName().data() << " has no parnt";
        } else {
            FilterObject(parent->getHVGXID(), isfilterd);
        }
    }


    if (m_display_child) {
        std::vector<Object*> children = obj->getChildren();
        if (children.size() == 0) {
            qDebug() << obj->getName().data() << " has no child";
        } else {
            for (int i = 0; i < children.size(); ++i) {
                qDebug() << "Showing " << children[i]->getName().data();
                FilterObject(children[i]->getHVGXID(), isfilterd);
            }
        }
    }

    if (m_display_synapses) {
        // get all objects that are connected to this directly or indirectly
        std::vector<Object*> synapses_list= obj->getSynapses();
        if (synapses_list.size() == 0)
            return;

        for (int i = 0; i < synapses_list.size(); ++i) {
            Object *synapse_obj = synapses_list[i];
            struct synapse synapse_data = synapse_obj->getSynapseData();
            if (synapse_data.axon != hvgxID
                && synapse_data.axon
                && objectMap.find(synapse_data.axon) != objectMap.end()) {
                FilterObject(synapse_data.axon, isfilterd);
            }

            if (synapse_data.dendrite != hvgxID
                && synapse_data.dendrite
                && objectMap.find(synapse_data.dendrite) != objectMap.end()) {
                FilterObject(synapse_data.dendrite, isfilterd);
            }

            if (synapse_data.spine != hvgxID
                && synapse_data.spine
                && objectMap.find(synapse_data.spine) != objectMap.end()) {
                FilterObject(synapse_data.spine, isfilterd);
            }

            if (synapse_data.bouton != hvgxID
                && synapse_data.bouton
                && objectMap.find(synapse_data.bouton) != objectMap.end()) {
                FilterObject(synapse_data.bouton, isfilterd);
            }
        }
    }

}

// ----------------------------------------------------------------------------
//
// if we filter the object we need to recompute the maximum astrocyte coverage and volume
void OpenGLManager::FilterObject(int ID, bool isfilterd)
{
    std::map<int, Object*>  objectMap = m_dataContainer->getObjectsMap();
    if ( ID > m_ssbo_data.size() ||
         objectMap.find(ID) == objectMap.end() ||
         objectMap[ID] == NULL )
    {
        // no need to filter something that doesnt exist
        return;
    }

    Object *obj = objectMap[ID];
    obj->updateFilteredFlag(isfilterd);


    if (isfilterd) {
        m_ssbo_data[ID].info.setW(1);
    } else {
        m_ssbo_data[ID].info.setW(0);
    }
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::FilterByID( QList<QString> tokens_Ids )
{
    for (int i = 0; i < m_ssbo_data.size(); ++i) {
        FilterObject(i, true);
    }

    for (int i = 0; i < tokens_Ids.size(); ++i) {
        int hvgxID = tokens_Ids[i].toInt();
        if (hvgxID > m_ssbo_data.size())
            continue;

        recursiveFilter(hvgxID, false);
    }

    m_dataContainer->recomputeMaxVolAstro();
    updateSSBO();}

// ----------------------------------------------------------------------------
//
void OpenGLManager::FilterByID( std::vector<int> tokens_Ids )
{
    for (int i = 0; i < m_ssbo_data.size(); ++i) {
        // recompute max vol and astro coverage
        FilterObject(i, true);
    }

    for (int i = 0; i < tokens_Ids.size(); ++i) {
        int hvgxID = tokens_Ids[i];
        if (hvgxID > m_ssbo_data.size())
            continue;

        recursiveFilter(hvgxID, false);

    }

    m_dataContainer->recomputeMaxVolAstro();
    updateSSBO();}

// ----------------------------------------------------------------------------
//
void OpenGLManager::showAll()
{
    for (int i = 0; i < m_ssbo_data.size(); ++i) {
        // recompute max vol and astro coverage
        FilterObject(i, false);
    }

    m_dataContainer->recomputeMaxVolAstro();
    updateSSBO();
}

void OpenGLManager::setZoom(float zoom)
{
	m_zoom = zoom;
}


void OpenGLManager::updateSSBO()
{
    std::map<int, Object*>  objectMap = m_dataContainer->getObjectsMap();

    for ( auto iter = objectMap.begin(); iter != objectMap.end(); iter++ ) {
        Object *obj = (*iter).second;
        if (obj->isFiltered() || obj->getObjectType() == Object_t::ASTROCYTE)
            continue;

        int hvgxID = obj->getHVGXID();
        if (m_ssbo_data.size() <= hvgxID)
            continue;

        float volume =  obj->getVolume() / m_dataContainer->getMaxVolume();

        float coverage =  obj->getAstroCoverage() / m_dataContainer->getMaxAstroCoverage();
        switch(m_size_encoding) {
        case Size_e::VOLUME:
            m_ssbo_data[hvgxID].info.setX( 20 *  volume);
            break;
        case Size_e::ASTRO_COVERAGE:
            m_ssbo_data[hvgxID].info.setX( 20 *  coverage);
            break;
        default:
            m_ssbo_data[hvgxID].info.setX( 20 *  volume);

        }

        switch(m_color_encoding) {
            case Color_e::TYPE:
            {
                QVector4D color = obj->getColor();
                m_ssbo_data[hvgxID].color.setX(color.x());
                m_ssbo_data[hvgxID].color.setY(color.y());
                m_ssbo_data[hvgxID].color.setZ(color.z());

                break;
            }
            case Color_e::ASTRO_COVERAGE:
            {
              QVector4D add_color = QVector4D(1, 1, 1, 0) * coverage;
              m_ssbo_data[hvgxID].color += QVector4D(0.2, 0.2, 0.2, 0) ;
              m_ssbo_data[hvgxID].color -= add_color;

              break;
            }
            case Color_e::FUNCTION:
            {
              m_ssbo_data[hvgxID].color.setX(1);
              break;
            }
        }


    }
}


void OpenGLManager::updateNodeSizeEncoding(Size_e encoding)
{
    if (m_size_encoding == encoding)
        return;

    m_size_encoding = encoding;

    updateSSBO();
}
void OpenGLManager::updateColorEncoding(Color_e encoding)
{
    if (m_color_encoding == encoding)
        return;

    m_color_encoding = encoding;

    updateSSBO();
}
