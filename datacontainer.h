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
//#include "dbscan.h"

class DataContainer
{
public:
    DataContainer();
    ~DataContainer();

    bool importXML(QString path);
    void parseObject(QXmlStreamReader &xml, Object *obj);
    void parseMesh(QXmlStreamReader &xml, Object *obj);
    void parseSkeleton(QXmlStreamReader &xml, Object *obj);
    void parseSkeletonNodes(QXmlStreamReader &xml, Object *obj);
    void parseBranch(QXmlStreamReader &xml, Object *obj);
    void parseSkeletonPoints(QXmlStreamReader &xml, Object *obj);

    void loadConnectivityGraph(QString path);
    void loadMetaDataHVGX(QString path);

	//glycogen
    int getGlycogenSize()              { return m_glycogenMap.size(); }
    std::map<int, Glycogen*> getGlycogenMap() { return m_glycogenMap; }
	std::map<int, Glycogen*>* getGlycogenMapPtr() { return &m_glycogenMap; }
	std::vector<VertexData*>* getGlycogenVertexDataPtr() { return &m_glycogenList; }
	SpacePartitioning::Octree* getGlycogenOctree() { return &m_glycogenOctree; }

	SpacePartitioning::Octree* getSpineOctree() { return &m_spineOctree; }
	SpacePartitioning::Octree* getBoutonOctree() { return &m_boutonOctree; }

    // graph related function
    std::map<int, Object*>  getObjectsMap();
    std::vector<QVector2D> getNeuritesEdges();

    int getSkeletonPointsSize();
    int getMeshIndicesSize();
    Mesh* getMeshPointer();

    Object_t getObjectTypeByID(int hvgxID);
    std::string getObjectName(int hvgxID);
    std::vector<int> getObjectsIDsByType(Object_t type);

    int getMaxAstroCoverage()   { return max_astro_coverage; }
    int getMaxVolume()          { return max_volume; }

    // iterate over objects and get max volume and astro coverage
    void recomputeMaxVolAstro()          { return; }

protected:
    int                                     m_tempCounter; // how many objects with no contacts to ast based on threshold
     // maximum volume from displayed objects
    int                                     max_volume;
    // maximum vertices from neurites covered by astrocyte
    int                                     max_astro_coverage;

    // store all vertices of the mesh.
    // unique vertices, faces to index them.
    Mesh                                    *m_mesh;

    Object                                  *m_curParent;
    int                                     m_skeleton_points_size;
    int                                     m_indices_size;
    int                                     m_vertex_offset;
    int                                     m_limit;

    std::map<int, Object*>                  m_objects;

    // for each type, make a list of their IDs
    std::map< Object_t, std::vector<int> >  m_objectsIDsByType;

    // graph related data
    std::vector<QVector2D>                  neurites_neurite_edge;

    std::map<int, Glycogen*>                m_glycogenMap;
	std::vector<VertexData*>				m_glycogenList;

	//octrees
	SpacePartitioning::Octree				m_spineOctree;
	SpacePartitioning::Octree				m_boutonOctree;
	SpacePartitioning::Octree				m_glycogenOctree;

};

#endif // OBJECTMANAGER_H
