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
