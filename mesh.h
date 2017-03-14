#ifndef MESH_H
#define MESH_H

#include "mainopengl.h"
#include "trimesh.h"

enum class Object_t;

// mesh vertex
struct VertexData {
    QVector4D   mesh_vertex;        // w: Object ID
    QVector4D   skeleton_vertex;    // w: markers distance to astrocyte (since this is per vertex, compute the distance from each vertex to astrocyte)


    int			index;
    bool		isGlycogen;			//because glycogen ids and object ids are seperate

    float  x()  const
    {
        return mesh_vertex.x();
    }
    float  y()  const
    {
        return mesh_vertex.y();
    }
    float  z()  const
    {
        return mesh_vertex.z();
    }
    float id() const
    {
        return mesh_vertex.w();
    }
};

class Mesh
{
public:
    Mesh();

    int addVertex(struct VertexData vdata, Object_t type);
    bool isValidFaces(int f1, int f2, int f3);
    int  getVerticesSize()       { return verticesList.size(); }
    void allocateVerticesVBO(QOpenGLBuffer vbo_mesh);
    std::vector< struct VertexData >* getVerticesList()  { return &verticesList; }
    std::vector< VertexData* >* getVerticesListByType(Object_t type)  {  return &m_typeVertexList[(int)type]; }

    void addTriangle(int index1, int index2, int index3);

    void buildMeshHalfEdge();

    void getVertexNeighbors(int v_index, std::vector< trimesh::index_t > &neighs);

protected:
    // faces indices
    std::vector< trimesh::triangle_t >          yig_triangles;
    std::vector< trimesh::edge_t >              yig_edges;
    trimesh::trimesh_t                          yig_mesh;

    // set of faces
    std::vector< struct VertexData >    verticesList;


    std::vector<VertexData*> m_typeVertexList[9];
    /*std::vector<VertexData*> m_spineVertexList;
    std::vector<VertexData*> m_boutonVertexList;
    std::vector<VertexData*> m_mitochondriaVertexList;
    std::vector<VertexData*> m_astrocytesVertexList;
    std::vector<VertexData*> m_axonsVertexList;
    std::vector<VertexData*> m_dendritesVertexList;*/


};

#endif // MESH_H
