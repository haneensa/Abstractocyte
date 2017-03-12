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
class VertexData;

namespace SpacePartitioning {
	class Octree;
}

class GlycogenAnalysisManager
{

	public:
		GlycogenAnalysisManager(std::map<int, Glycogen*>*, std::vector<VertexData*>*, SpacePartitioning::Octree*);
		~GlycogenAnalysisManager();

		std::map<int, Clustering::GlycogenCluster*>* runDBScan(float eps, int minPts);


	protected:
		void clear();


		std::map<int, Glycogen*>* m_glycogenMap;
		std::vector<VertexData*>* m_glycogenVertexDataList;

		Clustering::DBScan		  m_dbscan; 

		SpacePartitioning::Octree*				m_spineOctree;
		SpacePartitioning::Octree*				m_boutonOctree;
		SpacePartitioning::Octree*				m_glycogenOctree;

		std::map<int, Clustering::GlycogenCluster*> m_clusterResults;
};





#endif // GLYCOGENANALYSISMANAGER_H_