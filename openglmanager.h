// manage all vbos and ssbos here
// and drawing calls

// 1) Mesh Triangles
// 2) Mesh Points
// 3) Skeleton Points
// 3) Abstract Skeleton Graph (Nodes and Edges)
// 4) Neurites Graph (Nodes and Edges)

#ifndef OPENGLMANAGER_H
#define OPENGLMANAGER_H

#include "mainopengl.h"
#include "datacontainer.h"
#include "glsluniform_structs.h"

class OpenGLManager : public MainOpenGL
{
public:
    OpenGLManager(DataContainer * obj_mnger);
    ~OpenGLManager();
    void fillVBOsData();
    bool initOpenGLFunctions();
    void updateUniformsLocation(GLuint program);
    void updateAbstractUniformsLocation(GLuint program);
    void update2Dflag(bool is2D);

    void drawAll(struct GlobalUniforms grid_uniforms);

    // *********** 0) SSBO objects Data    ***********
    bool initSSBO();
    void write_ssbo_data();

    // *********** 1) Mesh Triangles     ***********
    bool initMeshTrianglesShaders();
    bool initMeshVertexAttrib();

    void drawMeshTriangles(struct GlobalUniforms grid_uniforms);

    // *********** 2) Mesh Points     ***********
    bool initMeshPointsShaders();
    void drawMeshPoints(struct GlobalUniforms grid_uniforms);

    // *********** 3) Skeleton Points    ***********
    bool initSkeletonShaders();
    void drawSkeletonPoints(struct GlobalUniforms grid_uniforms);

    // *********** 4) Abstract Skeleton Graph (Nodes and Edges) ***********
    bool initAbstractSkeletonShaders();
    void initSkeletonsVertexAttribPointer();
    void drawSkeletonsGraph(struct GlobalUniforms grid_uniforms);

    // ***********  5) Neurites Graph (Nodes and Edges) ***********
    bool initNeuritesGraphShaders();
    void initNeuritesVertexAttribPointer();
    void drawNeuritesGraph(struct GlobalUniforms grid_uniforms);



    // ssbo management
    // function to update ssbo data (layout 1, layout 2)
    void update_ssbo_data_layout1(QVector2D layout1, int hvgxID); // access them using IDs
    void update_ssbo_data_layout2(QVector2D layout2, int hvgxID); // access them using IDs

    void update_skeleton_layout1(QVector2D layout1, int node_index, int hvgxID);
    void update_skeleton_layout2(QVector2D layout2, int node_index, int hvgxID);
    void update_skeleton_layout3(QVector2D layout3, int node_index, int hvgxID);



protected:
    DataContainer                       *m_obj_mnger; // get the data to render from here

    bool                                m_glFunctionsSet;
    bool                                m_2D;
    struct GlobalUniforms               m_uniforms;

    // *********** 0) SSBO objects Data    ***********
    // todo: flag in ssbo to indicate this object on/off
    std::vector<struct ssbo_mesh>       m_ssbo_data; // Color, Cenert, Type
    GLuint                              m_ssbo;
    GLuint                              m_bindIdx;

    // *********** 1) Mesh Triangles     ***********

    QOpenGLVertexArrayObject            m_vao_mesh;
    QOpenGLBuffer                       m_vbo_mesh;
    QOpenGLBuffer                       m_vbo_IndexMesh;
    GLuint                              m_program_mesh;

    // *********** 2) Mesh Points     ***********
    QOpenGLVertexArrayObject            m_vao_mesh_points;
    GLuint                              m_program_mesh_points;

    // *********** 3) Skeleton Points    ***********
    QOpenGLVertexArrayObject            m_vao_skeleton;
    QOpenGLBuffer                       m_vbo_skeleton;
    GLuint                              m_program_skeleton;

    // *********** 4) Abstract Skeleton Graph (Nodes and Edges) ***********
    std::vector<struct AbstractSkelNode>   m_abstract_skel_nodes; // all skeletons here
    std::vector<GLuint>                    m_abstract_skel_edges;

    QOpenGLVertexArrayObject            m_SkeletonsGraphVAO;
    QOpenGLBuffer                       m_SkeletonsIndexVBO;
    QOpenGLBuffer                       m_SkeletonsNodesVBO;

    GLuint                              m_program_skeletons_2D_index;
    GLuint                              m_program_skeletons_2D_nodes;

    GLuint                              m_program_skeletons_23D_index;
    GLuint                              m_program_skeletons_23D_nodes;

    // ***********  5) Neurites Graph (Nodes and Edges) ***********
    std::vector<GLuint>                 m_neurites_nodes; // (place holders for neurites nodes) this initialized once and not changed after words
    std::vector<GLuint>                 m_neurites_edges; // (place holders for neurites edges) this as well

    QOpenGLVertexArrayObject            m_NeuritesGraphVAO;
    QOpenGLBuffer                       m_NeuritesIndexVBO;
    QOpenGLBuffer                       m_NeuritesNodesVBO;


    GLuint                              m_program_neurites_nodes;
    GLuint                              m_program_neurites_index;
};

#endif // OPENGLMANAGER_H
