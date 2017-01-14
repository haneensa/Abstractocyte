#ifndef MESH_H
#define MESH_H

// file manipulations
#include <QString>
#include <QFile>

#include "object.h"

class Mesh
{
public:
    Mesh();
    ~Mesh();
    bool loadObj(QString path);
    int getVertixCount();
    bool initVBO(QOpenGLBuffer vbo);

protected:
    int                         m_vertices_size;
    int                         m_limit;
    std::vector<Object*>        m_objects;
};

#endif // MESH_H
