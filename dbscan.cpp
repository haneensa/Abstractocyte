#include "mesh.h"
#include "glycogen.h"
#include "glycogencluster.h"
#include "octree.h"

#include "dbscan.h"


namespace Clustering
{
	//--------------------------------------------------------------------------------
	//
	DBScan::DBScan()
	{
		m_glycogenMap = 0;
		m_nodesList = 0;
		m_octree = 0;
		m_minPts = 3;
		m_eps = 0.06f;
	}

	//--------------------------------------------------------------------------------
	//
	DBScan::~DBScan()
	{

	}

	//--------------------------------------------------------------------------------
	//
	void DBScan::initialize(std::vector<VertexData*>* data, std::map<int, Glycogen*>* glycogenMap, SpacePartitioning::Octree* octree, float eps, int minPts)
	{
		m_nodesList = data;
		m_eps = eps;
		m_minPts = minPts;
		m_noiseList.clear();
		m_visitedList.clear();
		m_clusters.clear();
		m_nodeClusterMap.clear();
		m_octree = octree;
		m_glycogenMap = glycogenMap;
	}

	//--------------------------------------------------------------------------------
	//
	void DBScan::run()
	{
		//DBSCAN(D, eps, MinPts) {
		//	C = 0
		//	for each point P in dataset D {
		//		if P is visited
		//			continue next point
		//		mark P as visited
		//		NeighborPts = regionQuery(P, eps)
		//		if sizeof(NeighborPts) < MinPts
		//			mark P as NOISE
		//		else {
		//			C = next cluster
		//			expandCluster(P, NeighborPts, C, eps, MinPts)
		//		}
		//	}
		//}
		for (auto i = m_nodesList->begin(); i != m_nodesList->end(); i++)
		{
			VertexData* node = (*i);
			if (isVisited(node))
				continue;
			setVisited(node);
			std::vector<uint32_t> results;
			m_octree->radiusNeighbors(node->x(), node->y(), node->z(), m_eps, results, node->index);
			if (results.size() < m_minPts)
			{
				setAsNoise(node);
			}
			else
			{
				GlycogenCluster* c = new GlycogenCluster();
				m_clusters[c->getID()] = c;
				expandCluster(node, c, results);
			}


		}

	}

	//--------------------------------------------------------------------------------
	//
	void DBScan::expandCluster(VertexData* node, GlycogenCluster* cluster, std::vector<uint32_t> neighbors)
	{
		//expandCluster(P, NeighborPts, C, eps, MinPts) {
		//	add P to cluster C
		//	for each point P' in NeighborPts { 
		//		if P' is not visited {
		//			mark P' as visited
		//			NeighborPts' = regionQuery(P', eps)
		//			if sizeof(NeighborPts') >= MinPts
		//				NeighborPts = NeighborPts joined with NeighborPts'
		//		}
		//		if P' is not yet member of any cluster
		//			add P' to cluster C
		//	}
		//}
		addNodeToCluster(node, cluster);

		for (auto i = neighbors.begin(); i != neighbors.end(); i++)
		{
			uint32_t idx = (*i);
			VertexData* neighborNode = (*m_nodesList)[idx];
			if (!isVisited(neighborNode))
			{
				setVisited(neighborNode);
				std::vector<uint32_t> results2;
				m_octree->radiusNeighbors(neighborNode->x(), neighborNode->y(), neighborNode->z(), m_eps, results2, neighborNode->index);
				if (results2.size() >= m_minPts)
					expandCluster(node, cluster, results2);
			}
			if (!isInCluster(neighborNode))
			{
				addNodeToCluster(neighborNode, cluster);
			}
		}


	}

	//--------------------------------------------------------------------------------
	//
	bool DBScan::isVisited(VertexData* node)
	{
		if (!node)
			return false;

		return m_visitedList.find(node->id()) != m_visitedList.end();
	}

	//--------------------------------------------------------------------------------
	//
	void DBScan::setVisited(VertexData* node)
	{
		if (!node)
			return;

		m_visitedList[node->id()] = node;
	}

	//--------------------------------------------------------------------------------
	//
	void DBScan::setAsNoise(VertexData* node)
	{
		if (!node)
			return;

		m_noiseList[node->id()] = node;
		Glycogen* gnode = m_glycogenMap->at(node->id());
		gnode->setClusterID(0);
	}

	//--------------------------------------------------------------------------------
	//
	void DBScan::addNodeToCluster(VertexData* node, GlycogenCluster* cluster)
	{
		Glycogen* gnode = m_glycogenMap->at(node->id());
		cluster->addNode(gnode);
		m_nodeClusterMap[node->id()] = cluster->getID();
	}

	//--------------------------------------------------------------------------------
	//
	bool DBScan::isInCluster(VertexData* node)
	{
		if (!node)
			return false;

		return m_nodeClusterMap.find(node->id()) != m_nodeClusterMap.end();
	}

	//--------------------------------------------------------------------------------
	//
	std::map<int, GlycogenCluster*> DBScan::getClusters()
	{

		return m_clusters;
	}

	//--------------------------------------------------------------------------------
	//
	std::map<int, VertexData*>  DBScan::getNoiseList()
	{
		return m_noiseList;
	}
}