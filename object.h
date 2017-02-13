#ifndef OBJECT_H
#define OBJECT_H

#include <QVector3D>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include "skeleton.h"

enum class Object_t { AXON, DENDRITE, BOUTON, SPINE, MITO, SYNAPSE, ASTROCYTE, GLYCOGEN, UNKNOWN };

// ssbo data structure per object
struct VertexData {
    QVector3D   mesh_vertex;
    QVector3D   skeleton_vertex;
    GLint       ID;
    GLint       bleed;
};

// skeleton ssbo per object
struct SkeletonVertex {
    QVector3D   skeleton_vertex;
    GLint       ID;
};

struct ssbo_mesh {
    QVector4D color;
    QVector4D center;   // center.w = neurite/astrocyte
    QVector4D info;     // volume, type (axon, bouton, spine, dendrite, ..), ?, ?
};

class Object
{
public:
    Object(std::string name, int ID);
    ~Object();

    // temporary skeleton data points handleing function
    void add_s_vertex(struct SkeletonVertex vertex_data);
    std::vector<struct SkeletonVertex> get_s_Vertices() { return m_skeleton_vertices; }
    size_t get_s_Size()                 {  return m_skeleton_vertices.size(); }

    // mesh indices functions
    void addTriangleIndex(GLuint faces);
    size_t get_indices_Size()           { return m_meshIndices.size(); }
    void* get_indices()                 { return m_meshIndices.data(); }

    // properties getters
    Object_t getObjectType();
    std::string getName()               {  return m_name; }
    QVector4D getColor();
    QVector4D getCenter()               { return m_center; }
    int getVolume()                     { return m_volume; }
    int getHVGXID()                     { return m_ID; }
    struct ssbo_mesh                    getSSBOData();

    // properties setters
    void setColor(QVector4D color)      {  m_color = color; }
    void setCenter(QVector4D center);
    void setVolume(int volume)          { m_volume = volume; }



private:
    std::string                             m_name;
    int                                     m_ID; // hvgx
    // object properties
    Object_t                                m_object_t;     /* object type */
    int                                     m_volume;
    int                                     m_function; // -1:not applicable, 0:ex, 1:in, 3:unknown
    int                                     m_parentID; // -1 if none
    QVector4D                               m_center;
    QVector4D                               m_color;


    // replace this with Skeleton object
    std::vector< struct SkeletonVertex >    m_skeleton_vertices;

    // indices to access the global mesh vertices defined in mesh
    std::vector<GLuint>                     m_meshIndices;
};

#endif // OBJECT_H
