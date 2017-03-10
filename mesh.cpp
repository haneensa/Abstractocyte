#include "mesh.h"

Mesh::Mesh()
{

}

void Mesh::addVertex(struct VertexData vdata, Object_t type)
{
    verticesList.push_back(vdata);
	vdata.index = verticesList.size() - 1;
	m_typeVertexList[static_cast<int>(type)].push_back(&vdata);
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

void Mesh::MarkBleedingVertices(QStringList markersList, int vertex_offset)
{
    for (int i = 0; i < markersList.size(); ++i) {
        int rv_index = markersList.at(i).toInt() + vertex_offset;
        if (rv_index >= verticesList.size() || rv_index < 1) {
            qDebug() << "error! skiped rv " << rv_index << " vertices list size: " << verticesList.size();
            continue;
        }
        verticesList[rv_index-1].skeleton_vertex.setW(1);
    }
}


void Mesh::allocateVerticesVBO(QOpenGLBuffer vbo_mesh)
{
    vbo_mesh.allocate(verticesList.data(), verticesList.size() * sizeof(VertexData));
}
