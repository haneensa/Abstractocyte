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

enum class Size_e { VOLUME, ASTRO_COVERAGE, SYNAPSE_SIZE };
enum class Color_e { TYPE, FUNCTION, ASTRO_COVERAGE, GLYCOGEN_MAPPING };
enum class HeatMap2D_e { ASTRO_COVERAGE, GLYCOGEN_MAPPING };

class OpenGLManager : public MainOpenGL
{
public:
    OpenGLManager(DataContainer * obj_mnger, AbstractionSpace  *absSpace);
    ~OpenGLManager();
    void fillSSBO(Object*);
    int fillMeshVBO(Object *object_p, int offset, std::string vboLabel);
    int fillSkeletonPoints(Object *object_p, int offset);
    void fillGraphData(Object *object_p);
    void fillVBOsData();
    bool initOpenGLFunctions();
    void updateAbstractUniformsLocation(GLuint program);
    void update2Dflag(bool is2D);

    void updateUniformsData(struct GlobalUniforms grid_uniforms)    {    m_uniforms = grid_uniforms; }

    void drawAll();

    // *********** 0) SSBO objects Data    ***********
    bool initSSBO();
    void write_ssbo_data();

    // *********** 1) Mesh Triangles     ***********
    bool initMeshTrianglesShaders();
    bool initMeshVertexAttrib();
    void updateMeshPrograms(GLuint program);
    void drawMeshTriangles(bool selection );

    void renderVBOMesh(std::string vbolabel, int indices);
    void renderOrderToggle();

    // *********** 3) Skeleton Points    ***********
    bool initSkeletonShaders();
    void drawSkeletonPoints(bool selection );
    void updateSkeletonUniforms(GLuint program);
    // *********** 4) Abstract Skeleton Graph (Nodes and Edges) ***********
    bool initAbstractSkeletonShaders();
    void initSkeletonsVertexAttribPointer();
    void drawSkeletonsGraph(bool selection );
    void updateSkeletonGraphUniforms(GLuint program);
    void updateSkeletonGraphTransitionUniforms(GLuint program);

    // ***********  5) Neurites Graph (Nodes and Edges) ***********
    bool initNeuritesGraphShaders();
    void initNeuritesVertexAttribPointer();
    void drawNeuritesGraph();
    void updateNeuritesGraphUniforms(GLuint program);

    // *********** 6)Glycogen     ***********
    bool initGlycogenPointsShaders();
    void drawGlycogenPoints();
	void updateGlycogenPoints();
    void updateGlycogenUniforms(GLuint program);
    //***************************************
    void renderAbstractions();

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
    std::map<int, Object*>*  getObjectsMapPtr() { return &m_dataContainer->getObjectsMap(); }

    Object_t getObjectTypeByID(int hvgxID);
    void FilterByType(Object_t type, bool);
    void FilterByID( QList<QString> tokens_Ids, bool );
    void FilterByID(  std::set<int> tokens_Ids, bool );
    void VisibilityToggleSelectedObjects(  std::set<int> tokens_Ids, bool );
    void showAll();
    void FilterObject(int ID, bool isfilterd);
    void recursiveFilter(int ID, bool isfilterd);

    void updateDisplayChildFlag(bool flag)          { m_display_child = flag; }
    void updateDisplayParentFlag(bool flag)         { m_display_parent = flag; }
    void updateDisplaySynapseFlag(bool flag)        { m_display_synapses = flag; }

    void setRenderGlycogenGranules(bool render) { m_renderGlycogenGranules = render; }
    void updateDepth(int d)                         { m_depth = d; }

    void updateNodeSizeEncoding(Size_e encoding);
    void updateColorEncoding(Color_e encoding);
    void update2DTextureEncoding(HeatMap2D_e encoding);

    void updateSSBO();

    // ********** Selection ************************
    void updateCanvasDim(int w, int h, int retianScale);
    void initSelectionFrameBuffer();
    int processSelection(float x, float y);
    void renderSelection();

    void highlightObject(int hvgxID);

	void setZoom(float zoom);

	// ********** Splatting Enable ************************
	void setAstroSplat(bool enable) { m_is_astro_splat = enable; }
	void setGlycoSplat(bool enable) { m_is_glyco_splat = enable; }
	void setAMitoSplat(bool enable) { m_is_amito_splat = enable; }
	void setNMitoSplat(bool enable) { m_is_nmito_splat = enable; }

	void setSpecularLight(bool enable) { m_is_specular = enable; }

