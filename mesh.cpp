#include <chrono>
#include "mesh.h"

Mesh::Mesh()
    :  m_bindIdx(2),
       m_glFunctionsSet(false),
       m_vbo_mesh( QOpenGLBuffer::VertexBuffer ),
       m_vbo_skeleton( QOpenGLBuffer::VertexBuffer )
{
    m_vertices_size = 0;
    m_skeleton_nodes_size = 0;
    m_limit = 500;

    // to do: combine all these files in one .obj file -> m3_dataset.obj
    // to do: interface to load these files
    QString path= "://data/skeleton_astrocyte_m3/mouse3_astro_skelton.obj";
    loadObj(path);
//    path = "://data/mouse03_skeleton_centroid.obj";
//    loadObj(path);
//    path = "://data/mouse03_astro_skeleton.sk";
//    loadSkeletonPoints(path); // 11638884, 19131720
//    path = "://data/mouse03_skeletons.sk";
//    loadSkeletonPoints(path); // 11638884, 19131720
    path = "://scripts/mouse03_mesh_center.obj";
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
    qDebug() << "Func: loadVertices";
    auto t1 = std::chrono::high_resolution_clock::now();

    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "Could not open the file for reading";
        return false;
    }

    QTextStream in(&file);
    QList<QByteArray> wordList;

    Object *obj = NULL;
    std::string name;
    QVector4D center = QVector4D(0.0, 0.0, 0.0, 0.0);
    std::vector< struct VertexData > verticesList;
    GLint idx = -1;

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        wordList = line.split(' ');
        if (wordList[0] == "o") {
            // o objname_hvgxID
            QByteArray nameline  = wordList[1].data();
            QList<QByteArray> nameList = nameline.split('_');
            name.clear();
            for (int i = 0; i < nameList.size() - 1; ++i)
                name += nameList[i].data();

            int hvgxID = atoi(nameList[nameList.size() - 1].data());
            qDebug() << " " << name.data() << " " << hvgxID;

        } else if (wordList[0] == "p") {
            // p centerX centerY centerZ
            float x = atof(wordList[1].data());
            float y = atof(wordList[2].data());
            float z = atof(wordList[3].data());
            if (obj->getObjectType(name) == Object_t::ASTROCYTE ) {
                center = QVector4D(x, y, z, 0);
            } else {
                center = QVector4D(x, y, z, 1);
            }

        } else if (wordList[0]  == "v") {
            float x1 = atof(wordList[1].data());
            float y1 = atof(wordList[2].data());
            float z1 = atof(wordList[3].data());
            QVector3D mesh_vertex(x1, y1, z1);
            struct VertexData v;
            v.ID = idx;
            v.mesh_vertex = mesh_vertex;
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
            unsigned int f1_index = atoi(wordList[1].data());
            unsigned int f2_index = atoi(wordList[2].data());
            unsigned int f3_index = atoi(wordList[3].data());
            if (f1_index > verticesList.size() || f2_index > verticesList.size() || f3_index > verticesList.size()  ) {
                // error, break
                qDebug() << "Error in obj file! " << name.data() << " " << idx << " " << verticesList.size() << " " << f1_index << " " << f2_index << " " << f3_index ;
                delete obj;
                break;
            }

            // add faces indices to object itself
            /* vertex 1 */
            struct VertexData v1 = verticesList[f1_index - 1];
            obj->add_ms_vertex(v1);
            /* vertex 2 */
            struct VertexData v2 = verticesList[f2_index - 1];
            obj->add_ms_vertex(v2);
            /* vertex 3 */
            struct VertexData v3 = verticesList[f3_index - 1];
            obj->add_ms_vertex(v3);

            m_vertices_size += 3;
        }
    }

    return true;
}

