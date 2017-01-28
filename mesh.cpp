#include <chrono>
#include "mesh.h"

Mesh::Mesh()
    :  m_bindIdx(2),
       m_glFunctionsSet(false)
{
    m_vertices_size = 0;
    m_skeleton_nodes_size = 0;
    m_limit = 2;
}

Mesh::~Mesh()
{
    for (std::size_t i = 0; i != m_objects.size(); i++) {
        delete m_objects[i];
    }
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
           if (obj->getObjectType(name) == Object_t::AXON ||obj->getObjectType(name) == Object_t::DENDRITE ) {
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



bool Mesh::initVBO(QOpenGLBuffer vbo)
{
    vbo.allocate(NULL, getVertixCount()  * sizeof(VertexData));

    int offset = 0;
    for (std::size_t i = 0; i < m_objects.size(); i++) {
        int count = m_objects[i]->get_ms_Size() * sizeof(VertexData);
        qDebug() << i << " allocating: " << m_objects[i]->getName().data();
        vbo.write(offset, &m_objects[i]->get_ms_Vertices()[0], count);
        offset += count;

   }

    return true;
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

int Mesh::getBufferSize()
{
    return  m_buffer_data.size() * sizeof(struct ssbo_mesh);
}

void* Mesh::getBufferData()
{
    return &m_buffer_data[0];
    //return m_ssbo_data.data();
}

void Mesh::initOpenGLFunctions()
{
    m_glFunctionsSet = true;
    initializeOpenGLFunctions();
}

bool Mesh::initBuffer()
{
    if (m_glFunctionsSet == false)
        return false;

    glGenBuffers(1, &m_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, getBufferSize() , NULL, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bindIdx, m_buffer);
    qDebug() << "mesh buffer size: " << getBufferSize();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer);
    GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
    memcpy(p,  getBufferData(),  getBufferSize());
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
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
            if (obj->getObjectType(name) == Object_t::AXON ||obj->getObjectType(name) == Object_t::DENDRITE ) {
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


bool Mesh::initSkeletonVBO(QOpenGLBuffer vbo)
{
    if (m_glFunctionsSet == false)
        return false;

    int offset = 0;
    for (std::size_t i = 0; i < m_skeletons.size(); i++) {
        int count = m_skeletons[i]->get_s_Size() * sizeof(SkeletonVertex);
        vbo.write(offset, &m_skeletons[i]->get_s_Vertices()[0], count);
        qDebug() << i << " allocating: " << m_skeletons[i]->getName().data() << " count: " << count;
        offset += count;
   }

    return true;
}

