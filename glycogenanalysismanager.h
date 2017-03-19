#ifndef GLYCOGENANALYSISMANAGER_H_
#define GLYCOGENANALYSISMANAGER_H_

#include <map>
#include <vector>
#include <QVector3D>
#include "dbscan.h"

class Glycogen;
namespace Clustering {
	class GlycogenCluster;
}
struct VertexData;

namespace SpacePartitioning {
	class Octree;
	class SpatialHash3D;
}

class GlycogenAnalysisManager
{

	public:
		GlycogenAnalysisManager(std::map<int, Glycogen*>*, std::vector<VertexData*>*, SpacePartitioning::Octree*, std::vector< struct VertexData >*);
		~GlycogenAnalysisManager();

		void setBoutonAndSpineOctrees(SpacePartitioning::SpatialHash3D*, SpacePartitioning::SpatialHash3D*);

		std::map<int, Clustering::GlycogenCluster*>* runDBScan(float eps, int minPts);

		std::map<int, std::map<int, int>>*  computeGlycogenMapping(bool boutons, bool spines, bool clusters);
		
		

	protected:
		void clear();

		void computeGlycogenMappingToBoutons();
		void computeGlycogenMappingToSpines();
		void computeGlycogenMappingToBoutonsAndSpines();
		void computeGlycogenClusterMappingToBoutons();
		void computeGlycogenClusterMappingToSpines();
		void computeGlycogenClusterMappingToBoutonsAndSpines();

		//list of all vertices pointer
		std::vector< struct VertexData >*		m_verticesList;

		//glycogen data pointers
		std::map<int, Glycogen*>* m_glycogenMap;
		std::vector<VertexData*>* m_glycogenVertexDataList;

		//dbscan algorithm
		Clustering::DBScan		  m_dbscan; 

		//octrees 
		SpacePartitioning::SpatialHash3D*				m_spineHash;
		SpacePartitioning::SpatialHash3D*				m_boutonHash;
		SpacePartitioning::Octree*				m_glycogenOctree;

		//clustering results list
		std::map<int, Clustering::GlycogenCluster*> m_clusterResults;

		//mapping results lists
		std::map<int, int> m_glycogenIdToObjectVertexIndexMapping;
		std::map<int, std::map<int, int>> m_objectIdToGlycogenMapping;
};





#endif // GLYCOGENANALYSISMANAGER_H_