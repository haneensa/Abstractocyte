#include "mesh.h"
#include <fstream>
#include <sstream>

Mesh::Mesh()
{

}

void Mesh::dumpVericesList()
{
    const char* filename = "binary_data.dat";
    std::ofstream outfile(filename, std::ios::binary);
    // write size of file into binary and read it the first thing to allocate
    for (int i = 0; i < verticesList.size(); ++i) {
        outfile.write((char*)&verticesList[i], sizeof(struct VertexData));
    }

    outfile.close();
}

void Mesh::readVertexBinary()
{
    const int size = 21;
    struct VertexData  v[size];
    const char *filename = "binary_data.dat";
    std::ifstream ss(filename, std::ios::binary);
    ss.read((char*)&v, size * sizeof(struct VertexData));
    for (int i = 0; i < size; i++) {
        qDebug() << v[i].mesh_vertex << " " << v[i].skeleton_vertex << " " << v[i].index;
    }
}

int Mesh::addVertex(struct VertexData vdata, Object_t type)
{
    int idx = verticesList.size();
    verticesList.push_back(vdata);
	vdata.index = verticesList.size() - 1;
	m_typeVertexList[static_cast<int>(type)].push_back(&vdata);
    return idx;
}

int Mesh::addVertexNormal(QVector4D vnormal)
{
    int idx = m_normalsList.size();
    m_normalsList.push_back(vnormal);
    return idx;
}

bool Mesh::isValidFaces(int f1, int f2, int f3)
{
    if (    f1 > verticesList.size()
            || f2 > verticesList.size()
            || f3 > verticesList.size()  ) {
        qDebug() << "Error in obj file, verticesList size: " << verticesList.size();
        return false;
    }

    return true;
}

void Mesh::allocateVerticesVBO(QOpenGLBuffer vbo_mesh)
{
    vbo_mesh.allocate(verticesList.data(), verticesList.size() * sizeof(VertexData));
}

void Mesh::allocateNormalsVBO(QOpenGLBuffer vbo_mesh)
{
    vbo_mesh.allocate(m_normalsList.data(), m_normalsList.size() * sizeof(QVector4D));
}

void Mesh::addFace(int index1, int index2, int index3)
{
    struct face f;
    f.v[0] = index1;
    f.v[1] = index2;
    f.v[2] = index3;

    int face_index = m_faces.size();
    m_faces.push_back(f);

    m_vertexFaces[index1].push_back(face_index);
    m_vertexFaces[index2].push_back(face_index);
    m_vertexFaces[index3].push_back(face_index);
}

void Mesh::getVertexNeighbors(int v_index, std::set< int > &neighs)
{
    // Use 'mesh' to walk the connectivity.
    std::vector<int>  faces_list = m_vertexFaces[v_index];
    for (int i = 0; i < faces_list.size(); ++i) {
        struct face f = m_faces[faces_list[i]];
        for (int j = 0; j < 3; ++j) {
            if (f.v[j] != v_index) {
                neighs.insert(f.v[j]);
            }
        }
    }
}

