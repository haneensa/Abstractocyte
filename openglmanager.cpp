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
    m_renderGlycogenGranules = true;

    m_color_encoding = Color_e::TYPE;
    m_size_encoding = Size_e::VOLUME;
    m_normals_enabled = true;

    m_hoveredID = -1;

	m_is_astro_splat = true;
	m_is_glyco_splat = true;
	m_is_nmito_splat = true;
	m_is_amito_splat = true;

	m_is_specular = true;
}

// ----------------------------------------------------------------------------
//
OpenGLManager::~OpenGLManager()
{
    if (m_glFunctionsSet == false)
        return;

    m_vao_glycogen.destroy();
    m_vbo_glycogen.destroy();
}


// ############## Draw Call ###############################################
//
void OpenGLManager::drawAll()
{
    write_ssbo_data();


     render2DHeatMapTexture();

    renderAbstractions();
    renderSelection();

     drawNodesInto2DTexture();
 }

// ############## Data Initialization ###############################################
//
bool OpenGLManager::initOpenGLFunctions()
{
    m_glFunctionsSet = true;
    initializeOpenGLFunctions();

    m_GSkeleton.initOpenGLFunctions();
    m_TMesh.initOpenGLFunctions();
    m_SkeletonPoints.initOpenGLFunctions();
    m_GNeurites.initOpenGLFunctions();
    m_GlycogenPoints.initOpenGLFunctions();

    load3DTexturesFromRaw_3("mask_745_sigma3.raw", "astrocytic_mitochondria_s5.raw", "Neuronic_mitochondria_binary_s5.raw", m_splat_volume_3DTex, GL_TEXTURE2);
    //load3DTexturesFromRaw("mask_745_sigma3.raw", m_astro_3DTex ); // Astrocytic_mitochondria_b.raw
    //load3DTexturesFromRaw("astrocytic_mitochondria_s5.raw", m_mito_3DTex ); //
    //load3DTexturesFromRaw("Neuronic_mitochondria_binary_s5.raw", m_nmito_3DTex);
    load3DTexturesFromRaw("glycogen_volume_s5.raw", m_glycogen_3DTex, GL_TEXTURE3, 999, 999, 999);

    //init_Gly3DTex();
    //upload_Gly3DTex(m_dataContainer->getGlycogen3DGridData(), DIM_G, DIM_G, DIM_G);

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

// ----------------------------------------------------------------------------
//
void OpenGLManager::updateCanvasDim(int w, int h, int retianScale)
{
    if (m_canvas_h != h || m_canvas_w != w){
        m_canvas_h = h * retianScale;
        m_canvas_w = w * retianScale;
        m_retinaScale = retianScale;
        initSelectionFrameBuffer();
        init2DHeatMapTextures();
        GL_Error();
    }

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

    std::map<int, Object*> *objects_map = m_dataContainer->getObjectsMapPtr();
    for ( auto iter = objects_map->rbegin(); iter != objects_map->rend(); iter++) {
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
        m_ssbo_data[ID].info.setY( object_p->getAstroCoverage() / m_dataContainer->getMaxAstroCoverage() );

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
        // only show them if parent is filtered/same as mito

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
        if (objects_map->find(nID1) == objects_map->end()
            || objects_map->find(nID2) == objects_map->end()) {
            continue;
        }
        m_neurites_edges.push_back(objects_map->at(nID1)->getNodeIdx());
        m_neurites_edges.push_back(objects_map->at(nID2)->getNodeIdx());
    }

    m_TMesh.vboRelease("MeshIndices");
    m_SkeletonPoints.vboRelease("SkeletonPoints");

    // allocate neurites nodes


    qDebug() << "m_abstract_skel_nodes.size(): " << m_abstract_skel_nodes.size();
    qDebug() << "m_abstract_skel_edges.size(): " << m_abstract_skel_edges.size();
    qDebug() << "m_neurites_edges.size(): " << m_neurites_edges.size();
    qDebug() << "m_neurites_nodes.size(): " << m_neurites_nodes.size();


}

// ############## Textures ###############################################
//
void OpenGLManager::init_Gly3DTex()
{
    m_gly_3D_Tex = 0;

    glGenTextures(1, &m_gly_3D_Tex);
    glBindTexture(GL_TEXTURE_3D, m_gly_3D_Tex);
    GL_Error();

    //glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, 1000, 1000, 1000, 0, GL_RED, GL_FLOAT, 0);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    GL_Error();
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::upload_Gly3DTex(void* data, int sizeX, int sizeY, int sizeZ, GLenum type)
{
    glBindTexture(GL_TEXTURE_3D, m_gly_3D_Tex);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, sizeX, sizeY, sizeZ, 0, GL_RGBA, type, (GLvoid*)data);
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::init2DHeatMapTextures()
{
    // init Horizantal FBO
    m_2D_heatMap_FBO_H = 0;

    glGenFramebuffers(1, &m_2D_heatMap_FBO_H);
    glBindFramebuffer(GL_FRAMEBUFFER, m_2D_heatMap_FBO_H);
    GL_Error();

    glGenTextures(1, &m_2D_heatMap_Tex);
    glBindTexture(GL_TEXTURE_2D, m_2D_heatMap_Tex);
    GL_Error();

    // create empty image
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_canvas_w, m_canvas_h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL_Error();

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_2D_heatMap_Tex, 0);
    GL_Error();

    // set the list of draw buffers
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers);
    GL_Error();

    // check if frame buffer is ok
   if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
       qDebug() << "ERROR!!!!";
       return;
    }

    GL_Error();

    // init transfer function
    m_tf_2DHeatmap.push_back(QVector4D(0.8f, 0.0f, 0.0f, 0.0f)); // 0
    m_tf_2DHeatmap.push_back(QVector4D(0.5f, 0.0f, 0.5f, 0.9f)); // 1
    m_tf_2DHeatmap.push_back(QVector4D(1.0f, 0.0f, 0.0f, 1.0f)); // 2


    glGenTextures( 1, &m_tf_2DHeatMap_tex);
    GL_Error();

    glBindTexture( GL_TEXTURE_1D, m_tf_2DHeatMap_tex);
    GL_Error();

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    GL_Error();

    glTexImage1D( GL_TEXTURE_1D, 0, GL_RGBA, 3, 0, GL_RGBA, GL_FLOAT, m_tf_2DHeatmap.data());

    GL_Error();

}

