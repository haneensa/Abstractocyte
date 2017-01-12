#ifndef OBJECT_H
#define OBJECT_H

#include <QVector3D>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

enum class Object_t { AXON, DENDRITE, BOUTON, SPINE, MITO, SYNAPSE, ASTROCYTE, GLYCOGEN };

struct VertexData {
    QVector3D mesh_vertex;
    QVector3D skeleton_vertex;
};

class Object
{
public:
    Object(std::string name);
    ~Object();

    // add vertex to mesh only
    void add_m_vertex(QVector3D vertex);
    // add vertex to skeleton only
    void add_s_vertex(QVector3D vertex);
    // add mesh and skeleton vertices together using VertexData
    void add_ms_vertex(struct VertexData vertex_data);

    std::vector<QVector3D> get_m_Vertices();
    std::vector<QVector3D> get_s_Vertices();
    std::vector<struct VertexData> get_ms_Vertices();

    int get_m_Size();
    int get_s_Size();
    int get_ms_Size();

    Object_t getObjectType(std::string name);
    std::string getName();
    QVector4D getColor();
    void setColor(QVector4D color);

private:
    Object_t                m_object_t;     /* object type */
    std::string             m_name;
    int                     m_ID;
    std::vector<QVector3D>  m_mesh_vertices;
    std::vector<QVector3D>  m_skeleton_vertices;
    std::vector< struct VertexData >  m_mesh_skeleton_vertices;
    QVector4D               m_color;
};

#endif // OBJECT_H
