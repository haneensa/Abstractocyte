// cluster glycogen
// color glycoen based on cluster
// find closest object to center of cluster
// color that object with glycogen cluster

// todo: normalize volume baed on the non filtered objects
// get the largest volume from all present objects
// and normalize based on that
// so whwn dendrites is filtered, other volumes can be relatively objserved without astrocyte

#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

// file manipulations
#include <QString>
#include <QFile>
#include <QXmlStreamReader>

#include "object.h"
#include "mesh.h"
#include "glycogen.h"
#include "octree.h"
#include "grid3d.h"
//#include "dbscan.h"

enum class LoadData_t { ALL, ASTRO, NEURITES };
enum class LoadFile_t { DUMP_ASTRO, DUMP_NEURITES, LOAD_MESH_NO_VERTEX, LOAD_MESH_W_VERTEX };
enum class Normals_t { DUMP_NORMAL, LOAD_NORMAL, NO_NORMALS };

class DataContainer
{
public:
    DataContainer();
    ~DataContainer();

    void loadData();

    void loadNormals(QString path, const char* filename);
    void loadObjNormals(QString path);

    int importXML(QString path);
    void parseObject(QXmlStreamReader &xml, Object *obj);
    void parseMesh(QXmlStreamReader &xml, Object *obj);
    void parseMeshNoVertexnoFace(QXmlStreamReader &xml, Object *obj);
    void parseSkeleton(QXmlStreamReader &xml, Object *obj);
    void parseSkeletonNodes(QXmlStreamReader &xml, Object *obj);
    void parseBranch(QXmlStreamReader &xml, Object *obj);
    void parseSkeletonPoints(QXmlStreamReader &xml, Object *obj);

	/*bool importXML_DOM(QString path);
	void parseObject_DOM(QXmlStreamReader &xml, Object *obj);
	void parseMesh_DOM(QXmlStreamReader &xml, Object *obj);
	void parseSkeleton_DOM(QXmlStreamReader &xml, Object *obj);
	void parseSkeletonNodes_DOM(QXmlStreamReader &xml, Object *obj);
	void parseBranch_DOM(QXmlStreamReader &xml, Object *obj);
	void parseSkeletonPoints_DOM(QXmlStreamReader &xml, Object *obj);*/

    void loadConnectivityGraph(QString path);
    void loadMetaDataHVGX(QString path);
    void loadParentFromHVGX(QString path);

	//glycogen
    int getGlycogenSize()              { return m_glycogenMap.size(); }
    std::map<int, Glycogen*> getGlycogenMap() { return m_glycogenMap; }
	std::map<int, Glycogen*>* getGlycogenMapPtr() { return &m_glycogenMap; }
	std::vector<VertexData*>* getGlycogenVertexDataPtr() { return &m_glycogenList; }
	SpacePartitioning::Octree* getGlycogenOctree() { return &m_glycogenOctree; }

	SpacePartitioning::Octree* getSpineOctree() { return &m_spineOctree; }
	SpacePartitioning::Octree* getBoutonOctree() { return &m_boutonOctree; }
	float* getGlycogen3DGridData();

    // graph related function
    std::map<int, Object*>  getObjectsMap();
	std::map<int, Object*>*  getObjectsMapPtr() { return &m_objects; }
    std::vector<QVector2D> getNeuritesEdges();

    int getSkeletonPointsSize();
    int getMeshIndicesSize();
    Mesh* getMeshPointer();

    Object_t getObjectTypeByID(int hvgxID);
    std::string getObjectName(int hvgxID);
    std::vector<Object*> getObjectsByType(Object_t type);

    float getMaxAstroCoverage()   { return max_astro_coverage; }
    int getMaxVolume()          { return max_volume; }

    // iterate over objects and get max volume and astro coverage
    void recomputeMaxVolAstro();

    //************ Load Raw Data
    char* loadRawFile(QString path, int size);

protected:
    int                                         m_tempCounter; // how many objects with no contacts to ast based on threshold
     // maximum volume from displayed objects
    int                                         max_volume;
    // maximum vertices from neurites covered by astrocyte
    float                                       max_astro_coverage;

    // store all vertices of the mesh.
    // unique vertices, faces to index them.
    Mesh                                        *m_mesh;

    Object                                      *m_curParent;
    int                                         m_skeleton_points_size;
    int                                         m_indices_size; // used to allocate indices of a mesh
    int                                         m_vertex_offset; // used to unify vertices for one mesh
    int                                         m_faces_offset; // used to unify vertices for one mesh

    int                                         m_limit;

    std::map<int, int>                          m_parents;
    std::map<int, Object*>                      m_objects;
    std::map<Object_t, std::vector<Object*> >   m_objectsByType;
    std::map<Object_t, int >                    m_indices_size_byType;


    // graph related data
    std::vector<QVector2D>                      neurites_neurite_edge;

	// glycogen
    std::map<int, Glycogen*>                    m_glycogenMap;
    std::vector<VertexData*>                    m_glycogenList;
	SpacePartitioning::Grid3D					m_glycogen3DGrid;

	// octrees
    SpacePartitioning::Octree                   m_spineOctree;
    SpacePartitioning::Octree                   m_boutonOctree;
    SpacePartitioning::Octree                   m_glycogenOctree;

    // file management
    Normals_t                                   m_normals_t;
    LoadFile_t                                  m_loadType;
    LoadData_t                                  m_load_data;
};

#endif // OBJECTMANAGER_H
