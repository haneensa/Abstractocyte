#ifndef MESH_H
#define MESH_H

// file manipulations
#include <QString>
#include <QFile>
#include <QXmlStreamReader>

#include "mainopengl.h"
#include "graph.h"
#include "object.h"

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
    bool importXML(QString path);
    void parseObject(QXmlStreamReader &xml, Object *obj);
    void parseMesh(QXmlStreamReader &xml, Object *obj);
    void parseSkeleton(QXmlStreamReader &xml, Object *obj);
    void parseBranch(QXmlStreamReader &xml);
    void parseConnGraph(QXmlStreamReader &xml);


    int getNodesCount();
    // graph related function
    std::map<int, Object*>  getNeuriteNodes();
    std::vector<QVector2D> getNeuritesEdges();

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

    //std::vector<Object*>                m_objects; // make this map by hvgx ID instead of vector
    std::map<int, Object*>              m_objects;
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

    int                                 m_vertex_offset;

    // graph related data
    std::vector<QVector2D>              neurites_neurite_edge;
};

#endif // MESH_H
