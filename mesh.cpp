#include <chrono>
#include "mesh.h"

Mesh::Mesh()
{
    m_vertices_size = 0;
    m_limit = 20;
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
// todo: combine skeleton and mesh information in one obj file
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
    bool flag_prev = false;
    std::string name;

    // load all vertices once -> should be fast
    // for each object "o", go through its faces, and substitute using vertices loaded at the start
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        wordList = line.split(' ');
        if (wordList[0] == "o") {
            if (flag_prev & vertexIndices.size() > 0) {
                GLint idx = m_objects.size();
                idx  = 1;
                Object *obj = new Object(name, idx);
                m_vertices_size += vertexIndices.size();

                // todo: move this to "f" so instead of looping again on faces insert them once
                // todo: use vertix buffer and index buffer
                for ( unsigned int i = 0; i < vertexIndices.size(); ++i ) {
                    unsigned int vertexIndex = vertexIndices[i];
                    struct VertexData v = temp_vertices[vertexIndex - 1];
                    v.ID = idx;
                    obj->add_ms_vertex(v);
                }
                QVector3D color = QVector3D(1.0, 0.0, 1.0) ;
                m_ssbo_data.push_back(color);
                obj->setColor(QVector4D(color, 1.0));
                m_objects.push_back(obj);
                if (m_objects.size() > m_limit) {
                    flag_prev = false;
                    qDebug() << "Size limit";
                    break;
                }
            }

            name  = wordList[1].data();
            vertexIndices.clear();
            temp_vertices.clear();
            flag_prev = true;
        } else if (wordList[0]  == "v") {
            float x1 = atof(wordList[1].data());
            float y1 = atof(wordList[2].data());
            float z1 = atof(wordList[3].data());
            QVector3D mesh_vertex(x1, y1, z1);
            struct VertexData v;
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
        } else if (wordList[0]  == "f") {
            unsigned int f1_index = atoi(wordList[1].data());
            unsigned int f2_index = atoi(wordList[2].data());
            unsigned int f3_index = atoi(wordList[3].data());
            vertexIndices.push_back(f1_index);
            vertexIndices.push_back(f2_index);
            vertexIndices.push_back(f3_index);
            // insert into vertexIndices here
        } else if (wordList[0] == "vn") {
            qDebug() << "To do compute the normals and read them from here";
        }
    }

    if (flag_prev & vertexIndices.size() > 0) {
        GLint idx = m_objects.size();
        Object *obj = new Object(name, idx);
        m_vertices_size += vertexIndices.size();
        for ( unsigned int i = 0; i < vertexIndices.size(); ++i ) {
            unsigned int vertexIndex = vertexIndices[i];
            struct VertexData v = temp_vertices[vertexIndex - 1];
            v.ID = idx;
            obj->add_ms_vertex(v);
        }
        QVector3D color = QVector3D(1.0, 0.0, 0.0) ;
        m_ssbo_data.push_back(color);
        obj->setColor(QVector4D(color, 1.0));
        m_objects.push_back(obj);
    }

    file.close();

    qDebug() << "Done Func: loadVertices";
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
    int offset = 0;
    int vertices = 0;
    for (std::size_t i = 0; i < m_objects.size(); i++) {
        vertices +=  m_objects[i]->get_ms_Size();
        int count = m_objects[i]->get_ms_Size() * sizeof(VertexData);
        qDebug() << i << " allocating: " << m_objects[i]->getName().data() <<  " " << m_objects.size() <<" m_vertices_size: " << m_vertices_size << " all bytes: " <<  m_vertices_size * sizeof(VertexData) << " vertices: " << vertices << " count: " << count << " offset: " << offset;
        vbo.write(offset, &m_objects[i]->get_ms_Vertices()[0], count);
        offset += count;

   }

    return true;
}


void Mesh::addSSBOData(QVector3D d)
{
    m_ssbo_data.push_back(d);
}

int Mesh::getSSBOSize()
{
    return (sizeof(m_ssbo_data) * m_ssbo_data.size());
}

void* Mesh::getSSBOData()
{
    return m_ssbo_data.data();
}