    // ************ 2D HeatMap  *******
    void init2DHeatMapTextures();
    bool init2DHeatMapShaders();
    void drawNodesInto2DTexture();
    void render2DHeatMapTexture();
    void update2DTextureUniforms(GLuint program);
    // ************ 3D HeatMap  *******
	void init_Gly3DTex();
	void init_TF(GLuint &texture, GLenum texture_unit, GLvoid* data, int size);
	void upload_Gly3DTex(void* data, int sizeX = DIM_X, int sizeY = DIM_Y, int sizeZ = DIM_Z, GLenum type = GL_UNSIGNED_BYTE);
	void load3DTexturesFromRaw(QString path, GLuint &texture, GLenum texture_unit, int sizeX = DIM_X, int sizeY = DIM_Y, int sizeZ = DIM_Z);
	void load3DTexturesFromRaw_3(QString path1, QString path2, QString path3, GLuint &texture, GLenum texture_unit, int sizeX = DIM_X, int sizeY = DIM_Y, int sizeZ = DIM_Z);
protected:
    DataContainer                           *m_dataContainer;
    AbstractionSpace                        *m_2dspace;

    bool                                    m_glFunctionsSet;
    bool                                    m_2D;
    struct GlobalUniforms                   m_uniforms;

    // *********** 0) SSBO objects Data    ***********
    std::vector<struct ssbo_mesh>           m_ssbo_data; // Color, Cenert, Type
    GLuint                                  m_ssbo;
    GLuint                                  m_bindIdx;

    // *********** 1) Mesh Triangles     ***********
    RenderVertexData                        m_TMesh;
    bool                                    m_normals_enabled;
    std::string                             m_neurites_VBO_label;
    std::string                             m_astro_VBO_label;
    bool                                    m_transparent_astro;

    // *********** 3) Skeleton Points    ***********
    RenderVertexData                        m_SkeletonPoints;

    // *********** 4) Abstract Skeleton Graph (Nodes and Edges) ***********
    RenderVertexData                        m_GSkeleton;
    std::vector<struct AbstractSkelNode>    m_abstract_skel_nodes; // all skeletons here
    std::vector<GLuint>                     m_abstract_skel_edges;

    // ***********  5) Neurites Graph (Nodes and Edges) ***********
    RenderVertexData                        m_GNeurites;
    // (place holders for neurites nodes) this initialized once and not changed after words
    std::vector<GLuint>                     m_neurites_nodes;
    // (place holders for neurites edges) this as well
    std::vector<GLuint>                     m_neurites_edges;

    // *********** 6) Glycogen Nodes     ***********
    RenderVertexData                        m_GlycogenPoints;
    QOpenGLVertexArrayObject                m_vao_glycogen;
    QOpenGLBuffer                           m_vbo_glycogen;

	bool								m_renderGlycogenGranules;
    float                                   m_zoom;

	// ********** Splatting ************************
	bool									m_is_astro_splat;
	bool									m_is_glyco_splat;
	bool									m_is_nmito_splat;
	bool									m_is_amito_splat;

	// ********** lighting ************************
	bool									m_is_specular;

    // ********** Selection ************************
    int                                     m_canvas_w;
    int                                     m_canvas_h;
    int                                     m_retinaScale;

    GLuint                                  m_selectionFrameBuffer;
    GLuint                                  m_selectionRenderBuffer;

    bool                                    m_display_child;
    bool                                    m_display_parent;
    bool                                    m_display_synapses;
    int                                     m_depth;

    int                                     m_hoveredID;

    Color_e                                 m_color_encoding;
    Size_e                                  m_size_encoding;
    HeatMap2D_e                             m_2DHeatMap_encoding;

    // ************ 2D HeatMap  *******
    GLuint                                  m_2D_heatMap_FBO_H;
    GLuint                                  m_2D_heatMap_FBO_V;

    GLuint                                  m_2D_heatMap_Tex;
    int                                     m_quadSize;
    std::vector<QVector4D>                  m_Texquad;
    GLuint                                  m_tf_2DHeatMap_tex;
    std::vector<QVector4D>                  m_tf_2DHeatmap;

    GLuint                                  m_tf_2DHeatMap2_tex;
    std::vector<QVector4D>                  m_tf_2DHeatmap2;

    // ************ 3D HeatMap  *******
	GLuint									m_splat_volume_3DTex;
	GLuint									m_gly_3D_Tex;
    GLuint                                  m_astro_3DTex;
    GLuint                                  m_mito_3DTex;
	GLuint									m_nmito_3DTex;
	GLuint									m_glycogen_3DTex;
	GLuint									m_tf_glycogen;
 };

#endif // OPENGLMANAGER_H