// ----------------------------------------------------------------------------
//
void OpenGLManager::load3DTexturesFromRaw(QString path, GLuint &texture, GLenum texture_unit, int sizeX, int sizeY, int sizeZ)
{
    unsigned int size = sizeX * sizeY * sizeZ;
    unsigned char *rawData = (unsigned char *)m_dataContainer->loadRawFile(path, size);
    //load data into a 3D texture
    glGenTextures(1, &texture);
    glActiveTexture(texture_unit);
    glBindTexture(GL_TEXTURE_3D, texture);

     // set the texture parameters
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    //unsigned int size_4 = size * 4;

    unsigned char *bufferRGBA = new unsigned char[size];

    // convert to rgba
    for (int i = 0; i < size; ++i) {
        //unsigned int idx = i * 4;
        bufferRGBA[i] = (rawData[i] > 0) ? 255 : 0; //hack: remove later
        //bufferRGBA[idx + 1] = 0;
        //bufferRGBA[idx + 2] = 0;
        //bufferRGBA[idx + 3] = 255;
    }
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, sizeX, sizeY, sizeZ, 0, GL_RED, GL_UNSIGNED_BYTE, bufferRGBA);

    GL_Error();
    //glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, sizeX, sizeY, sizeZ, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)bufferRGBA);
    delete [] rawData;
    delete [] bufferRGBA;

}

// ----------------------------------------------------------------------------
//
void OpenGLManager::load3DTexturesFromRaw_3(QString path1, QString path2, QString path3, GLuint &texture, GLenum texture_unit, int sizeX, int sizeY, int sizeZ)
{
    unsigned int size = sizeX * sizeY * sizeZ;
    unsigned char *rawData1 = (unsigned char *)m_dataContainer->loadRawFile(path1, size);
    unsigned char *rawData2 = (unsigned char *)m_dataContainer->loadRawFile(path2, size);
    unsigned char *rawData3 = (unsigned char *)m_dataContainer->loadRawFile(path3, size);
    //load data into a 3D texture
    glGenTextures(1, &texture);
    glActiveTexture(texture_unit);
    glBindTexture(GL_TEXTURE_3D, texture);

    // set the texture parameters
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    unsigned int size_3 = size * 3;

    unsigned char *bufferRGBA = new unsigned char[size_3];

    // convert to rgba
    for (int i = 0; i < size; ++i) {
        unsigned int idx = i * 3;
        bufferRGBA[idx] = (rawData1[i] > 0) ? 255 : 0; //hack: remove later
        bufferRGBA[idx + 1] = (rawData2[i] > 0) ? 255 : 0;
        bufferRGBA[idx + 2] = (rawData3[i] > 0) ? 255 : 0;
        //bufferRGBA[idx + 3] = 255;
    }
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    //glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, sizeX, sizeY, sizeZ, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)bufferRGBA);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB8, sizeX, sizeY, sizeZ, 0, GL_RGB, GL_UNSIGNED_BYTE, bufferRGBA);
    //glTexStorage3D(GL_TEXTURE_3D, 0, GL_RGBA, sizeX, sizeY, sizeZ);// , 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)bufferRGBA);
    //glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, sizeX, sizeY, sizeZ, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)bufferRGBA);
    GL_Error();
    delete[] rawData1;
    delete[] rawData2;
    delete[] rawData3;
    delete[] bufferRGBA;

}

// ----------------------------------------------------------------------------
//
bool OpenGLManager::init2DHeatMapShaders()
{
    // init 2D HeatMap
    m_GNeurites.createProgram("2DHeatMap");
    bool res = m_GNeurites.compileShader("2DHeatMap",
                                    ":/shaders/nodes_2DHeatMap_vert.glsl",
                                    ":/shaders/nodes_2DHeatMap_geom.glsl",
                                    ":/shaders/nodes_2DHeatMap_frag.glsl");
    if (res == false)
        return res;
    GL_Error();

    m_GNeurites.vaoCreate("2DHeatMap");
    m_GNeurites.vaoBind("2DHeatMap");
    GL_Error();

    m_GNeurites.vboBind("nodes");

    GL_Error();

    m_GNeurites.useProgram("2DHeatMap");

    initNeuritesVertexAttribPointer();

    m_GNeurites.vboRelease("nodes");

    m_GNeurites.vaoRelease();

    GL_Error();

    m_GNeurites.createProgram("2DHeatMap_Texture");
    res = m_GNeurites.compileShader("2DHeatMap_Texture",
                                    ":/shaders/nodes_2DHeatMap_tex_vert.glsl",
                                    ":/shaders/nodes_2DHeatMap_tex_geom.glsl",
                                    ":/shaders/nodes_2DHeatMap_tex_frag.glsl");
    GL_Error();

    if (res == false)
        return res;

    // todo: initialize uniforms

    m_GNeurites.vaoCreate("2DHeatMap_Quad");
    m_GNeurites.vaoBind("2DHeatMap_Quad");

    m_Texquad.push_back(QVector4D(-1.0f, -1.0f,  0.0f, 0));
    m_Texquad.push_back(QVector4D( 1.0f, -1.0f,  1.0f, 0));
    m_Texquad.push_back(QVector4D(-1.0f,  1.0f,  0.0f, 1));
    m_Texquad.push_back(QVector4D(-1.0f,  1.0f,  0.0f, 1));
    m_Texquad.push_back(QVector4D( 1.0f, -1.0f,  1.0f, 0));
    m_Texquad.push_back(QVector4D( 1.0f,  1.0f,  1.0f, 1));



    // allocate skeleton nodes
    m_GNeurites.vboCreate("quad", Buffer_t::VERTEX, Buffer_USAGE_t::STATIC);
    m_GNeurites.vboBind("quad");
    m_GNeurites.vboAllocate("quad",
                            m_Texquad.data(),
                            m_Texquad.size() * sizeof(QVector4D));

    m_GNeurites.useProgram("2DHeatMap_Texture");
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0,  0);
    GL_Error();

    m_GNeurites.vboRelease("quad");
    m_GNeurites.vaoRelease();



}

