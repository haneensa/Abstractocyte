#include "glycogencluster.h"
#include "glycogenanalysismanager.h"



	//--------------------------------------------------------------------------------
	//
GlycogenAnalysisManager::GlycogenAnalysisManager(std::map<int, Glycogen*>* glycogenMap, std::vector<VertexData*>* glycogenList, SpacePartitioning::Octree* octree)
	{
		m_glycogenMap = glycogenMap;
		m_glycogenVertexDataList = glycogenList;
		m_glycogenOctree = octree;
	}

	//--------------------------------------------------------------------------------
	//
	GlycogenAnalysisManager::~GlycogenAnalysisManager()
	{

	}

	//--------------------------------------------------------------------------------
	//
	void GlycogenAnalysisManager::clear()
	{
		for (auto iter = m_clusterResults.begin(); iter != m_clusterResults.end(); iter++)
		{
			delete iter->second;
		}
		m_clusterResults.clear();
		//reset cluster ids
		Clustering::GlycogenCluster::clearIDs();
	}

	//--------------------------------------------------------------------------------
	//
	std::map<int, Clustering::GlycogenCluster*>* GlycogenAnalysisManager::runDBScan(float eps, int minPts)
	{

		clear();

		m_dbscan.initialize(m_glycogenVertexDataList, m_glycogenMap, m_glycogenOctree, eps, minPts);
		m_dbscan.run();

		m_clusterResults = m_dbscan.getClusters();

		//TODO get noise
		m_dbscan.getNoiseList();

		return &m_clusterResults;
	}