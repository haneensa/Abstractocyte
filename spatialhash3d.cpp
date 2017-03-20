#include "mesh.h"
#include "octree.h"
#include "spatialhash3d.h"



//----------------------------------------------------------------------------
//
SpacePartitioning::SpatialHash3D::SpatialHash3D()
{
	m_size[0] = 0;
	m_size[1] = 0;
	m_size[2] = 0;
	m_totalSize = 0;
	//m_vectorList = 0;
}

//----------------------------------------------------------------------------
//
SpacePartitioning::SpatialHash3D::SpatialHash3D(int sizeX, int sizeY, int sizeZ)
{
	m_size[0] = sizeX;
	m_size[1] = sizeY;
	m_size[2] = sizeZ;
	m_totalSize = sizeX * sizeY * sizeZ;
	m_vectorList.reserve(m_totalSize);
	m_vectorList.resize(m_totalSize);
	//m_vectorList = new std::vector<VertexData*>*[m_totalSize];

}

//----------------------------------------------------------------------------
//
SpacePartitioning::SpatialHash3D::~SpatialHash3D()
{
	//delete m_data;
}

//----------------------------------------------------------------------------
//
void SpacePartitioning::SpatialHash3D::setSize(int sizeX, int sizeY, int sizeZ)
{

	m_size[0] = sizeX;
	m_size[1] = sizeY;
	m_size[2] = sizeZ;
	m_totalSize = sizeX * sizeY * sizeZ;
	reset();

	
}

//----------------------------------------------------------------------------
//
void SpacePartitioning::SpatialHash3D::reset()
{
	//delete m_data;
	//m_vectorList = new T[m_size[0] * m_size[1] * m_size[2]];
	//memset(m_data, 0, sizeof(T)* m_size[0] * m_size[1] * m_size[2]);
	//for (int i = 0; i < m_totalSize; i++)
	//{
	//	if (m_vectorList[i])
	//		delete m_vectorList[i];
	//}
	//delete m_vectorList;
	m_vectorList.clear();
	m_vectorList.reserve(m_totalSize);
	m_vectorList.resize(m_totalSize);
	//m_vectorList = new std::vector<VertexData*>*[m_size[0] * m_size[1] * m_size[2]];
	//memset(m_vectorList, 0, sizeof(std::vector<VertexData*>*)*m_size[0] * m_size[1] * m_size[2]);
}

//----------------------------------------------------------------------------
//
void SpacePartitioning::SpatialHash3D::addNormalizedPoint(float nX, float nY, float nZ, VertexData* value)
{
	int X = 0;
	int Y = 1;
	int Z = 2;

	int gX = (int)(nX * (m_size[X] - 1));
	int gY = (int)(nY * (m_size[Y] - 1));
	int gZ = (int)(nZ * (m_size[Z] - 1));

	int idx = gX + gY * m_size[X] + gZ * m_size[Y] * m_size[Z];

	//if (!m_vectorList[idx]) 
	//{
	//	m_vectorList[idx] = new std::vector<VertexData*>();
	//}
	//m_vectorList[idx]->push_back(value);
	m_vectorList[idx].push_back(value);
}

//-----------------------------------------------------------------------------
//
//std::vector<VertexData*>** 
std::vector<std::vector<VertexData*>>* SpacePartitioning::SpatialHash3D::getData()
{
	return &m_vectorList;
}

//-----------------------------------------------------------------------------
//
std::vector<VertexData*>* SpacePartitioning::SpatialHash3D::getClosestNeighbors(float nX, float nY, float nZ)
{
	int X = 0;
	int Y = 1;
	int Z = 2;

	int gX = (int)(nX * (m_size[X] - 1));
	int gY = (int)(nY * (m_size[Y] - 1));
	int gZ = (int)(nZ * (m_size[Z] - 1));

	int idx = gX + gY * m_size[X] + gZ * m_size[Y] * m_size[Z];

	//TODO get neighboring cells as well
	//int gx_1 = gX + 1; int gx__1 = gX - 1;
	//int gy_1 = gY + 1; int gy__1 = gY - 1;
	//int gz_1 = gZ + 1; int gz__1 = gZ - 1;
	
	
	//std::vector<VertexData*> copy(m_vectorList[idx]);

	return &m_vectorList[idx];

}


