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
    void addSSBOData(QVector4D d);
    int getSSBOSize();
    void* getSSBOData();


protected:
    int                         m_vertices_size;
    int                         m_limit;

    // instead of storing the vertices for each object,
    // store all the vertices at once
    // use global index for each object
    // use that index to render the object or not
    // pros: storage for vertices,
    // cons: global index to objects faces
    std::vector<Object*>        m_objects;
    std::vector<QVector4D>      m_ssbo_data; // Color
};

#endif // MESH_H
