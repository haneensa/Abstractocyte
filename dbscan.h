#ifndef DBSCAN_H_
#define DBSCAN_H_

#include <vector>
#include <map>
#include <stdint.h>
#include <cassert>
#include <cmath>

struct VertexData;
class  GlycogenCluster;
namespace SpacePartitioning {
	class  Octree;
}
class  Glycogen;

namespace Clustering
{

	class DBScan
	{
	public:
		DBScan();
		~DBScan();

		
		void initialize(std::vector<VertexData*>* data, std::map<int, Glycogen*>* glycogenMap, SpacePartitioning::Octree* octree, float eps = 0.06f, int minPts = 3);
		void run();
		std::map<int, GlycogenCluster*> getClusters();
		std::map<int, VertexData*>  getNoiseList();


	protected:

		void expandCluster(VertexData* node, GlycogenCluster* cluster, std::vector<uint32_t> neighbors);
		bool isVisited(VertexData* node);
		void setVisited(VertexData* node);
		void setAsNoise(VertexData* node);
		void addNodeToCluster(VertexData* node, GlycogenCluster* cluster);
		bool isInCluster(VertexData* node);

		float m_eps;
		int   m_minPts;

		std::vector<VertexData*>* m_nodesList;

		std::map<int, VertexData*> m_noiseList;
		std::map<int, VertexData*> m_visitedList;
		std::map<int, int>		   m_nodeClusterMap;

		std::map<int, GlycogenCluster*> m_clusters;
		std::map<int, Glycogen*>*		m_glycogenMap;

		SpacePartitioning::Octree* m_octree;
	};
}

#endif // DBSCAN_H_