#ifndef OBJECT_H
#define OBJECT_H

#include <QVector3D>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

enum class Object_t { AXON, DENDRITE, BOUTON, SPINE, MITO, SYNAPSE, ASTROCYTE, GLYCOGEN, UNKNOWN };

struct VertexData {
    QVector3D mesh_vertex;
    QVector3D skeleton_vertex;
    GLint       ID;
};

struct SkeletonVertex {
    QVector3D skeleton_vertex;
    GLint       ID;
};

class Object
{
public:
    Object(std::string name, int idx, int ID = 0);
    ~Object();

    // add vertex to mesh only
    void add_m_vertex(QVector3D vertex);
    // add vertex to skeleton only
    void add_s_vertex(struct SkeletonVertex vertex);
    // add mesh and skeleton vertices together using VertexData
    void add_ms_vertex(struct VertexData vertex_data);

    std::vector<QVector3D> get_m_Vertices();
    std::vector<struct SkeletonVertex> get_s_Vertices();
    std::vector<struct VertexData> get_ms_Vertices();

    size_t get_m_Size();
    size_t get_s_Size();
    size_t get_ms_Size();

    Object_t getObjectType(std::string name);
    std::string getName();
    QVector4D getColor();
    void setColor(QVector4D color);

private:
    Object_t                m_object_t;     /* object type */
    std::string             m_name;
    int                     m_ID;
    int                     m_idxID;
    std::vector<QVector3D>  m_mesh_vertices;
    std::vector<struct SkeletonVertex>  m_skeleton_vertices;
    std::vector< struct VertexData >  m_mesh_skeleton_vertices;
    QVector4D               m_color;
};

#endif // OBJECT_H