// 75892 ----(remove debug msgs)---> 63610
// todo: make this efficient by writing the input as binary file and loading items at once
// todo: get the ID from hvgx and add it to the obj objects names -> used later to map skeleton to objects
bool Mesh::loadObj(QString path)
{
    qDebug() << "Func: loadVertices";
    auto t1 = std::chrono::high_resolution_clock::now();

    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "Could not open the file for reading";
        return false;
    }

    QTextStream in(&file);
    QList<QByteArray> wordList;

    // temp containters
    std::vector< unsigned int > vertexIndices;
    std::vector< struct VertexData > temp_vertices;
    int flag_prev = 0;
    std::string name;
    GLint idx = -1;
    Object *obj = NULL;
    QVector4D color = QVector4D(0.0, 0.0, 0.0, 1.0);
    QVector4D center = QVector4D(0.0, 0.0, 0.0, 0.0);
    struct ssbo_mesh ssbo_object_data;
    // load all vertices once -> should be fast
    // for each object "o", go through its faces, and substitute using vertices loaded at the start
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        wordList = line.split(' ');
        if (wordList[0] == "o") { 
            vertexIndices.clear();
            temp_vertices.clear();

            if (flag_prev == 3) {
                m_buffer_data.push_back(ssbo_object_data);
                m_objects.push_back(obj);
            }

            if (m_objects.size() > m_limit) {
                flag_prev = 0;
                qDebug() << "Size limit";
                break;
            }

            name  = wordList[1].data();
            idx = m_objects.size();
            obj = new Object(name, idx);
           if (obj->getObjectType(name) == Object_t::SYNAPSE  ||obj->getObjectType(name) == Object_t::MITO   ) {
               flag_prev = 0;
               continue;
           }
            // get objet color based on type
            color = obj->getColor();
            ssbo_object_data.color = color;
            flag_prev = 1;
        } else if (wordList[0]  == "p" && flag_prev >= 1 ) {
            float x = atof(wordList[1].data());
            float y = atof(wordList[2].data());
            float z = atof(wordList[3].data());
            if (obj->getObjectType(name) == Object_t::ASTROCYTE ) {
                center = QVector4D(x, y, z, 0);
            } else {
                center = QVector4D(x, y, z, 1);
            }

            obj->setCenter(center);
            ssbo_object_data.center = center;
        } else if (wordList[0]  == "v" && flag_prev >= 1 ) {
            float x1 = atof(wordList[1].data());
            float y1 = atof(wordList[2].data());
            float z1 = atof(wordList[3].data());
            QVector3D mesh_vertex(x1, y1, z1);
            struct VertexData v;
            v.ID = idx;
            v.mesh_vertex = mesh_vertex;
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
            temp_vertices.push_back(v);
            flag_prev = 2;
        } else if (wordList[0]  == "f"  && flag_prev >= 2) {
            unsigned int f1_index = atoi(wordList[1].data());
            unsigned int f2_index = atoi(wordList[2].data());
            unsigned int f3_index = atoi(wordList[3].data());

            if (f1_index > temp_vertices.size() || f2_index > temp_vertices.size() || f3_index > temp_vertices.size()  ) {
                // error, break
                qDebug() << "Error in obj file! " << name.data() << " " << idx << " " << temp_vertices.size() << " " << f1_index << " " << f2_index << " " << f3_index ;
                delete obj;
                flag_prev = 0;
                break;
            }

            /* vertex 1 */
            struct VertexData v1 = temp_vertices[f1_index - 1];
            obj->add_ms_vertex(v1);
            /* vertex 2 */
            struct VertexData v2 = temp_vertices[f2_index - 1];
            obj->add_ms_vertex(v2);
            /* vertex 3 */
            struct VertexData v3 = temp_vertices[f3_index - 1];
            obj->add_ms_vertex(v3);

            m_vertices_size += 3;
            flag_prev = 3;
        } else if (wordList[0] == "vn") {
            qDebug() << "To do compute the normals and read them from here";
        }
    }


    if (flag_prev == 3) {
        m_buffer_data.push_back(ssbo_object_data);
        m_objects.push_back(obj);
    }

    file.close();

    qDebug() << "Done Func: loadVertices, m_ssbo_data size : " <<   m_buffer_data.size();
    auto t2 = std::chrono::high_resolution_clock::now();
    qDebug() << "f() took "
                 << std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count()
                 << " milliseconds";

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
    return true;

}

