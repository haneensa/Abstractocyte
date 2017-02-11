#include <chrono>
#include "mesh.h"

Mesh::Mesh()
    :  m_bindIdx(2),
       m_glFunctionsSet(false),
       m_vbo_mesh( QOpenGLBuffer::VertexBuffer ),
       m_vbo_IndexMesh(QOpenGLBuffer::IndexBuffer),
       m_vbo_skeleton( QOpenGLBuffer::VertexBuffer )
{
    m_vertices_size = 0;
    m_indices_size = 0;
    m_skeleton_nodes_size = 0;
    m_limit = 43;
    m_vertex_offset = 0;

    // to do: combine all these files in one .obj file -> m3_dataset.obj
    // to do: interface to load these files
    QString path;
     //loadSkeletonPoints(path); // 11638884, 19131720
    path = "://data/input_data/mouse03_astro_mesh_center_skeleton.obj.bin";
    loadDataset(path);
    path = "://data/input_data/mouse03_neurite_center_skeleton_bleeding.obj.bin";
    loadDataset(path);
}

Mesh::~Mesh()
{
    qDebug() << "~Mesh()";
    if (m_glFunctionsSet) {
        qDebug() << "~Mesh()";
        for (std::size_t i = 0; i != m_objects.size(); i++) {
            delete m_objects[i];
        }

        glDeleteProgram(m_program_skeleton);
        glDeleteProgram(m_program_mesh);

        m_vao_mesh.destroy();
        m_vbo_mesh.destroy();
    }
}

/*
o astrocyte
p x y z # astrocyte center point
b branch_2_1_3
sv x y z # skeleton vertex
sv x y z
.....
l sv_id1 sv_id2  # line segments between skeleton vertices in branch
l sv_id1 sv_id2
.....
v x y z sv_index # mesh vertices and nearest skeleton vertex to it
v x y z sv_index
v x y z sv_index
v x y z sv_index
...
f v1 v2 v3
f v1 v2 v3
....

# Above Would be Repeated for all objects.

# connectivity edges between neurites
c connecitity_graph1
l p1_idx p2_idx
l p1_idx p2_idx

c connecitity_graph2
l p1_idx p2_idx
l p1_idx p2_idx

  */