// ----------------------------------------------------------------------------
//
void OpenGLManager::render2DHeatMapTexture()
{
    // 1) render this into fbo 1
    // ********* Debug Texture
    if (m_uniforms.x_axis == 100 && m_uniforms.y_axis == 100) {
        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        m_GNeurites.vaoBind("2DHeatMap_Quad");
        m_GNeurites.useProgram("2DHeatMap_Texture");
        // heatmap texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_2D_heatMap_Tex);
        GLint tex = glGetUniformLocation( m_GNeurites.getProgram("2DHeatMap_Texture"), "tex");
        glUniform1i(  tex, 0 );

        // transfer function
        glActiveTexture(GL_TEXTURE1);
        glBindTexture( GL_TEXTURE_1D,  m_tf_2DHeatMap_tex);
        GLint tf = glGetUniformLocation( m_GNeurites.getProgram("2DHeatMap_Texture"), "tf");
        glUniform1i(  tf, 1 );

        GLint dim = glGetUniformLocation(m_GNeurites.getProgram("2DHeatMap_Texture"), "dim");
        float dim_value[2] = {m_canvas_w, m_canvas_h};
        glUniform2fv(dim, 1, dim_value);

        glDrawArrays(GL_TRIANGLES, 0, m_Texquad.size() );
        glEnable(GL_DEPTH_TEST);

        m_GNeurites.vaoRelease();


   }
}

