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

    // graph related function
    std::map<int, Object*>  getObjectsMap();
    std::vector<QVector2D> getNeuritesEdges();

    // OpenGL initialization
    bool initOpenGLFunctions();
    bool iniShadersVBOs();
    bool initSSBO();
    bool initVertexAttrib();
    void draw();
    bool initMeshShaders();
    bool initMeshPointsShaders();
    bool initSkeletonShaders();
    void updateUniforms(struct GlobalUniforms uniforms);
    void updateUniformsLocation(GLuint program);

    void fillVBOsData();

    bool filterByType(Object *object_p );

    // ssbo management
    // function to update ssbo data (layout 1, layout 2)
    void update_ssbo_data_layout1(QVector2D layout1, int hvgxID); // access them using IDs
    void update_ssbo_data_layout2(QVector2D layout2, int hvgxID); // access them using IDs

    void update_skeleton_layout1(QVector2D layout1, int node_index, int hvgxID);
    void update_skeleton_layout2(QVector2D layout2, int node_index, int hvgxID);
    void update_skeleton_layout3(QVector2D layout3, int node_index, int hvgxID);

    void write_ssbo_data();

    // temp functions until I move all vbos to here

    int get_neurites_edges_size();
    int get_neurites_nodes_size();
    void allocate_neurites_edges(QOpenGLBuffer vbo);
    void allocate_neurites_nodes(QOpenGLBuffer vbo);

    void allocate_abs_skel_nodes(QOpenGLBuffer vbo);
    void allocate_abs_skel_edges(QOpenGLBuffer vbo);
    int get_abs_skel_nodes_size();
    int get_abs_skel_edges_size();

protected:
    // store all vertices of the mesh.
    // unique vertices, faces to index them.
    Mesh                                *m_mesh;


    int                                 m_skeleton_nodes_size;
    int                                 m_indices_size;
    int                                 m_vertex_offset;
    int                                 m_limit;

    std::map<int, Object*>              m_objects;

    // shared data
    std::vector<struct ssbo_mesh>       m_ssbo_data; // Color, Cenert, Type

    // most abstract data
    // todo: flag in ssbo to indicate this object on/off
    std::vector<GLuint>                 m_neurites_nodes; // (place holders for neurites nodes) this initialized once and not changed after words
    std::vector<GLuint>                 m_neurites_edges; // (place holders for neurites edges) this as well

    std::vector<struct AbstractSkelNode>   m_abstract_skel_nodes; // all skeletons here
    std::vector<GLuint>                 m_abstract_skel_edges;


    GLuint                              m_ssbo;
    GLuint                              m_bindIdx;

    bool                                m_glFunctionsSet;

    /* opengl buffers and vars */
    struct GlobalUniforms               m_uniforms;

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
