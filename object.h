#ifndef OBJECT_H
#define OBJECT_H

#include <QVector3D>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

enum class Object_t { AXON, DENDRITE, BOUTON, SPINE, MITO, SYNAPSE, ASTROCYTE, GLYCOGEN };

class Object
{
public:
    Object(std::string name);
    ~Object();
    void add_vertex(QVector3D vertex);
    void add_ms_vertex(QVector3D mesh_vertex, QVector3D skeleton_vertex);
    std::vector<QVector3D> getVertices();
    Object_t getObjectType(std::string name);
    std::string getName();
    int getSize();
    QVector4D getColor();
    void setColor(QVector4D color);

private:

    struct VertexData {
        QVector3D mesh_vertex;
        QVector3D skeleton_vertex;
    };

    Object_t                m_object_t;     /* object type */
    std::string             m_name;
    int                     m_ID;
    std::vector<QVector3D>  m_mesh_vertices;
    std::vector<QVector3D>  m_skeleton_vertices;

    std::vector< struct VertexData >  m_mesh_skeleton_vertices;

    QVector4D               m_color;

    // store skeleton nodes and edges here. graph? skeleton?
    // one graph for all skeletons?
};

#endif // OBJECT_H
