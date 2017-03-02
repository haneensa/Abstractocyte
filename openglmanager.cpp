#include "openglmanager.h"

OpenGLManager::OpenGLManager(DataContainer *obj_mnger)
    : m_vbo_skeleton( QOpenGLBuffer::VertexBuffer ),
      m_vbo_mesh( QOpenGLBuffer::VertexBuffer ),
      m_vbo_IndexMesh(QOpenGLBuffer::IndexBuffer),
      m_NeuritesIndexVBO( QOpenGLBuffer::IndexBuffer ),
      m_NeuritesNodesVBO( QOpenGLBuffer::VertexBuffer ),
      m_SkeletonsIndexVBO( QOpenGLBuffer::IndexBuffer ),
      m_SkeletonsNodesVBO( QOpenGLBuffer::VertexBuffer ),
      m_2D(false),
      m_bindIdx(2) // ssbo biding point
{
    m_dataContainer = obj_mnger;
    m_glFunctionsSet = false;
}

OpenGLManager::~OpenGLManager()
{
    if (m_glFunctionsSet == false)
        return;

    glDeleteProgram(m_program_skeleton);
    glDeleteProgram(m_program_mesh);
    glDeleteProgram(m_program_mesh_points);

    m_vao_mesh.destroy();
    m_vbo_mesh.destroy();

    m_NeuritesGraphVAO.destroy();
    m_NeuritesNodesVBO.destroy();
    m_NeuritesIndexVBO.destroy();

}

bool OpenGLManager::initOpenGLFunctions()
{
    m_glFunctionsSet = true;
    initializeOpenGLFunctions();

    fillVBOsData();

    initSSBO();

    // *********** 3) Skeleton Points    ***********
    initSkeletonShaders();
    initMeshTrianglesShaders();
    initMeshPointsShaders();
    initAbstractSkeletonShaders();
    initNeuritesGraphShaders();

    return true;
}


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

void OpenGLManager::write_ssbo_data()
{
    int bufferSize =  m_ssbo_data.size() * sizeof(struct ssbo_mesh);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
    GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
    memcpy(p,   m_ssbo_data.data(),  bufferSize);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}


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

    m_vbo_skeleton.create();
    m_vbo_skeleton.setUsagePattern( QOpenGLBuffer::StaticDraw);
    m_vbo_skeleton.bind();
    m_vbo_skeleton.allocate(NULL, m_dataContainer->getSkeletonPointsSize()  * sizeof(SkeletonPoint));
    int vbo_IndexMesh_offset = 0;

    // initialize index buffers
    m_vbo_IndexMesh.create();
    m_vbo_IndexMesh.bind();
    m_vbo_IndexMesh.allocate( NULL, m_dataContainer->getMeshIndicesSize() * sizeof(GLuint) );
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

        qDebug() << " allocating: " << object_p->getName().data();

        // allocating mesh indices
        int vbo_IndexMesh_count = object_p->get_indices_Size() * sizeof(GLuint);
        m_vbo_IndexMesh.write(vbo_IndexMesh_offset, object_p->get_indices(), vbo_IndexMesh_count);
        vbo_IndexMesh_offset += vbo_IndexMesh_count;

        // allocating skeleton vertices, if this object hash no skeleton, then this will return and wont write anything
        int vbo_skeleton_count = object_p->writeSkeletontoVBO(m_vbo_skeleton, vbo_skeleton_offset);
        vbo_skeleton_offset += vbo_skeleton_count;

        // allocate neurites nodes place holders
        if (object_p->getObjectType() != Object_t::ASTROCYTE
                && object_p->getObjectType() != Object_t::MITO
                && object_p->getObjectType()  != Object_t::SYNAPSE) {
            object_p->setNodeIdx(m_neurites_nodes.size());
            m_neurites_nodes.push_back(ID);
        }


        // initialize abstract skeleton data
        // find a way to fill the skeleton with data
        // get skeleton of the object
        // if no skeleton, this will be skipped, thus no graph for this object
        Skeleton *skeleton = object_p->getSkeleton(); // not null as long as the object exist
        std::vector<QVector3D> nodes3D = skeleton->getGraphNodes();
        std::vector<QVector2D> edges2D = skeleton->getGraphEdges();
        qDebug() << "set offset: " << m_abstract_skel_nodes.size();
        object_p->setSkeletonOffset(m_abstract_skel_nodes.size());
        // each skeleton node has local index within its list of nodes
        // and an offset within list of abstract skel nodes
        // add nodes
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

    m_vbo_IndexMesh.release();
    m_vbo_skeleton.release();

    // allocate skeleton nodes
    m_SkeletonsNodesVBO.create();
    m_SkeletonsNodesVBO.setUsagePattern( QOpenGLBuffer::DynamicDraw );
    m_SkeletonsNodesVBO.bind();

    m_SkeletonsNodesVBO.allocate( m_abstract_skel_nodes.data(),
                                  m_abstract_skel_nodes.size() * sizeof(struct AbstractSkelNode) );
    GL_Error();
    m_SkeletonsNodesVBO.release();

    // allocate skeleton edges
    m_SkeletonsIndexVBO.create();
    m_SkeletonsIndexVBO.bind();

    m_SkeletonsIndexVBO.allocate( m_abstract_skel_edges.data(),
                                  m_abstract_skel_edges.size() * sizeof(GLuint) );
    m_SkeletonsIndexVBO.release();
    GL_Error();

    // allocate neurites nodes
    m_NeuritesNodesVBO.create();
    m_NeuritesNodesVBO.setUsagePattern( QOpenGLBuffer::DynamicDraw );
    m_NeuritesNodesVBO.bind();

    m_NeuritesNodesVBO.allocate( m_neurites_nodes.data(),
                            m_neurites_nodes.size() * sizeof(GLuint) );
    m_NeuritesNodesVBO.release();


    // allocate neurites edges
    m_NeuritesIndexVBO.create();
    m_NeuritesIndexVBO.bind();

    m_NeuritesIndexVBO.allocate( m_neurites_edges.data(),
                                 m_neurites_edges.size() * sizeof(GLuint) );

    m_NeuritesIndexVBO.release();
    GL_Error();


    qDebug() << "m_abstract_skel_nodes.size(): " << m_abstract_skel_nodes.size();
    qDebug() << "m_abstract_skel_edges.size(): " << m_abstract_skel_edges.size();
    qDebug() << "m_neurites_edges.size(): " << m_neurites_edges.size();
    qDebug() << "m_neurites_nodes.size(): " << m_neurites_nodes.size();


}

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