bool Mesh::loadDataset(QString path)
{
    // open file
    // if 'o' then create new object and set its name and hvgx ID
    // if 'p' then add to obj its center
    // if vl: then add object volume
    // if 'sv' then add to the skeleton data in obj
    // if 'l' then add edges information which connects skeleton points
    // if v and f construct the mesh vertices

    // if 'c connecitity_graph1' then add these info to the points IDs for neurite neurite connictivity 2D mode which astrocyte disappears
    // if 'c connecitity_graph2' then add skeleton-neurite connecivity points which would be shown only when? in the 2D mode
    qDebug() << "Func: loadDataset";
    auto t1 = std::chrono::high_resolution_clock::now();

    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "Could not open the file for reading";
        return false;
    }

    QTextStream in(&file);
    QList<QByteArray> wordList;

    QList<QByteArray> bleeding_list;

    int flag_prev = 0;

    Object *obj = NULL;
    std::string name;
    QVector4D center = QVector4D(0.0, 0.0, 0.0, 0.0);
    GLint idx = -1;
    int vertex_offset = 0; // offset of the last vertex count from previus obj file
    // ssbo buffer info for mesh data
    // [color, center, volume, type, ?]
    struct ssbo_mesh ssbo_object_data;

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        wordList = line.split(' ');

        // load the connectiivity edges first here
        // the nodes are the drawn nodes, and the edges are their edges if both ends present
        // for each object, add it to the set of object nodes

        if (wordList[0] == "o") {
            // o objname_hvgxID

            if (flag_prev == 1) {
                m_buffer_data.push_back(ssbo_object_data);
                m_objects.push_back(obj);
            }

            if (m_objects.size() > m_limit) {
                flag_prev = 0;
                qDebug() << "* Reached size limit.";
                break;
            }

            QByteArray nameline  = wordList[1].data();
            QList<QByteArray> nameList = nameline.split('_');
            name.clear();
            for (int i = 0; i < nameList.size() - 1; ++i)
                name += nameList[i].data();

            int hvgxID = atoi(nameList[nameList.size() - 1].data());
            idx = m_objects.size();
            obj = new Object(name, idx, hvgxID);
            // get objet color based on type
            QVector4D color = obj->getColor();
            ssbo_object_data.color = color;
            flag_prev = 1;
        } else if (wordList[0] == "p") {
            // p centerX centerY centerZ
            if (wordList.size() < 4) {
                qDebug() << "p word list";
                break;
            }
            float x = atof(wordList[1].data());
            float y = atof(wordList[2].data());
            float z = atof(wordList[3].data());
            if (obj->getObjectType() == Object_t::ASTROCYTE ) {
                center = QVector4D(x, y, z, 0);
            } else {
                center = QVector4D(x, y, z, 1);
            }

            obj->setCenter(center);
            ssbo_object_data.center = center;
        } else if (wordList[0] == "vl") { // to do: update the mesh file to remove extra space
            if (wordList.size() < 2) {
                qDebug() << "vl word list";
                break;
            }
            int volume = atoi(wordList[1].data());
            obj->setVolume(volume);
            ssbo_object_data.info.setX(volume);
        } else if (wordList[0]  == "v") {
            if (wordList.size() < 4) {
                qDebug() << "v word list";
                break;
            }

            ++vertex_offset;

            float x1 = atof(wordList[1].data());
            float y1 = atof(wordList[2].data());
            float z1 = atof(wordList[3].data());
            QVector3D mesh_vertex(x1, y1, z1);
            struct VertexData v;
            v.ID = idx; // this used to index ssbo
            v.mesh_vertex = mesh_vertex;
            v.bleed = 0;
            if (wordList.size() < 6) {
                // place holder
                v.skeleton_vertex = mesh_vertex;
            } else {
                float x2 = atof(wordList[4].data());
                float y2 = atof(wordList[5].data());
                float z2 = atof(wordList[6].data());
                QVector3D skeleton_vertex(x2, y2, z2);
                v.skeleton_vertex = skeleton_vertex;
            }
            verticesList.push_back(v);

        }  else if (wordList[0]  == "f") {
            if (wordList.size() < 4) {
                qDebug() << "f word list";
                break;
            }
            unsigned int f1_index = atoi(wordList[1].data()) + m_vertex_offset;
            unsigned int f2_index = atoi(wordList[2].data()) + m_vertex_offset;
            unsigned int f3_index = atoi(wordList[3].data()) + m_vertex_offset;
            if (f1_index > verticesList.size() || f2_index > verticesList.size() || f3_index > verticesList.size()  ) {
                // error, break
                qDebug() << "Error in obj file! " << name.data() << " " << idx << " "
                         << verticesList.size() << " " << f1_index << " " << f2_index
                         << " " << f3_index ;
                delete obj;
                break;
            }

            // add faces indices to object itself
            /* vertex 1 */
            obj->addTriangleIndex(f1_index - 1);
            /* vertex 2 */
            obj->addTriangleIndex(f2_index - 1);
            /* vertex 3 */
            obj->addTriangleIndex(f3_index - 1);
            m_indices_size += 3;

            for (int i = 1; i < bleeding_list.size(); ++i) {
                int rv_index = atoi(bleeding_list[i].data()) + m_vertex_offset;
                if (rv_index > verticesList.size()) {
                    // error skip
                    qDebug() << "error! skiped rv " << rv_index << " " << bleeding_list[i].data();

                    continue;
                }
                verticesList[rv_index].bleed = 1;
            }

        } else if (wordList[0] == "b") { // branch
            // b branch_branchID_parentBranch_knot1_knot2
        } else if (wordList[0] == "sk") { // skeleton point
            // add it to skeleton point cloud
            float x = atof(wordList[1].data());
            float y = atof(wordList[2].data());
            float z = atof(wordList[3].data());
            struct SkeletonVertex sk_vertex = {QVector3D(x, y, z), idx};
            obj->add_s_vertex(sk_vertex);
            m_skeleton_nodes_size++;
        } else if (wordList[0] == "l") { // edge between skeleton points
        } else if (wordList[0] == "rv") {
            if (wordList.size() < 2)
                continue;
            bleeding_list = wordList;
        }
    }

    if (flag_prev == 1) {
        m_buffer_data.push_back(ssbo_object_data);
        m_objects.push_back(obj);
    }

    m_vertex_offset = vertex_offset;

    return true;
}

