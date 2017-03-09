#ifndef MESH_H
#define MESH_H

#include "mainopengl.h"

// mesh vertex
struct VertexData {
    QVector4D   mesh_vertex;        // w: ID
    QVector4D   skeleton_vertex;    // w: markers
};

class Mesh
{
public:
    Mesh();
    void addVertex(struct VertexData vdata);
    bool isValidFaces(int f1, int f2, int f3);
    bool MarkBleedingVertices(QStringList markersList, int vertex_offset); // for xml
    int getVerticesSize()       { return verticesList.size(); }
    void allocateVerticesVBO(QOpenGLBuffer vbo_mesh);

protected:
    // faces indices
    // set of faces
    std::vector< struct VertexData >    verticesList;

};

#endif // MESH_H