bool OpenGLManager::initAbstractSkeletonShaders()
{
    qDebug() << "OpenGLManager::initAbstractSkeletonShaders()";

    if (m_glFunctionsSet == false)
        return false;

    qDebug() << "nodes shader";
    m_program_skeletons_2D_nodes = glCreateProgram();
    bool res = initShader(m_program_skeletons_2D_nodes,
                          ":/shaders/abstract_skeleton_node_2D_vert.glsl",
                          ":/shaders/abstract_skeleton_node_geom.glsl",
                          ":/shaders/points_3d_frag.glsl");
    if (res == false)
        return res;

    qDebug() << "index shader";
    m_program_skeletons_2D_index = glCreateProgram();
    res = initShader(m_program_skeletons_2D_index,
                     ":/shaders/abstract_skeleton_node_2D_vert.glsl",
                     ":/shaders/abstract_skeleton_line_geom.glsl",
                     ":/shaders/points_passthrough_frag.glsl");

    if (res == false)
        return res;


    m_program_skeletons_23D_nodes = glCreateProgram();
    res = initShader(m_program_skeletons_23D_nodes,
                          ":/shaders/abstract_skeleton_node_transition_vert.glsl",
                          ":/shaders/abstract_skeleton_node_geom.glsl",
                          ":/shaders/points_3d_frag.glsl");
    if (res == false)
        return res;

    qDebug() << "index shader";
    m_program_skeletons_23D_index = glCreateProgram();
    res = initShader(m_program_skeletons_23D_index,
                     ":/shaders/abstract_skeleton_node_transition_vert.glsl",
                     ":/shaders/abstract_skeleton_line_geom.glsl",
                     ":/shaders/points_passthrough_frag.glsl");

    if (res == false)
        return res;

    m_SkeletonsGraphVAO.create();
    m_SkeletonsGraphVAO.bind();

    m_SkeletonsNodesVBO.bind();

    glUseProgram(m_program_skeletons_2D_nodes);
    initSkeletonsVertexAttribPointer();

    glUseProgram(m_program_skeletons_23D_nodes);
    initSkeletonsVertexAttribPointer();

    GL_Error();

    m_SkeletonsNodesVBO.release();

    m_SkeletonsIndexVBO.bind();
    glUseProgram(m_program_skeletons_2D_index);
    glUseProgram(m_program_skeletons_23D_index);

    m_SkeletonsIndexVBO.release();
    m_SkeletonsGraphVAO.release();
}


