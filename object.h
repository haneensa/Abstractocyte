#ifndef OBJECT_H
#define OBJECT_H

#include <QVector3D>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>


enum class Object_t { AXON, DENDRITE, BOUTON, SPINE, MITO, SYNAPSE, ASTROCYTE, GLYCOGEN, UNKNOWN };

struct VertexData {
    QVector3D   mesh_vertex;
    QVector3D   skeleton_vertex;
    // add  bool value for marking this vertex
    GLint       ID;
    GLint       bleed;
};

struct SkeletonVertex {
    QVector3D   skeleton_vertex;
    GLint       ID;
};

class Object
{
public:
    Object(std::string name, int idx, int ID = 0);
    ~Object();

    // add mesh and skeleton vertices together using VertexData
    void add_s_vertex(struct SkeletonVertex vertex_data);

    void addTriangleIndex(GLuint faces);

    std::vector<struct SkeletonVertex> get_s_Vertices();

    size_t get_s_Size();
    size_t get_indices_Size()   { return m_meshIndices.size(); }
    void* get_indices()         { return m_meshIndices.data(); }
    Object_t getObjectType();
    std::string getName();

    QVector4D getColor();
    QVector4D getCenter();
    int getVolume();

    void setColor(QVector4D color);
    void setCenter(QVector4D center);
    void setVolume(int volume);

    int getHVGXID()                 { return m_ID; }
    int getIdxID()                 { return m_idxID; }


private:
    std::string                             m_name;
    int                                     m_ID; // hvgx
    int                                     m_idxID;

    // object properties
    Object_t                                m_object_t;     /* object type */
    int                                     m_volume;
    int                                     m_function; // -1:not applicable, 0:ex, 1:in, 3:unknown
    int                                     m_parentID; // -1 if none

    QVector4D                               m_color;

    // object data representtion
    QVector4D                               m_center;
    std::vector< struct SkeletonVertex >    m_skeleton_vertices;

    std::vector<GLuint>                     m_meshIndices;

    // graph related data
    //  skeleton of this neurites (nodes + edges between them)
};

#endif // OBJECT_H
