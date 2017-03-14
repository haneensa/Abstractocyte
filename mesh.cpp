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

void Mesh::addTriangle(int index1, int index2, int index3)
{
    trimesh::triangle_t triangle;
    triangle.v[0] = index1;
    triangle.v[1] = index2;
    triangle.v[2] = index3;

    yig_triangles.push_back(triangle);
}

void  Mesh::buildMeshHalfEdge()
{
    trimesh::unordered_edges_from_triangles( yig_triangles.size(), &yig_triangles[0], yig_edges );
    yig_mesh.build( verticesList.size(),
                    yig_triangles.size(), &yig_triangles[0],
                    yig_edges.size(), &yig_edges[0] );

}
void Mesh::getVertexNeighbors(int v_index, std::vector< trimesh::index_t > &neighs)
{
    // Use 'mesh' to walk the connectivity.
    yig_mesh.vertex_vertex_neighbors( v_index, neighs );
}