// only the edges, the nodes itself they are not needed to be visible
// this will collabse into a node for the neurites at the most abstract view
void OpenGLManager::drawSkeletonsGraph(struct GlobalUniforms grid_uniforms)
{
    if (m_glFunctionsSet == false)
        return;

    // update skeleton data from object manager
    m_uniforms = grid_uniforms;

    m_SkeletonsGraphVAO.bind();
    m_SkeletonsNodesVBO.bind();

    m_SkeletonsNodesVBO.allocate( m_abstract_skel_nodes.data(),
                                  m_abstract_skel_nodes.size() * sizeof(struct AbstractSkelNode) );

    glUseProgram(m_program_skeletons_2D_nodes);
    updateAbstractUniformsLocation(m_program_skeletons_2D_nodes);
    glDrawArrays(GL_POINTS, 0,  m_abstract_skel_nodes.size() );

    glUseProgram(m_program_skeletons_23D_nodes);
    updateAbstractUniformsLocation(m_program_skeletons_23D_nodes);
    glDrawArrays(GL_POINTS, 0,  m_abstract_skel_nodes.size() );

    m_SkeletonsIndexVBO.bind();

    glUseProgram(m_program_skeletons_2D_index);
    updateAbstractUniformsLocation(m_program_skeletons_2D_index);
    glDrawElements(GL_LINES, m_abstract_skel_edges.size(), GL_UNSIGNED_INT, 0 );

    glUseProgram(m_program_skeletons_23D_index);
    updateAbstractUniformsLocation(m_program_skeletons_23D_index);
    glDrawElements(GL_LINES, m_abstract_skel_edges.size(), GL_UNSIGNED_INT, 0 );

    m_SkeletonsIndexVBO.release();


    m_SkeletonsNodesVBO.release();
    m_SkeletonsGraphVAO.release();
}


bool OpenGLManager::initMeshTrianglesShaders()
{
    qDebug() << "initMeshTrianglesShaders";
    m_program_mesh = glCreateProgram();
    bool res = initShader(m_program_mesh, ":/shaders/mesh_vert.glsl",
                                          ":/shaders/mesh_geom.glsl",
                                          ":/shaders/mesh_frag.glsl");
    if (res == false)
        return res;

    // create vbos and vaos
    m_vao_mesh.create();
    m_vao_mesh.bind();

    glUseProgram(m_program_mesh); // m_program_mesh->bind();

    QVector3D lightDir = QVector3D(-2.5f, -2.5f, -0.9f);
    GLuint lightDir_loc = glGetUniformLocation(m_program_mesh, "diffuseLightDirection");
    glUniform3fv(lightDir_loc, 1, &lightDir[0]);

    m_vbo_mesh.create();
    m_vbo_mesh.setUsagePattern( QOpenGLBuffer::StaticDraw );
    if ( !m_vbo_mesh.bind() ) {
        qDebug() << "Could not bind vertex buffer to the context.";
    }

    Mesh* mesh = m_dataContainer->getMeshPointer();
    mesh->allocateVerticesVBO(m_vbo_mesh);

    // initialize index buffers
    m_vbo_IndexMesh.bind();
    m_vbo_IndexMesh.release();

    initMeshVertexAttrib();

    m_vbo_mesh.release();
    m_vao_mesh.release();

}

void OpenGLManager::drawMeshTriangles(struct GlobalUniforms grid_uniforms)
{
//   qDebug() << "OpenGLManager::drawMeshTriangles()";

   m_vao_mesh.bind();
   glUseProgram(m_program_mesh);
   m_uniforms = grid_uniforms;

   updateUniformsLocation(m_program_mesh);

   m_vbo_IndexMesh.bind();
   glDrawElements(GL_TRIANGLES,  m_dataContainer->getMeshIndicesSize(),  GL_UNSIGNED_INT, 0 );
   m_vbo_IndexMesh.release();
   m_vao_mesh.release();
}

