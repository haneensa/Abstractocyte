#include "glycogencluster.h"
#include "mesh.h"
#include "octree.h"
#include "spatialhash3d.h"
#include "glycogenanalysismanager.h"



	//--------------------------------------------------------------------------------
	//
GlycogenAnalysisManager::GlycogenAnalysisManager(std::map<int, Glycogen*>* glycogenMap, std::vector<VertexData*>* glycogenList, SpacePartitioning::Octree* octree, std::vector< struct VertexData >* allvertices)
	{
		m_glycogenMap = glycogenMap;
		m_glycogenVertexDataList = glycogenList;
		m_glycogenOctree = octree;
		m_boutonHash = 0;
		m_spineHash = 0;
		m_current_max_glycogen_volume = 0;
		m_verticesList = allvertices;
	}

	//--------------------------------------------------------------------------------
	//
	GlycogenAnalysisManager::~GlycogenAnalysisManager()
	{

	}

	//--------------------------------------------------------------------------------
	//
	void GlycogenAnalysisManager::setBoutonAndSpineOctrees(SpacePartitioning::SpatialHash3D* boutonOct, SpacePartitioning::SpatialHash3D* spineOct)
	{
		m_boutonHash = boutonOct;
		m_spineHash = spineOct;
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
		if (!m_boutonHash || !m_glycogenVertexDataList)
			return;

		//clear previous mappings
		clearMapping();
		//loop on glycogen granules
		for (auto iter = m_glycogenVertexDataList->begin(); iter != m_glycogenVertexDataList->end(); iter++)
		{
			VertexData* glycogenVertex = (*iter);
			//get nearest bouton vertex to this granule
			//int boutonVertexIndex = m_boutonOctree->findNeighbor(glycogenVertex->x(), glycogenVertex->y(), glycogenVertex->z());
			VertexData* boutonVertex = m_boutonHash->getNeighbor(glycogenVertex->x(), glycogenVertex->y(), glycogenVertex->z());
			//store mapping
			m_glycogenIdToObjectVertexIndexMapping[glycogenVertex->id()] = boutonVertex->index;
			//get actual VertexData and use Object Id
			//VertexData* boutonVertex = &(m_verticesList->at(boutonVertexIndex));
			//store mapping
			if (m_objectIdToGlycogenMapping.find(boutonVertex->id()) != m_objectIdToGlycogenMapping.end())
			{
				//object id is already in the list - update its list
				m_objectIdToGlycogenMapping.at(boutonVertex->id())[glycogenVertex->id()] = boutonVertex->index;
				m_objectIdGlycogenVolumeMapped[boutonVertex->id()] += glycogenVertex->skeleton_vertex.y();
			}
			else
			{
				//object id is not already in the list - new list for it
				m_objectIdToGlycogenMapping[boutonVertex->id()] = std::map<int, int>();
				m_objectIdToGlycogenMapping.at(boutonVertex->id())[glycogenVertex->id()] = boutonVertex->index;
				m_objectIdGlycogenVolumeMapped[boutonVertex->id()] = glycogenVertex->skeleton_vertex.y();
			}
			float currentVolumeValue = m_objectIdGlycogenVolumeMapped.at(boutonVertex->id());
			if (m_current_max_glycogen_volume < currentVolumeValue)
			{
				m_current_max_glycogen_volume = currentVolumeValue;
			}
		}
	}

	//--------------------------------------------------------------------------------
	//
	void GlycogenAnalysisManager::computeGlycogenMappingToSpines()
	{
		//if we have an octree and glycogen list
		if (!m_spineHash || !m_glycogenVertexDataList)
			return;

		//clear previous mappings
		clearMapping();
		//loop on glycogen granules
		for (auto iter = m_glycogenVertexDataList->begin(); iter != m_glycogenVertexDataList->end(); iter++)
		{
			VertexData* glycogenVertex = (*iter);
			//get nearest spine vertex to this granule
			//int spineVertexIndex = m_spineOctree->findNeighbor(glycogenVertex->x(), glycogenVertex->y(), glycogenVertex->z());
			VertexData* spineVertex = m_spineHash->getNeighbor(glycogenVertex->x(), glycogenVertex->y(), glycogenVertex->z());
			//store mapping
			m_glycogenIdToObjectVertexIndexMapping[glycogenVertex->id()] = spineVertex->index;
			//get actual VertexData and use Object Id
			//VertexData* spineVertex = &(m_verticesList->at(spineVertexIndex));
			//store mapping
			if (m_objectIdToGlycogenMapping.find(spineVertex->id()) != m_objectIdToGlycogenMapping.end())
			{
				//object id is already in the list - update its list
				m_objectIdToGlycogenMapping.at(spineVertex->id())[glycogenVertex->id()] = spineVertex->index;
				m_objectIdGlycogenVolumeMapped[spineVertex->id()] += glycogenVertex->skeleton_vertex.y();
			}
			else
			{
				//object id is not already in the list - new list for it
				m_objectIdToGlycogenMapping[spineVertex->id()] = std::map<int, int>();
				m_objectIdToGlycogenMapping.at(spineVertex->id())[glycogenVertex->id()] = spineVertex->index;
				m_objectIdGlycogenVolumeMapped[spineVertex->id()] = glycogenVertex->skeleton_vertex.y();
			}
			float currentVolumeValue = m_objectIdGlycogenVolumeMapped.at(spineVertex->id());
			if (m_current_max_glycogen_volume < currentVolumeValue)
			{
				m_current_max_glycogen_volume = currentVolumeValue;
			}
		}
	}

	//--------------------------------------------------------------------------------
	//
	void GlycogenAnalysisManager::computeGlycogenMappingToBoutonsAndSpines()
	{
		//if we have an octree and glycogen list
		if (!m_spineHash || !m_boutonHash || !m_glycogenVertexDataList)
			return;

		//clear previous mappings
		clearMapping();
		//loop on glycogen granules
		for (auto iter = m_glycogenVertexDataList->begin(); iter != m_glycogenVertexDataList->end(); iter++)
		{
			VertexData* glycogenVertex = (*iter);
			//get nearest bouton vertex to this granule
			//int boutonVertexIndex = m_boutonOctree->findNeighbor(glycogenVertex->x(), glycogenVertex->y(), glycogenVertex->z());
			VertexData* boutonVertex = m_boutonHash->getNeighbor(glycogenVertex->x(), glycogenVertex->y(), glycogenVertex->z());
			//get nearest spine vertex to this granule
			//int spineVertexIndex = m_spineOctree->findNeighbor(glycogenVertex->x(), glycogenVertex->y(), glycogenVertex->z());
			VertexData* spineVertex = m_spineHash->getNeighbor(glycogenVertex->x(), glycogenVertex->y(), glycogenVertex->z());
			//get actual VertexData and use Object Id
			//VertexData* spineVertex = &(m_verticesList->at(spineVertexIndex));
			//VertexData* boutonVertex = &(m_verticesList->at(boutonVertexIndex));

			float d_spine = SpacePartitioning::L2Distance::compute(glycogenVertex->x(), glycogenVertex->y(), glycogenVertex->z(), spineVertex->x(), spineVertex->y(), spineVertex->z());
			float d_bouton = SpacePartitioning::L2Distance::compute(glycogenVertex->x(), glycogenVertex->y(), glycogenVertex->z(), boutonVertex->x(), boutonVertex->y(), boutonVertex->z());

			//get nearest of two
			int nearestVertexIndex = 0;
			VertexData* nearestVertex = 0;

			if (d_spine < d_bouton)
			{
				nearestVertexIndex = spineVertex->index;
				nearestVertex = spineVertex;
			}
			else
			{
				nearestVertexIndex = boutonVertex->index;
				nearestVertex = boutonVertex;
			}


			//store mapping
			m_glycogenIdToObjectVertexIndexMapping[glycogenVertex->id()] = nearestVertexIndex;


			//store mapping
			if (m_objectIdToGlycogenMapping.find(nearestVertex->id()) != m_objectIdToGlycogenMapping.end())
			{
				//object id is already in the list - update its list
				m_objectIdToGlycogenMapping.at(nearestVertex->id())[glycogenVertex->id()] = nearestVertex->index;
				m_objectIdGlycogenVolumeMapped[nearestVertex->id()] += glycogenVertex->skeleton_vertex.y();
			}
			else
			{
				//object id is not already in the list - new list for it
				m_objectIdToGlycogenMapping[nearestVertex->id()] = std::map<int, int>();
				m_objectIdToGlycogenMapping.at(nearestVertex->id())[glycogenVertex->id()] = nearestVertex->index;
				m_objectIdGlycogenVolumeMapped[nearestVertex->id()] = glycogenVertex->skeleton_vertex.y();
			}
			float currentVolumeValue = m_objectIdGlycogenVolumeMapped.at(nearestVertex->id());
			if (m_current_max_glycogen_volume < currentVolumeValue)
			{
				m_current_max_glycogen_volume = currentVolumeValue;
			}
		}

	}

	//--------------------------------------------------------------------------------
	//
	void GlycogenAnalysisManager::computeGlycogenClusterMappingToBoutons()
	{
		//if we have an octree and clusters
		if (!m_boutonHash || m_clusterResults.size() == 0)
			return;

		//clear previous mappings
		clearMapping();
		//loop on clusters
		for (auto iter = m_clusterResults.begin(); iter != m_clusterResults.end(); iter++)
		{
			//get cluster centroid
			Clustering::GlycogenCluster* cluster = iter->second;
			QVector3D clusterNode = cluster->getAvgNode();

			//get nearest bouton vertex to this centroid
			//int boutonVertexIndex = m_boutonOctree->findNeighbor(clusterNode.x(), clusterNode.y(), clusterNode.z());
			VertexData* boutonVertex = m_boutonHash->getNeighbor(clusterNode.x(), clusterNode.y(), clusterNode.z());
			
			if (!boutonVertex) //TODO: get nearest no matter what
			{
				qDebug() << "Cluster didn't find closest bouton. Cluster ID: " << cluster->getID();
				continue;
			}
			
			//store mapping
			m_glycogenIdToObjectVertexIndexMapping[cluster->getID()] = boutonVertex->index;
			//get actual VertexData and use Object Id
			//VertexData* boutonVertex = &(m_verticesList->at(boutonVertex->index));
			//store mapping
			if (m_objectIdToGlycogenMapping.find(boutonVertex->id()) != m_objectIdToGlycogenMapping.end())
			{
				//object id is already in the list - update its list
				m_objectIdToGlycogenMapping.at(boutonVertex->id())[cluster->getID()] = boutonVertex->index;
				m_objectIdGlycogenVolumeMapped[boutonVertex->id()] += cluster->getTotalVolume();
			}
			else
			{
				//object id is not already in the list - new list for it
				m_objectIdToGlycogenMapping[boutonVertex->id()] = std::map<int, int>();
				m_objectIdToGlycogenMapping.at(boutonVertex->id())[cluster->getID()] = boutonVertex->index;
				m_objectIdGlycogenVolumeMapped[boutonVertex->id()] = cluster->getTotalVolume();
			}
			float currentVolumeValue = m_objectIdGlycogenVolumeMapped.at(boutonVertex->id());
			if (m_current_max_glycogen_volume < currentVolumeValue)
			{
				m_current_max_glycogen_volume = currentVolumeValue;
			}
		}
	}

	//--------------------------------------------------------------------------------
	//
	void GlycogenAnalysisManager::computeGlycogenClusterMappingToSpines()
	{
		//if we have an octree and clusters
		if (!m_spineHash || m_clusterResults.size() == 0)
			return;

		//clear previous mappings
		clearMapping();
		//loop on clusters
		for (auto iter = m_clusterResults.begin(); iter != m_clusterResults.end(); iter++)
		{
			//get cluster centroid
			Clustering::GlycogenCluster* cluster = iter->second;
			QVector3D clusterNode = cluster->getAvgNode();

			//get nearest spine vertex to this centroid
			//int spineVertexIndex = m_spineOctree->findNeighbor(clusterNode.x(), clusterNode.y(), clusterNode.z());
			VertexData* spineVertex = m_spineHash->getNeighbor(clusterNode.x(), clusterNode.y(), clusterNode.z());

			if (!spineVertex) //TODO: get nearest no matter what
			{
				qDebug() << "Cluster didn't find closest spine. Cluster ID: " << cluster->getID();
				continue;
			}
			//store mapping
			m_glycogenIdToObjectVertexIndexMapping[cluster->getID()] = spineVertex->index;
			//get actual VertexData and use Object Id
			//VertexData* spineVertex = &(m_verticesList->at(spineVertex->index));
			//store mapping
			if (m_objectIdToGlycogenMapping.find(spineVertex->id()) != m_objectIdToGlycogenMapping.end())
			{
				//object id is already in the list - update its list
				m_objectIdToGlycogenMapping.at(spineVertex->id())[cluster->getID()] = spineVertex->index;
				m_objectIdGlycogenVolumeMapped[spineVertex->id()] += cluster->getTotalVolume();
			}
			else
			{
				//object id is not already in the list - new list for it
				m_objectIdToGlycogenMapping[spineVertex->id()] = std::map<int, int>();
				m_objectIdToGlycogenMapping.at(spineVertex->id())[cluster->getID()] = spineVertex->index;
				m_objectIdGlycogenVolumeMapped[spineVertex->id()] = cluster->getTotalVolume();
			}
			float currentVolumeValue = m_objectIdGlycogenVolumeMapped.at(spineVertex->id());
			if (m_current_max_glycogen_volume < currentVolumeValue)
			{
				m_current_max_glycogen_volume = currentVolumeValue;
			}
		}
	}

	//--------------------------------------------------------------------------------
	// 
	void GlycogenAnalysisManager::computeGlycogenClusterMappingToBoutonsAndSpines()
	{
		// if we both octrees and clusters
		if (!m_boutonHash || !m_spineHash || m_clusterResults.size() == 0)
			return;

		//clear previous mappings
		clearMapping();
		//loop on clusters
		for (auto iter = m_clusterResults.begin(); iter != m_clusterResults.end(); iter++)
		{
			//get cluster centroid
			Clustering::GlycogenCluster* cluster = iter->second;
			QVector3D clusterNode = cluster->getAvgNode();

			//get nearest bouton vertex to this centroid
			//int boutonVertexIndex = m_boutonOctree->findNeighbor(clusterNode.x(), clusterNode.y(), clusterNode.z());
			VertexData* boutonVertex = m_boutonHash->getNeighbor(clusterNode.x(), clusterNode.y(), clusterNode.z());
			//get nearest spine vertex to this centroid
			//int spineVertexIndex = m_spineOctree->findNeighbor(clusterNode.x(), clusterNode.y(), clusterNode.z());
			VertexData* spineVertex = m_spineHash->getNeighbor(clusterNode.x(), clusterNode.y(), clusterNode.z());
			//find which is closer bouton or spine
			//VertexData* boutonVertex = &(m_verticesList->at(boutonVertex->index));
			//VertexData* spineVertex = &(m_verticesList->at(spineVertex->index));

			//get nearest of two
			int nearestVertexIndex = 0;
			VertexData* nearestVertex = 0;

			if (boutonVertex && spineVertex)
			{
				float d_spine = SpacePartitioning::L2Distance::compute(clusterNode.x(), clusterNode.y(), clusterNode.z(), spineVertex->x(), spineVertex->y(), spineVertex->z());
				float d_bouton = SpacePartitioning::L2Distance::compute(clusterNode.x(), clusterNode.y(), clusterNode.z(), boutonVertex->x(), boutonVertex->y(), boutonVertex->z());

				if (d_spine < d_bouton)
				{
					nearestVertexIndex = spineVertex->index;
					nearestVertex = spineVertex;
				}
				else
				{
					nearestVertexIndex = boutonVertex->index;
					nearestVertex = boutonVertex;
				}
			}
			else if (boutonVertex)
			{
				nearestVertexIndex = boutonVertex->index;
				nearestVertex = boutonVertex;
			}
			else if (spineVertex)
			{
				nearestVertexIndex = spineVertex->index;
				nearestVertex = spineVertex;
			}
			else
			{
				qDebug() << "Cluster didn't find closest spine or bouton. Cluster ID: " << cluster->getID();
				continue;
			}

			//store mapping
			m_glycogenIdToObjectVertexIndexMapping[cluster->getID()] = nearestVertexIndex;
			//store mapping
			if (m_objectIdToGlycogenMapping.find(nearestVertex->id()) != m_objectIdToGlycogenMapping.end())
			{
				//object id is already in the list - update its list
				m_objectIdToGlycogenMapping.at(nearestVertex->id())[cluster->getID()] = nearestVertex->index;
				m_objectIdGlycogenVolumeMapped[nearestVertex->id()] += cluster->getTotalVolume();
			}
			else
			{
				//object id is not already in the list - new list for it
				m_objectIdToGlycogenMapping[nearestVertex->id()] = std::map<int, int>();
				m_objectIdToGlycogenMapping.at(nearestVertex->id())[cluster->getID()] = nearestVertex->index;
				m_objectIdGlycogenVolumeMapped[nearestVertex->id()] = cluster->getTotalVolume();
			}
			float currentVolumeValue = m_objectIdGlycogenVolumeMapped.at(nearestVertex->id());
			if (m_current_max_glycogen_volume < currentVolumeValue)
			{
				m_current_max_glycogen_volume = currentVolumeValue;
			}
		}
	}

	//--------------------------------------------------------------------------------
	// 
	void GlycogenAnalysisManager::clearMapping()
	{
		m_current_max_glycogen_volume = 0;
		m_objectIdGlycogenVolumeMapped.clear();
		m_glycogenIdToObjectVertexIndexMapping.clear();
		m_objectIdToGlycogenMapping.clear();
	}