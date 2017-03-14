#include "glycogencluster.h"
#include "mesh.h"
#include "octree.h"
#include "glycogenanalysismanager.h"



	//--------------------------------------------------------------------------------
	//
GlycogenAnalysisManager::GlycogenAnalysisManager(std::map<int, Glycogen*>* glycogenMap, std::vector<VertexData*>* glycogenList, SpacePartitioning::Octree* octree, std::vector< struct VertexData >* allvertices)
	{
		m_glycogenMap = glycogenMap;
		m_glycogenVertexDataList = glycogenList;
		m_glycogenOctree = octree;
		m_boutonOctree = 0;
		m_spineOctree = 0;
		m_verticesList = allvertices;
	}

	//--------------------------------------------------------------------------------
	//
	GlycogenAnalysisManager::~GlycogenAnalysisManager()
	{

	}

	//--------------------------------------------------------------------------------
	//
	void GlycogenAnalysisManager::setBoutonAndSpineOctrees(SpacePartitioning::Octree* boutonOct, SpacePartitioning::Octree* spineOct)
	{
		m_boutonOctree = boutonOct;
		m_spineOctree = spineOct;
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

	//--------------------------------------------------------------------------------
	//
	std::map<int, std::map<int, int>>*  GlycogenAnalysisManager::computeGlycogenMapping(bool boutons, bool spines, bool clusters)
	{
		if (clusters)
		{

			if (boutons && spines)
			{
				computeGlycogenClusterMappingToBoutonsAndSpines();
			}
			else if (boutons)
			{
				computeGlycogenClusterMappingToBoutons();
			}
			else if (spines)
			{
				computeGlycogenClusterMappingToSpines();
			}

		}
		else
		{
			if (boutons && spines)
			{
				computeGlycogenMappingToBoutonsAndSpines();
			}
			else if (boutons)
			{
				computeGlycogenMappingToBoutons();
			}
			else if (spines)
			{
				computeGlycogenMappingToSpines();
			}
		}

		return &m_objectIdToGlycogenMapping;
	}

	//--------------------------------------------------------------------------------
	//
	void GlycogenAnalysisManager::computeGlycogenMappingToBoutons()
	{
		//if we have an octree and glycogen list
		if (!m_boutonOctree || !m_glycogenVertexDataList)
			return;

		//clear previous mappings
		m_glycogenIdToObjectVertexIndexMapping.clear();
		m_objectIdToGlycogenMapping.clear();
		//loop on glycogen granules
		for (auto iter = m_glycogenVertexDataList->begin(); iter != m_glycogenVertexDataList->end(); iter++)
		{
			VertexData* glycogenVertex = (*iter);
			//get nearest bouton vertex to this granule
			int boutonVertexIndex = m_boutonOctree->findNeighbor(glycogenVertex->x(), glycogenVertex->y(), glycogenVertex->z(), 0.1f);
			//store mapping
			m_glycogenIdToObjectVertexIndexMapping[glycogenVertex->id()] = boutonVertexIndex;
			//get actual VertexData and use Object Id
			VertexData* boutonVertex = &(m_verticesList->at(boutonVertexIndex));
			//store mapping
			if (m_objectIdToGlycogenMapping.find(boutonVertex->id()) != m_objectIdToGlycogenMapping.end())
			{
				//object id is already in the list - update its list
				m_objectIdToGlycogenMapping.at(boutonVertex->id())[glycogenVertex->id()] = boutonVertex->index;
			}
			else
			{
				//object id is not already in the list - new list for it
				m_objectIdToGlycogenMapping[boutonVertex->id()] = std::map<int, int>();
				m_objectIdToGlycogenMapping.at(boutonVertex->id())[glycogenVertex->id()] = boutonVertex->index;
			}
		}
	}

	//--------------------------------------------------------------------------------
	//
	void GlycogenAnalysisManager::computeGlycogenMappingToSpines()
	{
		//if we have an octree and glycogen list
		if (!m_spineOctree || !m_glycogenVertexDataList)
			return;

		//clear previous mappings
		m_glycogenIdToObjectVertexIndexMapping.clear();
		m_objectIdToGlycogenMapping.clear();
		//loop on glycogen granules
		for (auto iter = m_glycogenVertexDataList->begin(); iter != m_glycogenVertexDataList->end(); iter++)
		{
			VertexData* glycogenVertex = (*iter);
			//get nearest spine vertex to this granule
			int spineVertexIndex = m_spineOctree->findNeighbor(glycogenVertex->x(), glycogenVertex->y(), glycogenVertex->z(), 0.1f);
			//store mapping
			m_glycogenIdToObjectVertexIndexMapping[glycogenVertex->id()] = spineVertexIndex;
			//get actual VertexData and use Object Id
			VertexData* spineVertex = &(m_verticesList->at(spineVertexIndex));
			//store mapping
			if (m_objectIdToGlycogenMapping.find(spineVertex->id()) != m_objectIdToGlycogenMapping.end())
			{
				//object id is already in the list - update its list
				m_objectIdToGlycogenMapping.at(spineVertex->id())[glycogenVertex->id()] = spineVertex->index;
			}
			else
			{
				//object id is not already in the list - new list for it
				m_objectIdToGlycogenMapping[spineVertex->id()] = std::map<int, int>();
				m_objectIdToGlycogenMapping.at(spineVertex->id())[glycogenVertex->id()] = spineVertex->index;
			}
		}
	}

	//--------------------------------------------------------------------------------
	//
	void GlycogenAnalysisManager::computeGlycogenMappingToBoutonsAndSpines()
	{
		//if we have an octree and glycogen list
		if (!m_spineOctree || !m_boutonOctree || !m_glycogenVertexDataList)
			return;

		//clear previous mappings
		m_glycogenIdToObjectVertexIndexMapping.clear();
		m_objectIdToGlycogenMapping.clear();
		//loop on glycogen granules
		for (auto iter = m_glycogenVertexDataList->begin(); iter != m_glycogenVertexDataList->end(); iter++)
		{
			VertexData* glycogenVertex = (*iter);
			//get nearest bouton vertex to this granule
			int boutonVertexIndex = m_boutonOctree->findNeighbor(glycogenVertex->x(), glycogenVertex->y(), glycogenVertex->z(), 0.1f);
			//get nearest spine vertex to this granule
			int spineVertexIndex = m_spineOctree->findNeighbor(glycogenVertex->x(), glycogenVertex->y(), glycogenVertex->z(), 0.1f);
			
			//get actual VertexData and use Object Id
			VertexData* spineVertex = &(m_verticesList->at(spineVertexIndex));
			VertexData* boutonVertex = &(m_verticesList->at(boutonVertexIndex));

			float d_spine = SpacePartitioning::L2Distance::compute(glycogenVertex->x(), glycogenVertex->y(), glycogenVertex->z(), spineVertex->x(), spineVertex->y(), spineVertex->z());
			float d_bouton = SpacePartitioning::L2Distance::compute(glycogenVertex->x(), glycogenVertex->y(), glycogenVertex->z(), boutonVertex->x(), boutonVertex->y(), boutonVertex->z());

			//get nearest of two
			int nearestVertexIndex = 0;
			VertexData* nearestVertex = 0;

			if (d_spine < d_bouton)
			{
				nearestVertexIndex = spineVertexIndex;
				nearestVertex = spineVertex;
			}
			else
			{
				nearestVertexIndex = boutonVertexIndex;
				nearestVertex = boutonVertex;
			}
			

			//store mapping
			m_glycogenIdToObjectVertexIndexMapping[glycogenVertex->id()] = nearestVertexIndex;
			

			//store mapping
			if (m_objectIdToGlycogenMapping.find(nearestVertex->id()) != m_objectIdToGlycogenMapping.end())
			{
				//object id is already in the list - update its list
				m_objectIdToGlycogenMapping.at(nearestVertex->id())[glycogenVertex->id()] = nearestVertex->index;
			}
			else
			{
				//object id is not already in the list - new list for it
				m_objectIdToGlycogenMapping[nearestVertex->id()] = std::map<int, int>();
				m_objectIdToGlycogenMapping.at(nearestVertex->id())[glycogenVertex->id()] = nearestVertex->index;
			}
		}

	}

	//--------------------------------------------------------------------------------
	//
	void GlycogenAnalysisManager::computeGlycogenClusterMappingToBoutons()
	{
		//if we have an octree and clusters
		if (!m_boutonOctree || m_clusterResults.size() == 0)
			return;

		//clear previous mappings
		m_glycogenIdToObjectVertexIndexMapping.clear();
		m_objectIdToGlycogenMapping.clear();
		//loop on clusters
		for (auto iter = m_clusterResults.begin(); iter != m_clusterResults.end(); iter++)
		{
			//get cluster centroid
			Clustering::GlycogenCluster* cluster = iter->second;
			QVector3D clusterNode =  cluster->getAvgNode();

			//get nearest bouton vertex to this centroid
			int boutonVertexIndex = m_boutonOctree->findNeighbor(clusterNode.x(), clusterNode.y(), clusterNode.z(), 0.1f);
			//store mapping
			m_glycogenIdToObjectVertexIndexMapping[cluster->getID()] = boutonVertexIndex;
			//get actual VertexData and use Object Id
			VertexData* boutonVertex = &(m_verticesList->at(boutonVertexIndex));
			//store mapping
			if (m_objectIdToGlycogenMapping.find(boutonVertex->id()) != m_objectIdToGlycogenMapping.end())
			{
				//object id is already in the list - update its list
				m_objectIdToGlycogenMapping.at(boutonVertex->id())[cluster->getID()] = boutonVertex->index;
			}
			else
			{
				//object id is not already in the list - new list for it
				m_objectIdToGlycogenMapping[boutonVertex->id()] = std::map<int, int>();
				m_objectIdToGlycogenMapping.at(boutonVertex->id())[cluster->getID()] = boutonVertex->index;
			}
		}
	}

	//--------------------------------------------------------------------------------
	//
	void GlycogenAnalysisManager::computeGlycogenClusterMappingToSpines()
	{
		//if we have an octree and clusters
		if (!m_spineOctree || m_clusterResults.size() == 0)
			return;

		//clear previous mappings
		m_glycogenIdToObjectVertexIndexMapping.clear();
		m_objectIdToGlycogenMapping.clear();
		//loop on clusters
		for (auto iter = m_clusterResults.begin(); iter != m_clusterResults.end(); iter++)
		{
			//get cluster centroid
			Clustering::GlycogenCluster* cluster = iter->second;
			QVector3D clusterNode = cluster->getAvgNode();

			//get nearest spine vertex to this centroid
			int spineVertexIndex = m_spineOctree->findNeighbor(clusterNode.x(), clusterNode.y(), clusterNode.z(), 0.1f);
			//store mapping
			m_glycogenIdToObjectVertexIndexMapping[cluster->getID()] = spineVertexIndex;
			//get actual VertexData and use Object Id
			VertexData* spineVertex = &(m_verticesList->at(spineVertexIndex));
			//store mapping
			if (m_objectIdToGlycogenMapping.find(spineVertex->id()) != m_objectIdToGlycogenMapping.end())
			{
				//object id is already in the list - update its list
				m_objectIdToGlycogenMapping.at(spineVertex->id())[cluster->getID()] = spineVertex->index;
			}
			else
			{
				//object id is not already in the list - new list for it
				m_objectIdToGlycogenMapping[spineVertex->id()] = std::map<int, int>();
				m_objectIdToGlycogenMapping.at(spineVertex->id())[cluster->getID()] = spineVertex->index;
			}
		}
	}

	//--------------------------------------------------------------------------------
	// TODO - 
	void GlycogenAnalysisManager::computeGlycogenClusterMappingToBoutonsAndSpines()
	{
		// if we both octrees and clusters
		if (!m_boutonOctree || !m_spineOctree || m_clusterResults.size() == 0)
			return;

		//clear previous mappings
		m_glycogenIdToObjectVertexIndexMapping.clear();
		//loop on clusters
		for (auto iter = m_clusterResults.begin(); iter != m_clusterResults.end(); iter++)
		{
			//get cluster centroid
			Clustering::GlycogenCluster* cluster = iter->second;
			QVector3D clusterNode = cluster->getAvgNode();

			//get nearest bouton vertex to this centroid
			int boutonVertexIndex = m_boutonOctree->findNeighbor(clusterNode.x(), clusterNode.y(), clusterNode.z(), 0.1f);
			//get nearest spine vertex to this centroid
			int spineVertexIndex = m_spineOctree->findNeighbor(clusterNode.x(), clusterNode.y(), clusterNode.z(), 0.1f);
			//find which is closer bouton or spine
			VertexData* boutonVertex = &(m_verticesList->at(boutonVertexIndex));
			VertexData* spineVertex = &(m_verticesList->at(spineVertexIndex));

			float d_spine = SpacePartitioning::L2Distance::compute(clusterNode.x(), clusterNode.y(), clusterNode.z(), spineVertex->x(), spineVertex->y(), spineVertex->z());
			float d_bouton = SpacePartitioning::L2Distance::compute(clusterNode.x(), clusterNode.y(), clusterNode.z(), boutonVertex->x(), boutonVertex->y(), boutonVertex->z());

			//get nearest of two
			int nearestVertexIndex = 0;
			VertexData* nearestVertex = 0;

			if (d_spine < d_bouton)
			{
				nearestVertexIndex = spineVertexIndex;
				nearestVertex = spineVertex;
			}
			else
			{
				nearestVertexIndex = boutonVertexIndex;
				nearestVertex = boutonVertex;
			}

			//store mapping
			m_glycogenIdToObjectVertexIndexMapping[cluster->getID()] = nearestVertexIndex;
			//store mapping
			if (m_objectIdToGlycogenMapping.find(nearestVertex->id()) != m_objectIdToGlycogenMapping.end())
			{
				//object id is already in the list - update its list
				m_objectIdToGlycogenMapping.at(nearestVertex->id())[cluster->getID()] = nearestVertex->index;
			}
			else
			{
				//object id is not already in the list - new list for it
				m_objectIdToGlycogenMapping[nearestVertex->id()] = std::map<int, int>();
				m_objectIdToGlycogenMapping.at(nearestVertex->id())[cluster->getID()] = nearestVertex->index;
			}
		}
	}