bool OpenGLManager::initMeshPointsShaders()
{
    qDebug() << "OpenGLManager::initMeshPointsShaders()";
    m_program_mesh_points = glCreateProgram();
    bool res = initShader(m_program_mesh_points, ":/shaders/mesh_vert.glsl",
                                                 ":/shaders/mesh_points_geom.glsl",
                                                 ":/shaders/points_3d_frag.glsl");
    if (res == false)
        return res;

    // create vbos and vaos
    m_vao_mesh_points.create();
    m_vao_mesh_points.bind();

    glUseProgram(m_program_mesh_points);

    QVector3D lightDir = QVector3D(-2.5f, -2.5f, -0.9f);
    GLuint lightDir_loc = glGetUniformLocation(m_program_mesh_points, "diffuseLightDirection");
    glUniform3fv(lightDir_loc, 1, &lightDir[0]);

    m_vbo_mesh.bind();

    initMeshVertexAttrib();

    m_vbo_mesh.release();
    m_vao_mesh_points.release();
}

void OpenGLManager::drawMeshPoints(struct GlobalUniforms grid_uniforms)
{
   // qDebug() << "OpenGLManager::drawMeshPoints()";


    // I need this because transitioning from mesh to skeleton is not smooth
    m_vao_mesh_points.bind();
    glUseProgram(m_program_mesh_points);
    m_uniforms = grid_uniforms;

    updateUniformsLocation(m_program_mesh_points);

    m_vbo_IndexMesh.bind();
    glDrawElements(GL_POINTS,  m_dataContainer->getMeshIndicesSize(),  GL_UNSIGNED_INT, 0 );
    m_vbo_IndexMesh.release();

    m_vao_mesh_points.release();

}


bool OpenGLManager::initSkeletonShaders()
{
    qDebug() << "OpenGLManager::initSkeletonShaders()";

    m_program_skeleton = glCreateProgram();
    bool res = initShader(m_program_skeleton, ":/shaders/skeleton_point_vert.glsl",
                                              ":/shaders/skeleton_point_geom.glsl",
                                              ":/shaders/points_3d_frag.glsl");
    if (res == false)
        return res;

    m_vao_skeleton.create();
    m_vao_skeleton.bind();

    glUseProgram(m_program_skeleton);

    m_vbo_skeleton.bind();

    GL_Error();

    int offset = 0;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(SkeletonPoint),  0);

    offset += sizeof(QVector4D);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(SkeletonPoint),  (GLvoid*)offset);

    offset += sizeof(QVector4D);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(SkeletonPoint),  (GLvoid*)offset);

    GL_Error();

    m_vbo_skeleton.release();
    m_vao_skeleton.release();
}


void OpenGLManager::drawSkeletonPoints(struct GlobalUniforms grid_uniforms)
{
   // qDebug() << "OpenGLManager::drawSkeletonPoints";
    // I need this because vertex <-> skeleton mapping is not complete
    m_vao_skeleton.bind();
    glUseProgram(m_program_skeleton);

    m_uniforms = grid_uniforms;
    updateUniformsLocation(m_program_skeleton);

    glDrawArrays(GL_POINTS, 0,  m_dataContainer->getSkeletonPointsSize()  );
    m_vao_skeleton.release();

}


// we initialize the vbos for drawing
bool OpenGLManager::initNeuritesGraphShaders()
{
    qDebug() << "OpenGLManager::initNeuritesGraphShaders()";

    if (m_glFunctionsSet == false)
        return false;


    m_program_neurites_nodes = glCreateProgram();
    bool res = initShader(m_program_neurites_nodes,  ":/shaders/nodes_vert.glsl",
                      ":/shaders/abstract_skeleton_node_geom.glsl",
                      ":/shaders/points_3d_frag.glsl");
    if (res == false)
        return false;

    m_program_neurites_index = glCreateProgram();
    res = initShader(m_program_neurites_index,  ":/shaders/nodes_vert.glsl",
                                       ":/shaders/abstract_skeleton_line_geom.glsl",
                                       ":/shaders/points_passthrough_frag.glsl");
    if (res == false)
        return res;

    GL_Error();

    // initialize buffers
    m_NeuritesGraphVAO.create();
    m_NeuritesGraphVAO.bind();

    m_NeuritesNodesVBO.bind();
    glUseProgram(m_program_neurites_nodes);


    GL_Error();

    initNeuritesVertexAttribPointer();

    GL_Error();

    // initialize uniforms

    m_NeuritesNodesVBO.release();

    m_NeuritesIndexVBO.bind();

    glUseProgram(m_program_neurites_index);

    m_NeuritesIndexVBO.release();
    m_NeuritesGraphVAO.release();

    return true;
}

