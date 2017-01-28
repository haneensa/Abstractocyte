#ifndef MESH_H
#define MESH_H

// file manipulations
#include <QString>
#include <QFile>
#include "mainopengl.h"

#include "object.h"
struct ssbo_mesh {
    QVector4D color;
    QVector4D center;
};

struct MeshUniforms {
    GLint y_axis;
    GLint x_axis;
    float* mMatrix;
    float* vMatrix;
    float* pMatrix;
};

class Mesh : protected MainOpenGL
{
public:
    Mesh();
    ~Mesh();
    bool loadObj(QString path);
    int getVertixCount();


    // temp functions
    bool loadSkeletonPoints(QString path);
    int getNodesCount();
    bool initSkeletonVBO(QOpenGLBuffer vbo);

    // OpenGL initialization
    bool initBuffer();
    bool initOpenGLFunctions(struct MeshUniforms mesh_uniforms);
    bool initVBO(QOpenGLBuffer vbo);
    bool initVertexAttrib();
    void draw(struct MeshUniforms mesh_uniforms);
    bool initMeshShaders();
    bool initMeshPointsShaders();
    bool initSkeletonShaders();

protected:
    int                                 m_vertices_size;
    int                                 m_skeleton_nodes_size;

    int                                 m_limit;

    // instead of storing the vertices for each object,
    // store all the vertices at once
    // use global index for each object
    // use that index to render the object or not
    // pros: storage for vertices,
    // cons: global index to objects faces
    std::vector<Object*>                m_objects;
    std::vector<Object*>                m_skeletons;
    std::vector<struct ssbo_mesh>       m_buffer_data; // Color, Cenert, Type
    GLuint                              m_buffer;
    GLuint                              m_bindIdx;

    bool                                m_glFunctionsSet;

    /* opengl buffers and vars */
    QOpenGLVertexArrayObject    m_vao_mesh;
    QOpenGLBuffer               m_vbo_mesh;
    GLuint                      m_program_mesh;


    QOpenGLVertexArrayObject    m_vao_mesh_points;
    GLuint                      m_program_mesh_points;


    QOpenGLVertexArrayObject    m_vao_skeleton;
    QOpenGLBuffer               m_vbo_skeleton;
    GLuint                      m_program_skeleton;
    struct MeshUniforms         m_uniforms;

};

#endif // MESH_H