// todo: combine this with the mesh class
bool Mesh::loadSkeletonPoints(QString path)
{
    qDebug() << "Func: loadSkeletonPoints";
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "Could not open the file for reading";
        return 0;
    }

    QTextStream in(&file);
    QList<QByteArray> wordList;
    std::string name;
    GLint idx = -1;


    struct SkeletonVertex vertex;
    Object *obj = NULL;
    int flag_prev = 0;
    // Point ID, thickness, X Coord, Y Coord, Z Coord, Object ID
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        wordList = line.split(' ');
        if (wordList[0] == "o") {
            if (flag_prev == 1) {
                m_skeletons.push_back(obj);
            }

            if (m_skeletons.size() > m_limit) {
                flag_prev = 0;
                qDebug() << "Size limit";
                break;
            }

            name  = wordList[1].data();
            idx = m_skeletons.size();
            obj = new Object(name, idx);
            if (obj->getObjectType(name) == Object_t::SYNAPSE ||obj->getObjectType(name) == Object_t::MITO  ) {
                flag_prev = 0;
                continue;
            }
            vertex.ID = idx;
            flag_prev = 1;
        } else if (wordList[0] == "p" && flag_prev >= 1) {
            float x = atof(wordList[2].data());
            float y = atof(wordList[3].data());
            float z = atof(wordList[4].data());
            vertex.skeleton_vertex = QVector3D(x, y, z);
            obj->add_s_vertex(vertex);
            m_skeleton_nodes_size++;
        }
    }

    if (flag_prev == 1) {
        m_skeletons.push_back(obj);
    }

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

    GLuint mMatrix = glGetUniformLocation(m_program_mesh, "mMatrix");
    glUniformMatrix4fv(mMatrix, 1, GL_FALSE, m_uniforms.mMatrix);

    GLuint vMatrix = glGetUniformLocation(m_program_mesh, "vMatrix");
    glUniformMatrix4fv(vMatrix, 1, GL_FALSE, m_uniforms.vMatrix);

    GLuint pMatrix = glGetUniformLocation(m_program_mesh, "pMatrix");
    glUniformMatrix4fv(pMatrix, 1, GL_FALSE, m_uniforms.pMatrix);


    QVector3D lightDir = QVector3D(-2.5f, -2.5f, -0.9f);
    GLuint lightDir_loc = glGetUniformLocation(m_program_mesh, "diffuseLightDirection");
    glUniform3fv(lightDir_loc, 1, &lightDir[0]);

    m_vbo_mesh.create();
    m_vbo_mesh.setUsagePattern( QOpenGLBuffer::StaticDraw );
    if ( !m_vbo_mesh.bind() ) {
        qDebug() << "Could not bind vertex buffer to the context.";
    }

    m_vbo_mesh.allocate(NULL, getVertixCount()  * sizeof(VertexData));

    int offset = 0;
    for (std::size_t i = 0; i < m_objects.size(); i++) {
        int count = m_objects[i]->get_ms_Size() * sizeof(VertexData);
        qDebug() << i << " allocating: " << m_objects[i]->getName().data();
        m_vbo_mesh.write(offset, &m_objects[i]->get_ms_Vertices()[0], count);
        offset += count;

   }

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

    GLuint mMatrix = glGetUniformLocation(m_program_mesh_points, "mMatrix");
    glUniformMatrix4fv(mMatrix, 1, GL_FALSE, m_uniforms.mMatrix);

    GLuint vMatrix = glGetUniformLocation(m_program_mesh_points, "vMatrix");
    glUniformMatrix4fv(vMatrix, 1, GL_FALSE, m_uniforms.vMatrix);

    GLuint pMatrix = glGetUniformLocation(m_program_mesh_points, "pMatrix");
    glUniformMatrix4fv(pMatrix, 1, GL_FALSE, m_uniforms.pMatrix);


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
    GLuint mMatrix = glGetUniformLocation(m_program_skeleton, "mMatrix");
    glUniformMatrix4fv(mMatrix, 1, GL_FALSE, m_uniforms.mMatrix);

    GLuint vMatrix = glGetUniformLocation(m_program_skeleton, "vMatrix");
    glUniformMatrix4fv(vMatrix, 1, GL_FALSE, m_uniforms.vMatrix);

    GLuint pMatrix = glGetUniformLocation(m_program_skeleton, "pMatrix");
    glUniformMatrix4fv(pMatrix, 1, GL_FALSE, m_uniforms.pMatrix);


    m_vbo_skeleton.create();
    m_vbo_skeleton.setUsagePattern( QOpenGLBuffer::StaticDraw);
    if ( !m_vbo_skeleton.bind() ) {
        qDebug() << "Could not bind vertex buffer to the context.";
    }

    m_vbo_skeleton.allocate(NULL, getNodesCount() * sizeof(SkeletonVertex));

    int offset = 0;
    for (std::size_t i = 0; i < m_skeletons.size(); i++) {
        int count = m_skeletons[i]->get_s_Size() * sizeof(SkeletonVertex);
        m_vbo_skeleton.write(offset, &m_skeletons[i]->get_s_Vertices()[0], count);
        qDebug() << i << " allocating: " << m_skeletons[i]->getName().data() << " count: " << count;
        offset += count;
   }


    qDebug() << " m_mesh.getNodesCount(): " << getNodesCount();
    qDebug() << " m_mesh.getNodesCount()* sizeof(QVector3D): " << getNodesCount()* sizeof(SkeletonVertex);

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
    GLint y_axis, x_axis;
    GLuint mMatrix, vMatrix, pMatrix;

    /************************/
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_vao_mesh.bind();
    glUseProgram(m_program_mesh);


    mMatrix = glGetUniformLocation(m_program_mesh, "mMatrix");
    glUniformMatrix4fv(mMatrix, 1, GL_FALSE, m_uniforms.mMatrix);

    vMatrix = glGetUniformLocation(m_program_mesh, "vMatrix");
    glUniformMatrix4fv(vMatrix, 1, GL_FALSE, m_uniforms.vMatrix);

    pMatrix = glGetUniformLocation(m_program_mesh, "pMatrix");
    glUniformMatrix4fv(pMatrix, 1, GL_FALSE, m_uniforms.pMatrix);

    y_axis = glGetUniformLocation(m_program_mesh, "y_axis");
    glUniform1iv(y_axis, 1, &m_uniforms.y_axis);

    x_axis = glGetUniformLocation(m_program_mesh, "x_axis");
    glUniform1iv(x_axis, 1, &m_uniforms.x_axis);

    glDrawArrays(GL_TRIANGLES, 0,  getVertixCount() );

    m_vao_mesh.release();
    /************************/
    m_vao_skeleton.bind();
    glUseProgram(m_program_skeleton);
    mMatrix = glGetUniformLocation(m_program_skeleton, "mMatrix");
    glUniformMatrix4fv(mMatrix, 1, GL_FALSE, m_uniforms.mMatrix);

    vMatrix = glGetUniformLocation(m_program_skeleton, "vMatrix");
    glUniformMatrix4fv(vMatrix, 1, GL_FALSE, m_uniforms.vMatrix);

    pMatrix = glGetUniformLocation(m_program_skeleton, "pMatrix");
    glUniformMatrix4fv(pMatrix, 1, GL_FALSE, m_uniforms.pMatrix);

    y_axis = glGetUniformLocation(m_program_skeleton, "y_axis");
    glUniform1iv(y_axis, 1, &m_uniforms.y_axis);

    x_axis = glGetUniformLocation(m_program_skeleton, "x_axis");
    glUniform1iv(x_axis, 1, &m_uniforms.x_axis);

    glDrawArrays(GL_POINTS, 0,  getNodesCount() );
    m_vao_skeleton.release();

    /************************/

    glDisable (GL_BLEND);
    glBlendFunc (GL_ONE, GL_ONE);

    m_vao_mesh_points.bind();
    glUseProgram(m_program_mesh_points);
    mMatrix = glGetUniformLocation(m_program_mesh_points, "mMatrix");
    glUniformMatrix4fv(mMatrix, 1, GL_FALSE, m_uniforms.mMatrix);

    vMatrix = glGetUniformLocation(m_program_mesh_points, "vMatrix");
    glUniformMatrix4fv(vMatrix, 1, GL_FALSE, m_uniforms.vMatrix);

    pMatrix = glGetUniformLocation(m_program_mesh_points, "pMatrix");
    glUniformMatrix4fv(pMatrix, 1, GL_FALSE, m_uniforms.pMatrix);

    y_axis = glGetUniformLocation(m_program_mesh_points, "y_axis");
    glUniform1iv(y_axis, 1, &m_uniforms.y_axis);

    x_axis = glGetUniformLocation(m_program_mesh_points, "x_axis");
    glUniform1iv(x_axis, 1, &m_uniforms.x_axis);

    glDrawArrays(GL_POINTS, 0,  getVertixCount() );
    m_vao_mesh_points.release();




}

void Mesh::updateUniforms(struct MeshUniforms mesh_uniforms)
{
    m_uniforms = mesh_uniforms;
}