int Mesh::getVertixCount()
{
    return m_vertices_size;
}

bool Mesh::initVertexAttrib()
{
    if (m_glFunctionsSet == false)
        return false;

    int offset = 0;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexData),  0);


    offset +=  sizeof(QVector3D);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexData), (GLvoid*)offset);


    offset += sizeof(QVector3D);
    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(2, 1, GL_INT, sizeof(VertexData), (GLvoid*)offset);

    offset += sizeof(GL_INT);
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 1, GL_INT, sizeof(VertexData), (GLvoid*)offset);

    return true;

}

// temp functions until we think of way to represent the data
int Mesh::getNodesCount()
{
    return m_skeleton_nodes_size;
}

bool Mesh::initOpenGLFunctions()
{
    m_glFunctionsSet = true;
    initializeOpenGLFunctions();

    return true;
}

bool Mesh::iniShadersVBOs()
{
    initBuffer();

    initMeshShaders();
    initMeshPointsShaders();
    initSkeletonShaders();

    return true;
}

bool Mesh::initBuffer()
{
    if (m_glFunctionsSet == false)
        return false;

    int bufferSize =  m_buffer_data.size() * sizeof(struct ssbo_mesh);

    glGenBuffers(1, &m_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize , NULL, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bindIdx, m_buffer);
    qDebug() << "mesh buffer size: " << bufferSize;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer);
    GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
    memcpy(p,   m_buffer_data.data(),  bufferSize);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    return true;
}

bool Mesh::initMeshShaders()
{
    /* start initializing mesh */
    qDebug() << "Initializing MESH";
    m_program_mesh = glCreateProgram();
    bool res = initShader(m_program_mesh, ":/shaders/mesh.vert", ":/shaders/mesh.geom", ":/shaders/mesh.frag");
    if (res == false)
        return res;

    // create vbos and vaos
    m_vao_mesh.create();
    m_vao_mesh.bind();

    glUseProgram(m_program_mesh); // m_program_mesh->bind();

    updateUniformsLocation(m_program_mesh);


    QVector3D lightDir = QVector3D(-2.5f, -2.5f, -0.9f);
    GLuint lightDir_loc = glGetUniformLocation(m_program_mesh, "diffuseLightDirection");
    glUniform3fv(lightDir_loc, 1, &lightDir[0]);

    m_vbo_mesh.create();
    m_vbo_mesh.setUsagePattern( QOpenGLBuffer::StaticDraw );
    if ( !m_vbo_mesh.bind() ) {
        qDebug() << "Could not bind vertex buffer to the context.";
    }

    m_vbo_mesh.allocate(verticesList.data(), verticesList.size() * sizeof(VertexData));

    // initialize index buffers
    m_vbo_IndexMesh.create();
    m_vbo_IndexMesh.bind();
    m_vbo_IndexMesh.allocate( NULL, m_indices_size * sizeof(GLuint) );
    int offset = 0;
    for (std::size_t i = 0; i < m_objects.size(); i++) {
        if (m_objects[i]->getObjectType() == Object_t::SYNAPSE  ||m_objects[i]->getObjectType() == Object_t::MITO   ) {
            continue;
        }
        int count = m_objects[i]->get_indices_Size() * sizeof(GLuint);
        qDebug() << i << " allocating: " << m_objects[i]->getName().data();
        m_vbo_IndexMesh.write(offset, m_objects[i]->get_indices(), count);
        offset += count;
   }

    m_vbo_IndexMesh.release();

    initVertexAttrib();

    m_vbo_mesh.release();
    m_vao_mesh.release();

}

