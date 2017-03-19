#include "mesh.h"
#include "glycogen.h"
#include "glycogencluster.h"


namespace Clustering
{
	int GlycogenCluster::ID_COUNTER = 0;
	//--------------------------------------------------------------------------------
	//
	GlycogenCluster::GlycogenCluster()
	{
		GlycogenCluster::ID_COUNTER++;
		m_id = GlycogenCluster::ID_COUNTER;
		m_totalVolume = 0;
		m_sumNode.setX(0);
		m_sumNode.setY(0);
		m_sumNode.setZ(0);

		m_state = 1;
	}

	//--------------------------------------------------------------------------------
	//
	GlycogenCluster::~GlycogenCluster()
	{
		for (auto iter = m_glycogenMap.begin(); iter != m_glycogenMap.end(); iter++)
		{
			(*iter).second->setClusterID(0);
		}
	}

	//--------------------------------------------------------------------------------
	//
	void GlycogenCluster::clearIDs()
	{
		GlycogenCluster::ID_COUNTER = 0;
	}

	//--------------------------------------------------------------------------------
	//
	int GlycogenCluster::getID()
	{
		return m_id;
	}

	//--------------------------------------------------------------------------------
	//
	int  GlycogenCluster::getState()
	{
		return m_state;
	}

	//--------------------------------------------------------------------------------
	//
	void GlycogenCluster::setState(int state)
	{
		if (state > 2 || state < 0)
			return;

		m_state = state;
		for (auto iter = m_glycogenMap.begin(); iter != m_glycogenMap.end(); iter++)
		{
			iter->second->setState(state);
		}


	}

	//--------------------------------------------------------------------------------
	//
	void GlycogenCluster::addNode(Glycogen* glycogen)
	{
		//if glycogne is null or glycogen already in cluster.. exit
		if (!glycogen || m_glycogenMap.find(glycogen->getID()) != m_glycogenMap.end())
			return;

		m_totalVolume += glycogen->getVolume();
		glycogen->setClusterID(m_id);

		m_sumNode.setX(m_sumNode.x() + glycogen->x());
		m_sumNode.setY(m_sumNode.y() + glycogen->y());
		m_sumNode.setZ(m_sumNode.z() + glycogen->z());

		m_glycogenMap[glycogen->getID()] = glycogen;
	}

	//--------------------------------------------------------------------------------
	//
	QVector3D GlycogenCluster::getAvgNode()
	{
		float n = m_glycogenMap.size();
		QVector3D avgNode(m_sumNode.x() / n, m_sumNode.y() / n, m_sumNode.z() / n);
		return avgNode;
	}

	//--------------------------------------------------------------------------------
	//
	bool GlycogenCluster::contains(int id)
	{
		auto found = m_glycogenMap.find(id);
		return found != m_glycogenMap.end();
	}

	//--------------------------------------------------------------------------------
	//
	int  GlycogenCluster::getClusterSize()
	{
		return m_glycogenMap.size();
	}

	//--------------------------------------------------------------------------------
	//
	float GlycogenCluster::getTotalVolume()
	{
		return m_totalVolume;
	}
}