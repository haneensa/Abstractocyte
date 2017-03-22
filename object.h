// todo: add mitochondira node to neurites skeletons
// neurites dont have points cooridnates
// they use parents nodes coordinates so they only have index in the skeleton
//
#ifndef OBJECT_H
#define OBJECT_H

#include <QVector3D>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

#include "skeleton.h"
#include "mesh.h"
#include "ssbo_structs.h"

#define ASTRO_DIST_THRESH   0.0001

// if object type == synapse
struct synapse {
    int axon;
    int dendrite;
    int spine;
    int bouton;
};

enum class Object_t { AXON = 2, DENDRITE = 4, BOUTON = 3 , SPINE = 5, MITO = 1, SYNAPSE = 7, ASTROCYTE = 6, GLYCOGEN = 8, UNKNOWN = 0, ASTRO_MITO = 9 };

class Skeleton;
class Object
{
public:
    Object(std::string name, int ID);
    ~Object();

    // temporary skeleton data points handleing function
    void add_s_vertex(struct SkeletonPoint vertex_data);

    int writeSkeletontoVBO(QOpenGLBuffer vbo, int offset);
    // mesh indices functions
    void addTriangleIndex(GLuint faces);
    size_t get_indices_Size()           { return m_meshIndices.size(); }
    void* get_indices()                 { return m_meshIndices.data(); }

    // properties getters
    bool hasParent();
    Object_t getObjectType();
    std::string getName()               {  return m_name; }
    QVector4D getColor();
    QVector4D getCenter()               { return m_center; }
    QVector4D getAstPoint()               { return m_ast_point; }
    int getVolume()                     { return m_volume; }
    int getHVGXID()                     { return m_ID; }
    struct ssbo_mesh getSSBOData();

    Object* getParent()                 { return m_parent; }
    int getParentID()                   { return m_parentID; }

    std::vector<Object*> getChildren()  { return m_children; }
    float getAstroCoverage();
    int getFunction()                   { return m_function; }

    // properties setters
    void setFunction(int function)      { m_function = function; }
    void setColor(QVector4D color)      {  m_color = color; }
    void setCenter(QVector4D center);
    void setAstPoint(QVector4D ast_point);
    void setVolume(int volume)          { m_volume = volume; }

    void setParentID(int parentID)      { m_parentID = parentID; }

    void addChild(Object *child);
    // skeleton management
    void addSkeletonNode(QVector3D coords);
    void addSkeletonPoint(QVector3D coords);
    bool addSkeletonBranch(SkeletonBranch *branch, Object *parent);
    Skeleton* getSkeleton()            { return m_skeleton; }
    void fixSkeleton(Object *parent);
    void setSkeletonOffset(int offset) { m_skeleton->setIndexOffset(offset);}
    int  getSkeletonOffset()           { return m_skeleton->getIndexOffset(); }
    void setNodeIdx(int node_index)    { m_nodeIndx = node_index ; }
    int  getNodeIdx()                  { return m_nodeIndx; }

    void markChildSubSkeleton(SkeletonBranch *branch, int ID);

    void updateFilteredFlag(bool isFiltered)   { m_isFiltered = isFiltered; }
    bool isFiltered()                          { return m_isFiltered; }


    void updateAstSynapseFlag(bool flag)        { m_isAstroSynapse = flag; }

    void updateClosestAstroVertex(float dist,  int vertexIdx);
    void UpdateSynapseData(int axons_id, int dendrite_id, int spine_id, int bouton_id);
    void addSynapse(Object *synapse_object);
    std::vector<Object*> getSynapses()          { return m_synapses; }
    struct synapse getSynapseData()             { return m_synapse_data; }

	void setMappedValue(float value){ m_mappedValue = value; }
	float getMappedValue() { return m_mappedValue; }

private:
    std::string                             m_name;
    int                                     m_ID; // hvgx

    int                                     m_nodeIndx;

    // object properties
    Object_t                                m_object_t;     /* object type */
    int                                     m_volume;
    int                                     m_function;     // -1:not applicable, 0:ex, 1:in, 3:unknown

    Object                                  *m_parent;    // NULL if none
    int                                     m_parentID;
    std::vector<Object*>                    m_children;   // axon-> bouton, den->spine

    QVector4D                               m_center;
    // closest point from astrocyte skeleton to this object so we can project the object on skeleton and be part of it
    QVector4D                               m_ast_point;
    QVector4D                               m_color;

    Skeleton                                *m_skeleton;

    // indices to access the global mesh vertices defined in mesh
    std::vector<GLuint>                     m_meshIndices;

    bool                                    m_isFiltered;   // 1 yes, 0 no

    bool                                    m_isAstroSynapse;

    std::pair<int, float>                   m_closest_astro_vertex;

    std::vector<int>                        m_VertexidxCloseToAstro;  // less than ASTRO_DIST_THRESH ----> we can refine this later
    float                                     m_averageDistance;
	float									m_mappedValue;

    // list of synapses
    std::vector<Object*>                    m_synapses;


    struct synapse                          m_synapse_data;

    std::map<Object_t, void*>               m_dataByType; // use this to store info based on data type
};

#endif // OBJECT_H