bool Mesh::initMeshPointsShaders()
{
    /***************************************/
    qDebug() << "Initializing MESH POINTS";
    m_program_mesh_points = glCreateProgram();
    bool res = initShader(m_program_mesh_points, ":/shaders/mesh.vert", ":/shaders/mesh_points.geom", ":/shaders/mesh_points.frag");
    if (res == false)
        return res;

    // create vbos and vaos
    m_vao_mesh_points.create();
    m_vao_mesh_points.bind();

    glUseProgram(m_program_mesh_points);

    QVector3D lightDir = QVector3D(-2.5f, -2.5f, -0.9f);
    GLuint lightDir_loc = glGetUniformLocation(m_program_mesh_points, "diffuseLightDirection");
    glUniform3fv(lightDir_loc, 1, &lightDir[0]);


    if ( !m_vbo_mesh.bind() ) {
        qDebug() << "Could not bind vertex buffer to the context.";
    }

    updateUniformsLocation(m_program_mesh_points);

    initVertexAttrib();

    m_vbo_mesh.release();
    m_vao_mesh_points.release();
}

bool Mesh::initSkeletonShaders()
{
    /********** START SKELETON **************/
    qDebug() << "point";

    m_program_skeleton = glCreateProgram();
    bool res = initShader(m_program_skeleton, ":/shaders/skeleton_point.vert", ":/shaders/skeleton_point.geom", ":/shaders/skeleton_point.frag");
    if (res == false)
        return res;

    qDebug() << "Initializing SKELETON 1";
    m_vao_skeleton.create();
    m_vao_skeleton.bind();

    glUseProgram(m_program_skeleton);

    updateUniformsLocation(m_program_skeleton);

    m_vbo_skeleton.create();
    m_vbo_skeleton.setUsagePattern( QOpenGLBuffer::StaticDraw);
    if ( !m_vbo_skeleton.bind() ) {
        qDebug() << "Could not bind vertex buffer to the context.";
    }

    m_vbo_skeleton.allocate(NULL, m_skeleton_nodes_size * sizeof(SkeletonVertex));

    int offset = 0;
    for (std::size_t i = 0; i < m_objects.size(); i++) {
        int count = m_objects[i]->get_s_Size() * sizeof(SkeletonVertex);
        m_vbo_skeleton.write(offset, &m_objects[i]->get_s_Vertices()[0], count);
        qDebug() << i << " allocating: " << m_objects[i]->getName().data() << " count: " << count;
        offset += count;
   }

    qDebug() << " m_mesh.getNodesCount(): " << m_skeleton_nodes_size;
    qDebug() << " m_mesh.getNodesCount()* sizeof(QVector3D): " << m_skeleton_nodes_size* sizeof(SkeletonVertex);

    GL_Error();

    offset = 0;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkeletonVertex),  0);

    offset += sizeof(QVector3D);
    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(1, 1, GL_INT, sizeof(SkeletonVertex), (GLvoid*)offset);

    GL_Error();

    m_vbo_skeleton.release();
    m_vao_skeleton.release();
}

void Mesh::draw()
{
    /************************/
    // I need this because vertex <-> skeleton mapping is not complete
    m_vao_skeleton.bind();
    glUseProgram(m_program_skeleton);

    updateUniformsLocation(m_program_skeleton);

    glDrawArrays(GL_POINTS, 0,  m_skeleton_nodes_size );
    m_vao_skeleton.release();

    /************************/

    glDisable (GL_BLEND);
    glBlendFunc (GL_ONE, GL_ONE);

    // I need this because transitioning from mesh to skeleton is not smooth
    m_vao_mesh_points.bind();
    glUseProgram(m_program_mesh_points);

    updateUniformsLocation(m_program_mesh_points);

    m_vbo_IndexMesh.bind();
    glDrawElements(GL_POINTS, m_indices_size,  GL_UNSIGNED_INT, 0 );
    m_vbo_IndexMesh.release();

    m_vao_mesh_points.release();
    /************************/
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_vao_mesh.bind();
    glUseProgram(m_program_mesh);

    updateUniformsLocation(m_program_mesh);

    m_vbo_IndexMesh.bind();
    glDrawElements(GL_TRIANGLES, m_indices_size,  GL_UNSIGNED_INT, 0 );
    m_vbo_IndexMesh.release();
    m_vao_mesh.release();
}

void Mesh::updateUniforms(struct MeshUniforms mesh_uniforms)
{
    m_uniforms = mesh_uniforms;
}

void Mesh::updateUniformsLocation(GLuint program)
{
    if (m_glFunctionsSet == false)
        return;

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
