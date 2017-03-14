// todo: separate astrocyte and neurites in two different buffers

// manage all vbos and ssbos here
// and drawing calls

// 1) Mesh Triangles
// 2) Skeleton Points
// 3) Abstract Skeleton Graph (Nodes and Edges)
// 4) Neurites Graph (Nodes and Edges)
// 5) Glycogen
#ifndef OPENGLMANAGER_H
#define OPENGLMANAGER_H

#include "abstractionspace.h"
#include "mainopengl.h"
#include "datacontainer.h"
#include "glsluniform_structs.h"
#include "rendervertexdata.h"

class OpenGLManager : public MainOpenGL
{
public:
    OpenGLManager(DataContainer * obj_mnger, AbstractionSpace  *absSpace);
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

    void drawMeshTriangles(struct GlobalUniforms grid_uniforms, bool selection );

    // *********** 3) Skeleton Points    ***********
    bool initSkeletonShaders();
    void drawSkeletonPoints(struct GlobalUniforms grid_uniforms, bool selection );

    // *********** 4) Abstract Skeleton Graph (Nodes and Edges) ***********
    bool initAbstractSkeletonShaders();
    void initSkeletonsVertexAttribPointer();
    void drawSkeletonsGraph(struct GlobalUniforms grid_uniforms, bool selection );

    // ***********  5) Neurites Graph (Nodes and Edges) ***********
    bool initNeuritesGraphShaders();
    void initNeuritesVertexAttribPointer();
    void drawNeuritesGraph(struct GlobalUniforms grid_uniforms);


    // *********** 6) Mesh Points     ***********
    bool initGlycogenPointsShaders();
    void drawGlycogenPoints(struct GlobalUniforms grid_uniforms);
	void updateGlycogenPoints();


    // ssbo management
    // function to update ssbo data (layout 1, layout 2)
    void update_ssbo_data_layout1(QVector2D layout1, int hvgxID); // access them using IDs
    void update_ssbo_data_layout2(QVector2D layout2, int hvgxID); // access them using IDs

    void update_skeleton_layout1(QVector2D layout1,  long node_index, int hvgxID);
    void update_skeleton_layout2(QVector2D layout2,  long node_index, int hvgxID);
    void update_skeleton_layout3(QVector2D layout3,  long node_index, int hvgxID);

    void multiplyWithRotation(QMatrix4x4 rotationMatrix);

    // ********** Filtering ************************

    std::map<int, Object*>  getObjectsMap() { return m_dataContainer->getObjectsMap(); }
    Object_t getObjectTypeByID(int hvgxID);
    void FilterByType(Object_t type);
    void FilterByID( QList<QString> tokens_Ids );
    void FilterByID(  std::vector<int> tokens_Ids );
    void showAll();
    void FilterObject(int ID, bool isfilterd);
    void recursiveFilter(int ID, bool isfilterd);

    void updateDisplayChildFlag(bool flag)          { m_display_child = flag; }
    void updateDisplayParentFlag(bool flag)         { m_display_parent = flag; }
    void updateDisplaySynapseFlag(bool flag)        { m_display_synapses = flag; }

	void setRenderGlycogenGranules(bool render) { m_renderGlycogenGranules = render; };

    // ********** Selection ************************
    void updateCanvasDim(int w, int h, int retianScale);
    void initSelectionFrameBuffer();
    int processSelection(float x, float y);

	void setZoom(float zoom);

protected:
    DataContainer                       *m_dataContainer; // get the data to render from here
    AbstractionSpace                    *m_2dspace;

    bool                                m_glFunctionsSet;
    bool                                m_2D;
    struct GlobalUniforms               m_uniforms;

    // *********** 0) SSBO objects Data    ***********
    // todo: flag in ssbo to indicate this object on/off
    std::vector<struct ssbo_mesh>       m_ssbo_data; // Color, Cenert, Type
    GLuint                              m_ssbo;
    GLuint                              m_bindIdx;

    // *********** 1) Mesh Triangles     ***********

    RenderVertexData                    m_TMesh;
    QOpenGLVertexArrayObject            m_vao_mesh;
    QOpenGLVertexArrayObject            m_vao_selection_mesh;

    QOpenGLBuffer                       m_vbo_mesh;
    QOpenGLBuffer                       m_Neurite_vbo_IndexMesh;


    // *********** 3) Skeleton Points    ***********
    RenderVertexData                    m_SkeletonPoints;
    QOpenGLVertexArrayObject            m_vao_selection_skeleton;
    QOpenGLVertexArrayObject            m_vao_skeleton;
    QOpenGLBuffer                       m_vbo_skeleton;

    // *********** 4) Abstract Skeleton Graph (Nodes and Edges) ***********
    std::vector<struct AbstractSkelNode>   m_abstract_skel_nodes; // all skeletons here
    std::vector<GLuint>                    m_abstract_skel_edges;


    // todo: gather data needed for each render type in a class
    RenderVertexData                        m_GSkeleton;
    QOpenGLVertexArrayObject                m_vao_selection_skeletonGraph;
    QOpenGLVertexArrayObject                m_vao_SkeletonsGraph;


    // ***********  5) Neurites Graph (Nodes and Edges) ***********
    RenderVertexData                    m_GNeurites;
    std::vector<GLuint>                 m_neurites_nodes; // (place holders for neurites nodes) this initialized once and not changed after words
    std::vector<GLuint>                 m_neurites_edges; // (place holders for neurites edges) this as well

    QOpenGLVertexArrayObject            m_NeuritesGraphVAO;
    QOpenGLBuffer                       m_NeuritesIndexVBO;
    QOpenGLBuffer                       m_NeuritesNodesVBO;

    // *********** 6) Glycogen Nodes     ***********
    RenderVertexData                    m_GlycogenPoints;
    QOpenGLVertexArrayObject            m_vao_glycogen;
    QOpenGLBuffer                       m_vbo_glycogen;

	bool								m_renderGlycogenGranules;
	float								m_zoom;

    // ********** Selection ************************
    int                                 m_canvas_w;
    int                                 m_canvas_h;
    int                                 m_retinaScale;

    int                                 m_hits;
    GLuint                              m_selectionFrameBuffer;
    GLuint                              m_selectionRenderBuffer;

    bool                                m_display_child;
    bool                                m_display_parent;
    bool                                m_display_synapses;


 };

#endif // OPENGLMANAGER_H