void OpenGLManager::update2DTextureUniforms(GLuint program)
{
    // initialize uniforms
    int m_noRartionMatrix = glGetUniformLocation(program, "m_noRartionMatrix");
    if (m_noRartionMatrix >= 0) glUniformMatrix4fv(m_noRartionMatrix, 1, GL_FALSE, m_uniforms.modelNoRotMatrix);

    int vMatrix = glGetUniformLocation(program, "vMatrix");
    if (vMatrix >= 0) glUniformMatrix4fv(vMatrix, 1, GL_FALSE, m_uniforms.vMatrix);

    int pMatrix = glGetUniformLocation(program, "pMatrix");
    if (pMatrix >= 0) glUniformMatrix4fv(pMatrix, 1, GL_FALSE, m_uniforms.pMatrix);
    GL_Error();
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::drawNodesInto2DTexture()
{
    //****************** Render Nodes Into Texture ***********************
    glEnable(GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_DST_ALPHA);

    glDisable(GL_DEPTH_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, m_2D_heatMap_FBO_H);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_GNeurites.vaoBind("2DHeatMap");
    m_GNeurites.vboBind("nodes");

    m_GNeurites.useProgram("2DHeatMap");

    update2DTextureUniforms( m_GNeurites.getProgram("2DHeatMap") );

    glDrawArrays(GL_POINTS, 0,  m_neurites_nodes.size() );

    m_GNeurites.vboRelease("nodes");
    m_GNeurites.vaoRelease();

    glFlush();
    glFinish();

    glEnable(GL_DEPTH_TEST);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


// ############## Skeleton Graph ###############################################

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

void OpenGLManager::updateSkeletonGraphUniforms(GLuint program)
{
    int m_noRartionMatrix = glGetUniformLocation(program, "m_noRartionMatrix");
    if (m_noRartionMatrix >= 0) glUniformMatrix4fv(m_noRartionMatrix, 1, GL_FALSE, m_uniforms.modelNoRotMatrix);

    int mMatrix = glGetUniformLocation(program, "mMatrix");
    if (mMatrix >= 0) glUniformMatrix4fv(mMatrix, 1, GL_FALSE, m_uniforms.mMatrix);


    int vMatrix = glGetUniformLocation(program, "vMatrix");
    if (vMatrix >= 0) glUniformMatrix4fv(vMatrix, 1, GL_FALSE, m_uniforms.vMatrix);

    int pMatrix = glGetUniformLocation(program, "pMatrix");
    if (pMatrix >= 0) glUniformMatrix4fv(pMatrix, 1, GL_FALSE, m_uniforms.pMatrix);

    int y_axis = glGetUniformLocation(program, "y_axis");
    if (y_axis >= 0) glUniform1iv(y_axis, 1, &m_uniforms.y_axis);

    int x_axis = glGetUniformLocation(program, "x_axis");
    if (x_axis >= 0) glUniform1iv(x_axis, 1, &m_uniforms.x_axis);


    int hoveredID = glGetUniformLocation(program, "hoveredID");
    if (hoveredID >= 0) glUniform1iv(hoveredID, 1, &m_hoveredID);
    GL_Error();
}


void OpenGLManager::updateSkeletonGraphTransitionUniforms(GLuint program)
{
    int m_noRartionMatrix = glGetUniformLocation(program, "m_noRartionMatrix");
    if (m_noRartionMatrix >= 0) glUniformMatrix4fv(m_noRartionMatrix, 1, GL_FALSE, m_uniforms.modelNoRotMatrix);

    int mMatrix = glGetUniformLocation(program, "mMatrix");
    if (mMatrix >= 0) glUniformMatrix4fv(mMatrix, 1, GL_FALSE, m_uniforms.mMatrix);

    int vMatrix = glGetUniformLocation(program, "vMatrix");
    if (vMatrix >= 0) glUniformMatrix4fv(vMatrix, 1, GL_FALSE, m_uniforms.vMatrix);

    int pMatrix = glGetUniformLocation(program, "pMatrix");
    if (pMatrix >= 0) glUniformMatrix4fv(pMatrix, 1, GL_FALSE, m_uniforms.pMatrix);

    int y_axis = glGetUniformLocation(program, "y_axis");
    if (y_axis >= 0) glUniform1iv(y_axis, 1, &m_uniforms.y_axis);

    int x_axis = glGetUniformLocation(program, "x_axis");
    if (x_axis >= 0) glUniform1iv(x_axis, 1, &m_uniforms.x_axis);


    int hoveredID = glGetUniformLocation(program, "hoveredID");
    if (hoveredID >= 0) glUniform1iv(hoveredID, 1, &m_hoveredID);
    GL_Error();
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::drawSkeletonsGraph(bool selection )
{
    // only the edges, the nodes itself they are not needed to be visible
    // this will collabse into a node for the neurites at the most abstract view

    if (m_glFunctionsSet == false)
        return;

    // update skeleton data from object manager

    m_GSkeleton.vboBind("nodes");
    m_GSkeleton.vboAllocate("nodes",
                            m_abstract_skel_nodes.data(),
                            m_abstract_skel_nodes.size() * sizeof(struct AbstractSkelNode) );
    m_GSkeleton.vboRelease("nodes");

    if (selection) {
        m_GSkeleton.vaoBind("Selection");
        m_GSkeleton.useProgram("selection_2D_nodes");
        updateSkeletonGraphUniforms(m_GSkeleton.getProgram("selection_2D_nodes"));
        glDrawArrays(GL_POINTS, 0,  m_abstract_skel_nodes.size() );

        m_GSkeleton.useProgram("selection_23D_nodes");
        updateSkeletonGraphUniforms(m_GSkeleton.getProgram("selection_23D_nodes"));
        glDrawArrays(GL_POINTS, 0,  m_abstract_skel_nodes.size() );

        m_GSkeleton.vboBind("index");

        m_GSkeleton.useProgram("selection_2D_index");
        updateSkeletonGraphUniforms(m_GSkeleton.getProgram("selection_2D_index"));
        glDrawElements(GL_LINES, m_abstract_skel_edges.size(), GL_UNSIGNED_INT, 0 );

        m_GSkeleton.useProgram("selection_23D_index");
        updateSkeletonGraphTransitionUniforms(m_GSkeleton.getProgram("selection_23D_index"));
        glDrawElements(GL_LINES, m_abstract_skel_edges.size(), GL_UNSIGNED_INT, 0 );

        m_GSkeleton.vboRelease("index");
        m_GSkeleton.vaoRelease();
    } else {
        m_GSkeleton.vaoBind("SkeletonGraph");
        m_GSkeleton.useProgram("2D_nodes");
        updateSkeletonGraphUniforms( m_GSkeleton.getProgram("2D_nodes") );
        glDrawArrays(GL_POINTS, 0,  m_abstract_skel_nodes.size() );

        m_GSkeleton.useProgram("23D_nodes");
        updateSkeletonGraphTransitionUniforms( m_GSkeleton.getProgram("23D_nodes") );
        glDrawArrays(GL_POINTS, 0,  m_abstract_skel_nodes.size() );

        m_GSkeleton.vboBind("index");

        m_GSkeleton.useProgram("2D_index");
        updateSkeletonGraphUniforms( m_GSkeleton.getProgram("2D_index"));
        glDrawElements(GL_LINES, m_abstract_skel_edges.size(), GL_UNSIGNED_INT, 0 );

        m_GSkeleton.useProgram("23D_index");
        updateSkeletonGraphUniforms( m_GSkeleton.getProgram("23D_index"));
        glDrawElements(GL_LINES, m_abstract_skel_edges.size(), GL_UNSIGNED_INT, 0 );
        m_GSkeleton.vboRelease("index");
        m_GSkeleton.vaoRelease();
    }
}

// ############## Neurites Graph ###############################################
//
void OpenGLManager::initNeuritesVertexAttribPointer()
{
    int offset = 0;
    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, 1, GL_INT, 0, (void*)offset);

}

// ----------------------------------------------------------------------------
//
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

    updateNeuritesGraphUniforms( m_GNeurites.getProgram("index") );

    GL_Error();

    m_GNeurites.vboRelease("index");
    m_GNeurites.vaoRelease();


    init2DHeatMapShaders();

    return true;
}

void OpenGLManager::updateNeuritesGraphUniforms(GLuint program)
{
    int m_noRartionMatrix = glGetUniformLocation(program, "m_noRartionMatrix");
    if (m_noRartionMatrix >= 0) glUniformMatrix4fv(m_noRartionMatrix, 1, GL_FALSE, m_uniforms.modelNoRotMatrix);

    int vMatrix = glGetUniformLocation(program, "vMatrix");
    if (vMatrix >= 0) glUniformMatrix4fv(vMatrix, 1, GL_FALSE, m_uniforms.vMatrix);

    int pMatrix = glGetUniformLocation(program, "pMatrix");
    if (pMatrix >= 0) glUniformMatrix4fv(pMatrix, 1, GL_FALSE, m_uniforms.pMatrix);

    int y_axis = glGetUniformLocation(program, "y_axis");
    if (y_axis >= 0) glUniform1iv(y_axis, 1, &m_uniforms.y_axis);

    int x_axis = glGetUniformLocation(program, "x_axis");
    if (x_axis >= 0) glUniform1iv(x_axis, 1, &m_uniforms.x_axis);


    int hoveredID = glGetUniformLocation(program, "hoveredID");
    if (hoveredID >= 0) glUniform1iv(hoveredID, 1, &m_hoveredID);
    GL_Error();
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::drawNeuritesGraph()
{
    // only the edges, because the skeleton itself will collabse into a node

    if (m_glFunctionsSet == false)
        return;

    m_GNeurites.vaoBind("ConnectivityGraph");
    m_GNeurites.vboBind("nodes");

    m_GNeurites.vboBind("index");

    m_GNeurites.useProgram("index");
    updateNeuritesGraphUniforms( m_GNeurites.getProgram("index") );
    glDrawElements(GL_LINES,  m_neurites_edges.size(), GL_UNSIGNED_INT, 0 );

    m_GNeurites.vboRelease("index");

    m_GNeurites.vboRelease("nodes");
    m_GNeurites.vaoRelease();
}

// ############## Mesh ###############################################
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
    GLuint mesh_program = m_TMesh.getProgram("3Dtriangles");

    GLint splat_tex = glGetUniformLocation(mesh_program, "splat_tex");
    if (splat_tex >= 0) glUniform1i(splat_tex, 2);

    GLint gly_tex = glGetUniformLocation(mesh_program, "gly_tex");
    if (gly_tex >= 0) glUniform1i(gly_tex, 3);

    //GLint mito_tex = glGetUniformLocation(mesh_program, "mito_tex");
    //glUniform1i(mito_tex, 4);

    //GLint nmito_tex = glGetUniformLocation(mesh_program, "nmito_tex");
    //glUniform1i(nmito_tex, 5);

    m_TMesh.vboCreate("MeshVertices", Buffer_t::VERTEX, Buffer_USAGE_t::STATIC);
    m_TMesh.vboBind("MeshVertices");

    Mesh* mesh = m_dataContainer->getMeshPointer();
    mesh->allocateVerticesVBO( m_TMesh.getVBO("MeshVertices") );
    initMeshVertexAttrib();

    GL_Error();

    // create normals vbo
    if (m_normals_enabled) {
        m_TMesh.vboCreate("VertexNormals", Buffer_t::VERTEX, Buffer_USAGE_t::STATIC);
        m_TMesh.vboBind("VertexNormals");
        // allocate
        mesh->allocateNormalsVBO( m_TMesh.getVBO("VertexNormals") );

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0,  0);

        m_TMesh.vboBind("VertexNormals");
        GL_Error();
    }

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
    GL_Error();


    m_TMesh.vaoRelease();

    GL_Error();
    /*  end selection buffer **/

}
// ----------------------------------------------------------------------------
//
void OpenGLManager::updateMeshPrograms(GLuint program)
{
    int mMatrix = glGetUniformLocation(program, "mMatrix");
    if (mMatrix >= 0) glUniformMatrix4fv(mMatrix, 1, GL_FALSE, m_uniforms.mMatrix);

    int vMatrix = glGetUniformLocation(program, "vMatrix");
    if (vMatrix >= 0) glUniformMatrix4fv(vMatrix, 1, GL_FALSE, m_uniforms.vMatrix);

    int pMatrix = glGetUniformLocation(program, "pMatrix");
    if (pMatrix >= 0) glUniformMatrix4fv(pMatrix, 1, GL_FALSE, m_uniforms.pMatrix);

    int y_axis = glGetUniformLocation(program, "y_axis");
    if (y_axis >= 0) glUniform1iv(y_axis, 1, &m_uniforms.y_axis);

    int x_axis = glGetUniformLocation(program, "x_axis");
    if (x_axis >= 0) glUniform1iv(x_axis, 1, &m_uniforms.x_axis);

    int hoveredID = glGetUniformLocation(program, "hoveredID");
    if (hoveredID >= 0) glUniform1iv(hoveredID, 1, &m_hoveredID);

	int splat_flags = glGetUniformLocation(program, "splat_flags");
	if (splat_flags >= 0) glUniform4i(splat_flags,	(m_is_astro_splat) ? 1 : 0, 
													(m_is_glyco_splat) ? 1 : 0, 
													(m_is_nmito_splat) ? 1 : 0, 
													(m_is_amito_splat) ? 1 : 0);

	int specular_flag = glGetUniformLocation(program, "specular_flag");
	if (specular_flag >= 0) glUniform1i(specular_flag, (m_is_specular) ? 1 : 0);

    GL_Error();
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::drawMeshTriangles(bool selection )
{
   if (selection) {
       m_TMesh.vaoBind("Selection");
       m_TMesh.useProgram("selection");

      updateMeshPrograms( m_TMesh.getProgram("selection") );

       m_TMesh.vboBind("MeshIndices");

       glDrawElements(GL_TRIANGLES,  m_dataContainer->getMeshIndicesSize(),  GL_UNSIGNED_INT, 0 );
       m_TMesh.vboRelease("MeshIndices");
       m_TMesh.vaoRelease();
   } else {
       m_TMesh.vaoBind("Mesh");
       m_TMesh.useProgram("3Dtriangles");

       updateMeshPrograms( m_TMesh.getProgram("3Dtriangles") );

       // transfer function
       //glActiveTexture(GL_TEXTURE1);
       //glBindTexture( GL_TEXTURE_1D,  m_tf_2DHeatMap_tex);
       //GLint tf = glGetUniformLocation( m_TMesh.getProgram("3Dtriangles"), "tf");
       //glUniform1i(  tf, 1 );


       // Astrocyte 3D volume

       GLint splat_tex = glGetUniformLocation(m_TMesh.getProgram("3Dtriangles"), "splat_tex");

       if (splat_tex >= 0) {
           glUniform1i(splat_tex, 2);
           glActiveTexture(GL_TEXTURE2);
           glBindTexture(GL_TEXTURE_3D, m_splat_volume_3DTex);// m_astro_3DTex);
       }
       // Glycogen 3D volume

       GLint gly_tex = glGetUniformLocation( m_TMesh.getProgram("3Dtriangles"), "gly_tex");
       if (gly_tex >= 0) {
           glUniform1i(  gly_tex, 3 );
           glActiveTexture(GL_TEXTURE3);
           glBindTexture(GL_TEXTURE_3D, m_glycogen_3DTex);
        }

       // Astrocyte 3D volume

       //GLint mito_tex = glGetUniformLocation( m_TMesh.getProgram("3Dtriangles"), "mito_tex");
       //glUniform1i(  mito_tex, 4 );
       //glActiveTexture(GL_TEXTURE4);
       //glBindTexture(GL_TEXTURE_3D, m_mito_3DTex);

       //glActiveTexture(GL_TEXTURE5);
       //glBindTexture(GL_TEXTURE_3D, m_nmito_3DTex);

       m_TMesh.vboBind("MeshIndices");
       glDrawElements(GL_TRIANGLES,  m_dataContainer->getMeshIndicesSize(),  GL_UNSIGNED_INT, 0 );
       m_TMesh.vboRelease("MeshIndices");

       m_TMesh.vaoRelease();
   }
}

// ############## Skeleton Points ###############################################
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

void OpenGLManager::updateSkeletonUniforms(GLuint program)
{

    // initialize uniforms
    int mMatrix = glGetUniformLocation(program, "mMatrix");
    if (mMatrix >= 0) glUniformMatrix4fv(mMatrix, 1, GL_FALSE, m_uniforms.mMatrix);

    int vMatrix = glGetUniformLocation(program, "vMatrix");
    if (vMatrix >= 0) glUniformMatrix4fv(vMatrix, 1, GL_FALSE, m_uniforms.vMatrix);

    int pMatrix = glGetUniformLocation(program, "pMatrix");
    if (pMatrix >= 0) glUniformMatrix4fv(pMatrix, 1, GL_FALSE, m_uniforms.pMatrix);

    int y_axis = glGetUniformLocation(program, "y_axis");
    if (y_axis >= 0) glUniform1iv(y_axis, 1, &m_uniforms.y_axis);

    int x_axis = glGetUniformLocation(program, "x_axis");
    if (x_axis >= 0) glUniform1iv(x_axis, 1, &m_uniforms.x_axis);

    int hoveredID = glGetUniformLocation(program, "hoveredID");
    if (hoveredID >= 0) glUniform1iv(hoveredID, 1, &m_hoveredID);
    GL_Error();
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::drawSkeletonPoints(bool selection)
{
   // qDebug() << "OpenGLManager::drawSkeletonPoints";
    // I need this because vertex <-> skeleton mapping is not complete
    if (selection) {
        m_SkeletonPoints.vaoBind("Selection");
        m_SkeletonPoints.useProgram("selection");
        updateSkeletonUniforms( m_SkeletonPoints.getProgram("selection") );
        glDrawArrays(GL_POINTS, 0,  m_dataContainer->getSkeletonPointsSize()  );
        m_SkeletonPoints.vaoRelease();
    } else {
        m_SkeletonPoints.vaoBind("SkeletonPoints");
        m_SkeletonPoints.useProgram("3DPoints");
        updateSkeletonUniforms( m_SkeletonPoints.getProgram("3DPoints") );
        glDrawArrays(GL_POINTS, 0,  m_dataContainer->getSkeletonPointsSize()  );
        m_SkeletonPoints.vaoRelease();
    }
}

// ############## Glycogen Points ###############################################
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

void OpenGLManager::updateGlycogenUniforms(GLuint program)
{
     // initialize uniforms
     int mMatrix = glGetUniformLocation(program, "mMatrix");
     if (mMatrix >= 0) glUniformMatrix4fv(mMatrix, 1, GL_FALSE, m_uniforms.mMatrix);

     int m_noRartionMatrix = glGetUniformLocation(program, "m_noRartionMatrix");
     if (m_noRartionMatrix >= 0) glUniformMatrix4fv(m_noRartionMatrix, 1, GL_FALSE, m_uniforms.modelNoRotMatrix);

     int vMatrix = glGetUniformLocation(program, "vMatrix");
     if (vMatrix >= 0) glUniformMatrix4fv(vMatrix, 1, GL_FALSE, m_uniforms.vMatrix);

     int pMatrix = glGetUniformLocation(program, "pMatrix");
     if (pMatrix >= 0) glUniformMatrix4fv(pMatrix, 1, GL_FALSE, m_uniforms.pMatrix);
     GL_Error();

}

// ----------------------------------------------------------------------------
//
void OpenGLManager::drawGlycogenPoints()
{
    // I need this because transitioning from mesh to skeleton is not smooth
    m_vao_glycogen.bind();

    m_GlycogenPoints.useProgram("3DPoints");

    updateGlycogenUniforms(m_GlycogenPoints.getProgram("3DPoints"));
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
void OpenGLManager::renderAbstractions()
{
    struct ast_neu_properties space_properties = m_2dspace->getSpaceProper();

    // ********* Render Abstraction

    if (m_renderGlycogenGranules)
        drawGlycogenPoints();


    if ( (space_properties.ast.render_type.x() == 1 &&  space_properties.neu.render_type.x() == 1) ) {
        glDisable (GL_BLEND);
        glBlendFunc (GL_ONE, GL_ONE);
        drawSkeletonPoints(false);
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    if ( (space_properties.ast.render_type.y() == 1 &&  space_properties.ast.render_type.x() == 0) || space_properties.neu.render_type.y() == 1 )
        drawSkeletonPoints(false); // transparency is allowed

    if ( space_properties.ast.render_type.z() == 1 ||  space_properties.neu.render_type.z() == 1)
        drawSkeletonsGraph(false);

    if ( space_properties.ast.render_type.w() == 1 ||  space_properties.neu.render_type.w() == 1) {
        glDisable(GL_DEPTH_TEST);
        drawSkeletonsGraph(false);
        drawNeuritesGraph();
        glEnable(GL_DEPTH_TEST);
    }

    if ( space_properties.ast.render_type.x() == 1 ||  space_properties.neu.render_type.x() == 1)
        drawMeshTriangles(false);
}



// ############## Selection ###############################################
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

    GLuint depthBuffer;
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_canvas_w, m_canvas_h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);


    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// ----------------------------------------------------------------------------
//
int OpenGLManager::processSelection(float x, float y)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_selectionFrameBuffer);
    glEnable(GL_DEPTH_TEST);

    GLubyte pixel[3];

    glReadPixels(x, y, 1, 1, GL_RGB,GL_UNSIGNED_BYTE, (void *)pixel);
    int pickedID = pixel[0] + pixel[1] * 256 + pixel[2] * 65536;
    qDebug() << pixel[0] << " " << pixel[1] << " " << pixel[2];

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    qDebug() << "Picked ID: " << pickedID;

    return pickedID;
 }

// ----------------------------------------------------------------------------
//
void OpenGLManager::renderSelection()
{
    // ********* Render Selection

    glBindFramebuffer(GL_FRAMEBUFFER, m_selectionFrameBuffer);
    //clear
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //disable dithering -- important
    glDisable(GL_DITHER);
    glDisable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glDisable (GL_BLEND);

    //render graph
    drawMeshTriangles(true);
    drawSkeletonPoints(true);
    drawSkeletonsGraph(true);

    //enable dithering again
    glEnable(GL_DITHER);
    glEnable (GL_BLEND);
    glEnable(GL_MULTISAMPLE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::update2Dflag(bool is2D)
{
    m_2D = is2D;
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::setZoom(float zoom)
{
    m_zoom = zoom;
}

// ############## Filtering #########################
//
Object_t OpenGLManager::getObjectTypeByID(int hvgxID)
{
    if (m_dataContainer == NULL)
        return Object_t::UNKNOWN;

    return m_dataContainer->getObjectTypeByID(hvgxID);
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::recursiveFilter(int hvgxID, bool isfilterd)
{
    std::map<int, Object*> *objectMap = m_dataContainer->getObjectsMapPtr();
    if (objectMap->find(hvgxID) == objectMap->end() || objectMap->at(hvgxID) == NULL) {
        return;
    }

    FilterObject(hvgxID, isfilterd);

    if (isfilterd) // hide
        return;

    Object *obj = objectMap->at(hvgxID);

    // if it has a children then get them
    // else if has parent get them
    if (m_display_parent) {
        int parentID = obj->getParentID();
        if (objectMap->find(parentID) == objectMap->end()) {
            qDebug() << obj->getName().data() << " has no parnt";
        } else {
            FilterObject(parentID, isfilterd);
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
                && objectMap->find(synapse_data.axon) != objectMap->end()) {
                FilterObject(synapse_data.axon, isfilterd);
            }

            if (synapse_data.dendrite != hvgxID
                && synapse_data.dendrite
                && objectMap->find(synapse_data.dendrite) != objectMap->end()) {
                FilterObject(synapse_data.dendrite, isfilterd);
            }

            if (synapse_data.spine != hvgxID
                && synapse_data.spine
                && objectMap->find(synapse_data.spine) != objectMap->end()) {
                FilterObject(synapse_data.spine, isfilterd);
            }

            if (synapse_data.bouton != hvgxID
                && synapse_data.bouton
                && objectMap->find(synapse_data.bouton) != objectMap->end()) {
                FilterObject(synapse_data.bouton, isfilterd);
            }
        }
    }

}

// ----------------------------------------------------------------------------
//
void OpenGLManager::FilterObject(int ID, bool isfilterd)
{
    // if we filter the object we need to recompute the maximum astrocyte coverage and volume

    std::map<int, Object*> *objectMap = m_dataContainer->getObjectsMapPtr();
    if ( ID > m_ssbo_data.size() ||
         objectMap->find(ID) == objectMap->end() ||
         objectMap->at(ID) == NULL )
    {
        // no need to filter something that doesnt exist
        return;
    }

    Object *obj = objectMap->at(ID);
    obj->updateFilteredFlag(isfilterd);

    int visibility = m_ssbo_data[ID].info.w();

    if (isfilterd) {
        visibility |= 1 << 0;
    } else {
        visibility &= ~(1 << 0);
    }

    m_ssbo_data[ID].info.setW(visibility);

}

// ----------------------------------------------------------------------------
//
void OpenGLManager::FilterByType(Object_t type, bool flag)
{
    // this should be fast, I should somehow group by IDs for types, and when we filter by type I just get all IDs with this type and set them to 1 which means filtered
    std::vector<Object*> objects_list = m_dataContainer->getObjectsByType(type);
    qDebug() << "Filtering " << objects_list.size() << " objects";

    for (int i = 0; i < objects_list.size(); i++) {
        int hvgxID = objects_list[i]->getHVGXID();
        FilterObject(hvgxID, flag);

    }
    m_dataContainer->recomputeMaxValues();
    updateSSBO();
}


// ----------------------------------------------------------------------------
//
void OpenGLManager::FilterByID( QList<QString> tokens_Ids, bool invisibility )
{
    if (invisibility == false) { // show only
        // hide everything else
        for (int i = 0; i < m_ssbo_data.size(); ++i) {
            FilterObject(i, !invisibility);
        }
    }

    for (int i = 0; i < tokens_Ids.size(); ++i) {
        int hvgxID = tokens_Ids[i].toInt();
        if (hvgxID > m_ssbo_data.size())
            continue;

        recursiveFilter(hvgxID, invisibility);
    }

    m_dataContainer->recomputeMaxValues();
    updateSSBO();
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::FilterByID( std::set<int> tokens_Ids, bool invisibility )
{
    if (invisibility == false) { // show only
          // hide everything else
          for (int i = 0; i < m_ssbo_data.size(); ++i) {
              FilterObject(i, !invisibility);
          }
      }

    for (auto iter = tokens_Ids.begin(); iter != tokens_Ids.end(); ++iter) {
        int hvgxID = *iter;
        if (hvgxID > m_ssbo_data.size())
            continue;

        recursiveFilter(hvgxID, invisibility);

    }

    m_dataContainer->recomputeMaxValues();
    updateSSBO();
}


// ----------------------------------------------------------------------------
//
void OpenGLManager::VisibilityToggleSelectedObjects( std::set<int> tokens_Ids, bool invisibility )
{
    for (auto iter = tokens_Ids.begin(); iter != tokens_Ids.end(); ++iter) {
        int hvgxID = *iter;
        if (hvgxID > m_ssbo_data.size())
            continue;

        recursiveFilter(hvgxID, invisibility);

    }

    m_dataContainer->recomputeMaxValues();
    updateSSBO();}


// ----------------------------------------------------------------------------
//
void OpenGLManager::showAll()
{
    for (int i = 0; i < m_ssbo_data.size(); ++i) {
        // recompute max vol and astro coverage
        FilterObject(i, false);
    }

    m_dataContainer->recomputeMaxValues();
    updateSSBO();
}

// ############## Update VBOS and SSBO for Layouting #########################
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
void OpenGLManager::update_skeleton_layout1(QVector2D layout1,  long node_index, int hvgxID)
{
    // Graph_t::ALL_SKELETONS, dont discriminate between types
    // get the object -> get its skeleton -> update the layout
    std::map<int, Object*> *objects_map = m_dataContainer->getObjectsMapPtr();

    if (objects_map->find(hvgxID) == objects_map->end()) {
        return;
    }

    Skeleton *skel = objects_map->at(hvgxID)->getSkeleton();
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
    std::map<int, Object*> *objects_map = m_dataContainer->getObjectsMapPtr();
    if (objects_map->find(hvgxID) == objects_map->end()) {
        qDebug() << "Object not found";
        return;
    }

    Skeleton *skel = objects_map->at(hvgxID)->getSkeleton();
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
    std::map<int, Object*> *objects_map = m_dataContainer->getObjectsMapPtr();
    if (objects_map->find(hvgxID) == objects_map->end()) {
        return;
    }

    Skeleton *skel = objects_map->at(hvgxID)->getSkeleton();
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


    std::map<int, Object*> *objectMap = m_dataContainer->getObjectsMapPtr();
    for ( auto iter = objectMap->begin(); iter != objectMap->end(); iter++ ) {
        Object *obj = (*iter).second;

        int hvgxID = obj->getHVGXID();
        if (m_ssbo_data.size() <= hvgxID)
            continue;

        QVector3D center = rotationMatrix * m_ssbo_data[hvgxID].center.toVector3D();

        m_ssbo_data[hvgxID].layout1 = center.toVector2D();
        m_ssbo_data[hvgxID].layout2 = center.toVector2D();
    }
}

// ----------------------------------------------------------------------------
//
float translate(float value, float leftMin, float leftMax, float rightMin, float rightMax)
{
    // if value < leftMin -> value = leftMin
    value = std::max(value, leftMin);
    // if value > leftMax -> value = leftMax
    value = std::min(value, leftMax);
    // Figure out how 'wide' each range is
    float leftSpan = leftMax - leftMin;
    float rightSpan = rightMax - rightMin;

    // Convert the left range into a 0-1 range (float)
    float valueScaled = float(value - leftMin) / float(leftSpan);

    // Convert the 0-1 range into a value in the right range.
    return rightMin + (valueScaled * rightSpan);
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::updateSSBO()
{
    std::map<int, Object*> *objectMap = m_dataContainer->getObjectsMapPtr();

	float glycogen_tf[15] = { 
		254.0 / 255.0, 235.0 / 255.0, 226.0 / 255.0, 
		251.0 / 255.0, 180.0 / 255.0, 185.0 / 255.0,
		247.0 / 255.0, 104.0 / 255.0, 161.0 / 255.0,
		197.0 / 255.0, 27.0 / 255.0, 138.0 / 255.0,
		122.0 / 255.0, 1.0 / 255.0, 119.0 / 255.0
	};

    for ( auto iter = objectMap->begin(); iter != objectMap->end(); iter++ ) {
        Object *obj = (*iter).second;
        if (obj->isFiltered() || obj->getObjectType() == Object_t::ASTROCYTE)
            continue;

        int hvgxID = obj->getHVGXID();
        if (m_ssbo_data.size() <= hvgxID)
            continue;


        float coverage = translate( obj->getAstroCoverage(),
                                   0,  m_dataContainer->getMaxAstroCoverage() ,
                                   0, 0.7);


        m_ssbo_data[hvgxID].info.setY( obj->getAstroCoverage() / m_dataContainer->getMaxAstroCoverage()  );

        switch(m_size_encoding) {
            case Size_e::VOLUME: {
                float volume =  translate(obj->getVolume(), 0,
                                          m_dataContainer->getMaxVolume(), 0, 1);
                m_ssbo_data[hvgxID].info.setX( 20 *  volume);
                break;
            }
            case Size_e::ASTRO_COVERAGE:
                m_ssbo_data[hvgxID].info.setX( 20 *  coverage);
                break;
            case Size_e::SYNAPSE_SIZE: {
                float synapse_volume =  translate(obj->getSynapseSize(), 0,
                                                  m_dataContainer->getMaxSynapseVolume(), 0, 1);
                m_ssbo_data[hvgxID].info.setX( 20 * synapse_volume);
                break;
            }
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
              QVector4D color = obj->getColor();
              m_ssbo_data[hvgxID].color = QVector4D(color.x() + 0.3, color.y() +  0.3, color.z() + 0.3, color.w()) ;
              QVector4D add_color = QVector4D(1, 1, 1, 0) * coverage;
              m_ssbo_data[hvgxID].color -= add_color;

              break;
            }
            case Color_e::FUNCTION:
            {
              int function = obj->getFunction();
              if (function == 0) {
                  m_ssbo_data[hvgxID].color.setX(0.7);
                  m_ssbo_data[hvgxID].color.setY(0.1);
                  m_ssbo_data[hvgxID].color.setZ(0.1);
              } else if (function == 1) {
                  m_ssbo_data[hvgxID].color.setX(0.1);
                  m_ssbo_data[hvgxID].color.setY(0.1);
                  m_ssbo_data[hvgxID].color.setZ(0.7);
              } else {
                  m_ssbo_data[hvgxID].color.setX(0.5);
                  m_ssbo_data[hvgxID].color.setY(0.5);
                  m_ssbo_data[hvgxID].color.setZ(0.5);

              }

              break;
            }
			case Color_e::GLYCOGEN_MAPPING:
				
				if (obj->getObjectType() == Object_t::AXON || obj->getObjectType() == Object_t::DENDRITE)
				{
					m_ssbo_data[hvgxID].color.setX(0.7);
					m_ssbo_data[hvgxID].color.setY(0.7);
					m_ssbo_data[hvgxID].color.setZ(0.7);
				}
				else if (obj->getObjectType() == Object_t::SPINE || obj->getObjectType() == Object_t::BOUTON)
				{
					//TODO: Ugly - change later
					float mappedValue = obj->getMappedValue();
					float r = 0.0; float r1 = 0.0; float r2 = 0.0;
					float g = 0.0; float g1 = 0.0; float g2 = 0.0;
					float b = 0.0; float b1 = 0.0; float b2 = 0.0;
					float mix = 0.0;
					if (mappedValue > 0.75)
					{
						mix = (mappedValue - 0.75) / 0.25;
						r1 = glycogen_tf[4 * 3];
						g1 = glycogen_tf[4 * 3 + 1];
						b1 = glycogen_tf[4 * 3 + 2];
						r2 = glycogen_tf[3 * 3];
						g2 = glycogen_tf[3 * 3 + 1];
						b2 = glycogen_tf[3 * 3 + 2];
						r = r1 * mix + r2 * (1 - mix);
						g = g1 * mix + g2 * (1 - mix);
						b = b1 * mix + b2 * (1 - mix);
					}
					else if (mappedValue > 0.5)
					{
						mix = (mappedValue - 0.5) / 0.25;
						r1 = glycogen_tf[3 * 3];
						g1 = glycogen_tf[3 * 3 + 1];
						b1 = glycogen_tf[3 * 3 + 2];
						r2 = glycogen_tf[2 * 3];
						g2 = glycogen_tf[2 * 3 + 1];
						b2 = glycogen_tf[2 * 3 + 2];
						r = r1 * mix + r2 * (1 - mix);
						g = g1 * mix + g2 * (1 - mix);
						b = b1 * mix + b2 * (1 - mix);
					}
					else if (mappedValue > 0.25)
					{
						mix = (mappedValue - 0.25) / 0.25;
						r1 = glycogen_tf[2 * 3];
						g1 = glycogen_tf[2 * 3 + 1];
						b1 = glycogen_tf[2 * 3 + 2];
						r2 = glycogen_tf[1 * 3];
						g2 = glycogen_tf[1 * 3 + 1];
						b2 = glycogen_tf[1 * 3 + 2];
						r = r1 * mix + r2 * (1 - mix);
						g = g1 * mix + g2 * (1 - mix);
						b = b1 * mix + b2 * (1 - mix);
					}
					else
					{
						mix = mappedValue / 0.25;
						r1 = glycogen_tf[1 * 3];
						g1 = glycogen_tf[1 * 3 + 1];
						b1 = glycogen_tf[1 * 3 + 2];
						r2 = glycogen_tf[0 * 3];
						g2 = glycogen_tf[0 * 3 + 1];
						b2 = glycogen_tf[0 * 3 + 2];
						r = r1 * mix + r2 * (1 - mix);
						g = g1 * mix + g2 * (1 - mix);
						b = b1 * mix + b2 * (1 - mix);
					}
					m_ssbo_data[hvgxID].color.setX(r);
					m_ssbo_data[hvgxID].color.setY(g);
					m_ssbo_data[hvgxID].color.setZ(b);
				}
				break;
        }
    }
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::updateNodeSizeEncoding(Size_e encoding)
{
    if (m_size_encoding == encoding)
        return;

    m_size_encoding = encoding;

    updateSSBO();
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::updateColorEncoding(Color_e encoding)
{
    if (m_color_encoding == encoding)
        return;

    m_color_encoding = encoding;

    updateSSBO();
}

// ----------------------------------------------------------------------------
//
void OpenGLManager::highlightObject(int hvgxID)
{
    if (m_ssbo_data.size() <= hvgxID) {
        m_hoveredID = -1;
        return;
    }

    m_hoveredID = hvgxID;
}
