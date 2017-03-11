#ifndef MESH_H
#define MESH_H

#include "mainopengl.h"

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
        return mesh_vertex.x();
    }
    float  z()  const
    {
        return mesh_vertex.x();
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

protected:
    // faces indices
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
