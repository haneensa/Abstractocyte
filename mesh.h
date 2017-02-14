#ifndef MESH_H
#define MESH_H

#include "mainopengl.h"

// mesh vertex
struct VertexData {
    QVector3D   mesh_vertex;
    QVector3D   skeleton_vertex;
    GLint       ID;
    GLint       bleed;
};

class Mesh
{
public:
    Mesh();
    void addVertex(struct VertexData vdata);
    bool isValidFaces(int f1, int f2, int f3);
    void MarkBleedingVertices(QStringList markersList, int vertex_offset); // for xml
    void MarkBleedingVertices(QList<QByteArray> markersList, int vertex_offset); // for .obj
    int getVerticesSize()       { return verticesList.size(); }
    void allocateVerticesVBO(QOpenGLBuffer vbo_mesh);

protected:
    // faces indices
    // set of faces
    std::vector< struct VertexData >    verticesList;
    int                                 m_indices_size;

};

#endif // MESH_H
