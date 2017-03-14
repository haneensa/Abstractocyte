#include "mesh.h"

Mesh::Mesh()
{

}


int Mesh::addVertex(struct VertexData vdata, Object_t type)
{
    int idx = verticesList.size();
    verticesList.push_back(vdata);
	vdata.index = verticesList.size() - 1;
	m_typeVertexList[static_cast<int>(type)].push_back(&vdata);
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