// only the edges, because the skeleton itself will collabse into a node
void OpenGLManager::drawNeuritesGraph(struct GlobalUniforms grid_uniforms)
{
    if (m_glFunctionsSet == false)
        return;


    m_NeuritesGraphVAO.bind();
    m_NeuritesNodesVBO.bind();

    m_uniforms = grid_uniforms;

    glUseProgram(m_program_neurites_nodes);
    updateAbstractUniformsLocation(m_program_neurites_nodes);

    glDrawArrays(GL_POINTS, 0,  m_neurites_nodes.size() );

    m_NeuritesIndexVBO.bind();

    glUseProgram(m_program_neurites_index);
    updateAbstractUniformsLocation(m_program_neurites_index);

    glDrawElements(GL_LINES,  m_neurites_edges.size(), GL_UNSIGNED_INT, 0 );

    m_NeuritesIndexVBO.release();

    m_NeuritesNodesVBO.release();
    m_NeuritesGraphVAO.release();
}

void OpenGLManager::update2Dflag(bool is2D)
{
    m_2D = is2D;
}

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

}

void OpenGLManager::drawAll(struct GlobalUniforms grid_uniforms)
{
    m_uniforms = grid_uniforms;
    write_ssbo_data();

    // 1) Mesh Triangles
    // 2) Mesh Points

    glDisable (GL_BLEND);
    glBlendFunc (GL_ONE, GL_ONE);
    drawMeshPoints(grid_uniforms);
    drawMeshTriangles(grid_uniforms);

    // 3) Skeleton Points
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    drawSkeletonPoints(grid_uniforms);
    // 3) Abstract Skeleton Graph (Nodes and Edges)
    drawSkeletonsGraph(grid_uniforms);
    // 4) Neurites Graph (Nodes and Edges)
    drawNeuritesGraph(grid_uniforms);
}

void OpenGLManager::updateUniformsLocation(GLuint program)
{
    if (m_glFunctionsSet == false)
        return;

   // qDebug() << "OpenGLManager::updateUniformsLocation";
    // initialize uniforms
    GLuint mMatrix = glGetUniformLocation(program, "mMatrix");
    glUniformMatrix4fv(mMatrix, 1, GL_FALSE, m_uniforms.mMatrix);

    GLuint vMatrix = glGetUniformLocation(program, "vMatrix");
    glUniformMatrix4fv(vMatrix, 1, GL_FALSE, m_uniforms.vMatrix);

    GLuint pMatrix = glGetUniformLocation(program, "pMatrix");
    glUniformMatrix4fv(pMatrix, 1, GL_FALSE, m_uniforms.pMatrix);

    GLint y_axis = glGetUniformLocation(program, "y_axis");
    glUniform1iv(y_axis, 1, &m_uniforms.y_axis);

    GLint x_axis = glGetUniformLocation(program, "x_axis");
    glUniform1iv(x_axis, 1, &m_uniforms.x_axis);
}


void OpenGLManager::update_ssbo_data_layout1(QVector2D layout1, int hvgxID)
{
    if (m_ssbo_data.size() < hvgxID)
        return;

    m_ssbo_data[hvgxID].layout1 = layout1;
}

void OpenGLManager::update_ssbo_data_layout2(QVector2D layout2, int hvgxID)
{
    if (m_ssbo_data.size() < hvgxID)
        return;

    m_ssbo_data[hvgxID].layout2 = layout2;
}

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

Object_t OpenGLManager::getObjectTypeByID(int hvgxID)
{
    if (m_dataContainer == NULL)
        return Object_t::UNKNOWN;

    return m_dataContainer->getObjectTypeByID(hvgxID);
}
