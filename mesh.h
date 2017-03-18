#ifndef MESH_H
#define MESH_H

#include "mainopengl.h"

#include <set>
#include <vector>

enum class Object_t;

struct face {
   int v[3];
};

// mesh vertex
struct VertexData {
    QVector4D   mesh_vertex;        // w: Object ID
    QVector4D   skeleton_vertex;    // w: markers distance to astrocyte (since this is per vertex, compute the distance from each vertex to astrocyte)


    int			index;
    //bool		isGlycogen;			//because glycogen ids and object ids are seperate

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

    int addVertex(struct VertexData* vdata, Object_t type);
    int addVertexNormal(QVector4D vnormal);
    bool isValidFaces(int f1, int f2, int f3);
    int  getVerticesSize()       { return verticesList.size(); }

    void addVertexNeighbor(int v_index, int face_index);

    std::vector< struct VertexData >* getVerticesList()  { return &verticesList; }
    std::vector< VertexData* >* getVerticesListByType(Object_t type)  {  return &(m_typeVertexList[(int)type]); }

    void addFace(int index1, int index2, int index3);
    void getVertexNeighbors(int v_index, std::set< int > &neighs);

    std::vector< struct face > *getFacesList()      { return &m_faces; }
    int getFacesListSize()                          { return m_faces.size(); }

    // opengl functions
    void allocateVerticesVBO(QOpenGLBuffer vbo_mesh);
    void allocateNormalsVBO(QOpenGLBuffer vbo_mesh);

    void dumpVertexData(const char* filename);
    void readVertexData(const char* filename);

    void dumpVericesList(const char* filename);
    bool readVertexBinary(const char* filename);

    void dumpFaceList(const char* filename);
    bool readFacesBinary(const char* filename);

    void dumpNormalsList(const char* filename);
    bool readNormalsBinary(const char* filename);

protected:
    std::vector< struct face >          m_faces; // sequential, write

    // each vertex could belong to more than one face
    // for each vertex in each face, add face to vertex

    // set of faces
    std::vector< struct VertexData  >    verticesList; // .
    std::map<int, std::vector<int> >    m_vertexFaces;

    std::vector< QVector4D >            m_normalsList;

    std::vector<VertexData*> m_typeVertexList[9];
    /*std::vector<VertexData*> m_spineVertexList;
    std::vector<VertexData*> m_boutonVertexList;
    std::vector<VertexData*> m_mitochondriaVertexList;
    std::vector<VertexData*> m_astrocytesVertexList;
    std::vector<VertexData*> m_axonsVertexList;
    std::vector<VertexData*> m_dendritesVertexList;*/


};

#endif // MESH_H
