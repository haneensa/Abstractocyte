#ifndef MESH_H
#define MESH_H

// file manipulations
#include <QString>
#include <QFile>
#include "mainopengl.h"

#include "object.h"
struct ssbo_mesh {
    QVector4D color;
    QVector4D center;   // center.w = neurite/astrocyte
    QVector4D info;     // volume, type (axon, bouton, spine, dendrite, ..), ?, ?
};

struct MeshUniforms {
    GLint y_axis;
    GLint x_axis;
    float* mMatrix;
    float* vMatrix;
    float* pMatrix;
};

class Mesh : public MainOpenGL
{
public:
    Mesh();
    ~Mesh();
    int getVertixCount();

    bool loadDataset(QString path);
    // temp functions
    bool loadSkeletonPoints(QString path);
    int getNodesCount();

    // OpenGL initialization
    bool initOpenGLFunctions();
    bool iniShadersVBOs();
    bool initBuffer();
    bool initVertexAttrib();
    void draw();
    bool initMeshShaders();
    bool initMeshPointsShaders();
    bool initSkeletonShaders();
    void updateUniforms(struct MeshUniforms mesh_uniforms);
    void updateUniformsLocation(GLuint program);

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
    QOpenGLVertexArrayObject            m_vao_mesh;
    QOpenGLBuffer                       m_vbo_mesh;
    QOpenGLBuffer                       m_vbo_IndexMesh;
    GLuint                              m_program_mesh;


    QOpenGLVertexArrayObject            m_vao_mesh_points;
    GLuint                              m_program_mesh_points;


    QOpenGLVertexArrayObject            m_vao_skeleton;
    QOpenGLBuffer                       m_vbo_skeleton;
    GLuint                              m_program_skeleton;
    struct MeshUniforms                 m_uniforms;

    // store all vertices of the mesh.
    // vertices are sequential increasing for all objects
    std::vector< struct VertexData >    verticesList;
    int                                 m_indices_size;
};

#endif // MESH_H
