#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

// file manipulations
#include <QString>
#include <QFile>
#include <QXmlStreamReader>

#include "mainopengl.h"
#include "object.h"
#include "glsluniform_structs.h"
#include "mesh.h"


class ObjectManager : public MainOpenGL
{
public:
    ObjectManager();
    ~ObjectManager();

    bool importXML(QString path);
    void parseObject(QXmlStreamReader &xml, Object *obj);
    void parseMesh(QXmlStreamReader &xml, Object *obj);
    void parseSkeleton(QXmlStreamReader &xml, Object *obj);
    void parseSkeletonNodes(QXmlStreamReader &xml, Object *obj);
    void parseBranch(QXmlStreamReader &xml, Object *obj);
    void parseConnGraph(QXmlStreamReader &xml);
    void parseSkeletonPoints(QXmlStreamReader &xml, Object *obj);

    int getNodesCount();
    // graph related function
    std::map<int, Object*>  getObjectsMap();
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
    // store all vertices of the mesh.
    // unique vertices, faces to index them.
    Mesh                                *m_mesh;

    int                                 m_skeleton_nodes_size;
    int                                 m_indices_size;
    int                                 m_vertex_offset;
    int                                 m_limit;

    //std::vector<Object*>                m_objects; // make this map by hvgx ID instead of vector
    std::map<int, Object*>              m_objects;
    std::vector<struct ssbo_mesh>       m_ssbo_data; // Color, Cenert, Type
    GLuint                              m_ssbo;
    GLuint                              m_bindIdx;

    bool                                m_glFunctionsSet;

    /* opengl buffers and vars */
    struct MeshUniforms                 m_uniforms;

    QOpenGLVertexArrayObject            m_vao_mesh;
    QOpenGLBuffer                       m_vbo_mesh;
    QOpenGLBuffer                       m_vbo_IndexMesh;
    GLuint                              m_program_mesh;

    QOpenGLVertexArrayObject            m_vao_mesh_points;
    GLuint                              m_program_mesh_points;

    QOpenGLVertexArrayObject            m_vao_skeleton;
    QOpenGLBuffer                       m_vbo_skeleton;
    GLuint                              m_program_skeleton;

    // graph related data
    std::vector<QVector2D>              neurites_neurite_edge;
};

#endif // OBJECTMANAGER_H