//-----------------------------------------------------------------------------
//
VertexData* SpacePartitioning::SpatialHash3D::getNeighbor(float nX, float nY, float nZ)
{
	int X = 0;
	int Y = 1;
	int Z = 2;

	int gX = (int)(nX * (m_size[X] - 1));
	int gY = (int)(nY * (m_size[Y] - 1));
	int gZ = (int)(nZ * (m_size[Z] - 1));

	//int idx = gX + gY * m_size[X] + gZ * m_size[Y] * m_size[Z];

	float distance2 = 99999999;
	VertexData* closest = 0;

	

	//for (int i = 1; i )
	//neighbors
	int gx_1[3] = { gX, gX + 1, gX - 1 };
	int gy_1[3] = { gY, gY + 1, gY - 1 };
	int gz_1[3] = { gZ, gZ + 1, gZ - 1 };

	for ( int z = 0; z < 3; z++)
	{
		if (gz_1[z] < 0 || gz_1[z] >= m_size[Z]) //exceeds boundaries ->skip
			continue;
		for (int y = 0; y < 3; y++)
		{
			if (gy_1[y] < 0 || gy_1[y] >= m_size[Y]) //exceeds boundaries ->skip
				continue;

			for (int x = 0; x < 3; x++)
			{
				if (gx_1[x] < 0 || gx_1[x] >= m_size[X]) //exceeds boundaries ->skip
					continue;

				//get index of cell
				int idx = gx_1[x] + gy_1[y] * m_size[X] + gz_1[z] * m_size[Y] * m_size[Z];
				//if (!m_vectorList[idx])
				//	continue;
				//for (auto iter = m_vectorList[idx]->begin(); iter != m_vectorList[idx]->end(); iter++)
				for (auto iter = m_vectorList[idx].begin(); iter != m_vectorList[idx].end(); iter++)
				{
					float currentDistance = L2Distance::compute(nX, nY, nZ, (*iter)->x(), (*iter)->y(), (*iter)->z());
					if (currentDistance < distance2)
					{
						closest = (*iter);
						distance2 = currentDistance;
					}
				}
			}
		}
	}

	if (!closest)
	{

		int gx_2[2] = { gX + 2, gX - 2 };
		int gy_2[2] = { gY + 2, gY - 2 };
		int gz_2[2] = { gZ + 2, gZ - 2 };

		for (int x = 0; x < 2; x++)
		{
			if (gx_2[x] < 0 || gx_2[x] >= m_size[X]) //exceeds boundaries ->skip
				continue;

			for (int z = 0; z < 3; z++)
			{
				if (gz_1[z] < 0 || gz_1[z] >= m_size[Z]) //exceeds boundaries ->skip
					continue;
				for (int y = 0; y < 3; y++)
				{
					if (gy_1[y] < 0 || gy_1[y] >= m_size[Y]) //exceeds boundaries ->skip
						continue;

					int idx = gx_2[x] + gy_1[y] * m_size[X] + gz_1[z] * m_size[Y] * m_size[Z];

					for (auto iter = m_vectorList[idx].begin(); iter != m_vectorList[idx].end(); iter++)
					{
						float currentDistance = L2Distance::compute(nX, nY, nZ, (*iter)->x(), (*iter)->y(), (*iter)->z());
						if (currentDistance < distance2)
						{
							closest = (*iter);
							distance2 = currentDistance;
						}
					}
				}
			}

		}

		for (int y = 0; y < 2; y++)
		{
			if (gy_2[y] < 0 || gy_2[y] >= m_size[Y]) //exceeds boundaries ->skip
				continue;

			for (int z = 0; z < 3; z++)
			{
				if (gz_1[z] < 0 || gz_1[z] >= m_size[Z]) //exceeds boundaries ->skip
					continue;
				for (int x = 0; x < 3; x++)
				{
					if (gx_1[x] < 0 || gy_1[x] >= m_size[X]) //exceeds boundaries ->skip
						continue;

					int idx = gx_1[x] + gy_2[y] * m_size[X] + gz_1[z] * m_size[Y] * m_size[Z];

					for (auto iter = m_vectorList[idx].begin(); iter != m_vectorList[idx].end(); iter++)
					{
						float currentDistance = L2Distance::compute(nX, nY, nZ, (*iter)->x(), (*iter)->y(), (*iter)->z());
						if (currentDistance < distance2)
						{
							closest = (*iter);
							distance2 = currentDistance;
						}
					}
				}
			}

		}


		for (int z = 0; z < 2; z++)
		{
			if (gz_2[z] < 0 || gz_2[z] >= m_size[Z]) //exceeds boundaries ->skip
				continue;

			for (int x = 0; x < 3; x++)
			{
				if (gx_1[z] < 0 || gx_1[x] >= m_size[X]) //exceeds boundaries ->skip
					continue;
				for (int y = 0; y < 3; y++)
				{
					if (gy_1[y] < 0 || gy_1[y] >= m_size[Y]) //exceeds boundaries ->skip
						continue;

					int idx = gx_1[x] + gy_1[y] * m_size[X] + gz_2[z] * m_size[Y] * m_size[Z];

					for (auto iter = m_vectorList[idx].begin(); iter != m_vectorList[idx].end(); iter++)
					{
						float currentDistance = L2Distance::compute(nX, nY, nZ, (*iter)->x(), (*iter)->y(), (*iter)->z());
						if (currentDistance < distance2)
						{
							closest = (*iter);
							distance2 = currentDistance;
						}
					}
				}
			}

		}
	}
	return closest;
}

