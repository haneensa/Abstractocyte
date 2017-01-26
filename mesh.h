#ifndef MESH_H
#define MESH_H

// file manipulations
#include <QString>
#include <QFile>

#include "object.h"
struct ssbo_mesh {
    QVector4D color;
    QVector4D center;
};

class Mesh
{
public:
    Mesh();
    ~Mesh();
    bool loadObj(QString path);
    int getVertixCount();
    bool initVBO(QOpenGLBuffer vbo);

    // temp functions
    bool loadSkeletonPoints(QString path);
    int getNodesCount();
    bool initSkeletonVBO(QOpenGLBuffer vbo);

    // ssbo buffer data
    int getSSBOSize();
    void* getSSBOData();


protected:
    int                         m_vertices_size;
    int                         m_skeleton_nodes_size;

    int                         m_limit;

    // instead of storing the vertices for each object,
    // store all the vertices at once
    // use global index for each object
    // use that index to render the object or not
    // pros: storage for vertices,
    // cons: global index to objects faces
    std::vector<Object*>                m_objects;
    std::vector<Object*>                m_skeletons;
    std::vector<struct ssbo_mesh>    m_ssbo_data; // Color, Cenert, Type
};

#endif // MESH_H
