#include "mesh.h"
#include "octree.h"
#include "spatialhash3d.h"

#define MAX_RING_SIZE 10

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

	int idx = gX + gY * m_size[X] + gZ * m_size[Y] * m_size[X];

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

	int idx = gX + gY * m_size[X] + gZ * m_size[Y] * m_size[X];

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
				int idx = gx_1[x] + gy_1[y] * m_size[X] + gz_1[z] * m_size[Y] * m_size[X];
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

					int idx = gx_2[x] + gy_1[y] * m_size[X] + gz_1[z] * m_size[Y] * m_size[X];

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

					int idx = gx_1[x] + gy_2[y] * m_size[X] + gz_1[z] * m_size[Y] * m_size[X];

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

					int idx = gx_1[x] + gy_1[y] * m_size[X] + gz_2[z] * m_size[Y] * m_size[X];

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

//-----------------------------------------------------------------------------
//
VertexData* SpacePartitioning::SpatialHash3D::getNeighbor2(float nX, float nY, float nZ)
{
	int X = 0;
	int Y = 1;
	int Z = 2;

	//grid indices
	int gX = (int)(nX * (m_size[X] - 1));
	int gY = (int)(nY * (m_size[Y] - 1));
	int gZ = (int)(nZ * (m_size[Z] - 1));

	//int idx = gX + gY * m_size[X] + gZ * m_size[Y] * m_size[Z];

	float distance2 = 99999999; // will hold the shortest distance squared
	VertexData* closest = 0;

	int idx = gX + gY * m_size[X] + gZ * m_size[Y] * m_size[X];

	//check current cells first
	for (auto iter = m_vectorList[idx].begin(); iter != m_vectorList[idx].end(); iter++)
	{
		float currentDistance = L2Distance::compute(nX, nY, nZ, (*iter)->x(), (*iter)->y(), (*iter)->z());
		if (currentDistance < distance2)
		{
			closest = (*iter);
			distance2 = currentDistance;
		}
	}

	//loop on 3d rings 
	for (int r = 1; r < MAX_RING_SIZE; r++)
	{

		//scan x
		for (int x = gX - r; x < gX + r + 1; x++)
		{
			int y1 = gY - r;
			int y2 = gY + r;

			//go through  z+r <-- z --> z-r (scan z)
			for (int z = gZ - r; z < gZ + r + 1; z++)
			{
				VertexData* result = 0;
				float currentDistance = findClosesVertexInCell(x, y1, z, result, nX, nY, nZ);
				VertexData* result2 = 0;
				float currentDistance2 = 99999999;
				if (y1 != y2)
					currentDistance2 = findClosesVertexInCell(x, y2, z, result2, nX, nY, nZ);
				//update shortest distance vertex
				if (currentDistance > currentDistance2)
				{
					currentDistance = currentDistance2;
					result = result2;
				}
				if (currentDistance < distance2)
				{
					closest = result;
					distance2 = currentDistance;
				}
			}

		}

		//scan y
		for (int y = gY - r; y < gY + r; y++)
		{
			int x1 = gX - r;
			int x2 = gX + r;

			//go through  z+r <-- z --> z-r (scan z)
			for (int z = gZ - r; z < gZ + r + 1; z++)
			{
				VertexData* result = 0;
				float currentDistance = findClosesVertexInCell(x1, y, z, result, nX, nY, nZ);
				VertexData* result2 = 0;
				float currentDistance2 = findClosesVertexInCell(x2, y, z, result2, nX, nY, nZ);
				//update shortest distance vertex
				if (currentDistance > currentDistance2)
				{
					currentDistance = currentDistance2;
					result = result2;
				}
				if (currentDistance < distance2)
				{
					closest = result;
					distance2 = currentDistance;
				}
			}
		}

		//xy-faces on front and back remains
		int z1 = gZ - r;
		int z2 = gZ + r;
		for (int r2 = 0; r2 < r; r2++)
		{
			//scan x
			for (int x = gX - r2; x < gX + r2 + 1; x++)
			{
				int y1 = gY - r2;
				int y2 = gY + r2;

				VertexData* result = 0;
				float currentDistance = findClosesVertexInCell(x, y1, z1, result, nX, nY, nZ);
				VertexData* result2 = 0;
				float currentDistance2 = findClosesVertexInCell(x, y1, z2, result2, nX, nY, nZ);
				VertexData* result3 = 0;
				float currentDistance3 = findClosesVertexInCell(x, y2, z1, result3, nX, nY, nZ);
				VertexData* result4 = 0;
				float currentDistance4 = findClosesVertexInCell(x, y2, z2, result4, nX, nY, nZ);
				//update shortest distance vertex
				if (currentDistance > currentDistance2)
				{
					currentDistance = currentDistance2;
					result = result2;
				}
				if (currentDistance3 > currentDistance4)
				{
					currentDistance3 = currentDistance4;
					result3 = result4;
				}
				if (currentDistance > currentDistance3)
				{
					currentDistance = currentDistance3;
					result = result3;
				}

				if (currentDistance < distance2)
				{
					closest = result;
					distance2 = currentDistance;
				}
			}

			//scan y
			for (int y = gY - r2; y < gY + r2; y++)
			{
				int x1 = gX - r2;
				int x2 = gX + r2;


					VertexData* result = 0;
					float currentDistance = findClosesVertexInCell(x1, y, z1, result, nX, nY, nZ);
					VertexData* result2 = 0;
					float currentDistance2 = findClosesVertexInCell(x1, y, z2, result2, nX, nY, nZ);
					VertexData* result3 = 0;
					float currentDistance3 = findClosesVertexInCell(x2, y, z1, result3, nX, nY, nZ);
					VertexData* result4 = 0;
					float currentDistance4 = findClosesVertexInCell(x2, y, z2, result4, nX, nY, nZ);
					//update shortest distance vertex
					if (currentDistance > currentDistance2)
					{
						currentDistance = currentDistance2;
						result = result2;
					}
					if (currentDistance3 > currentDistance4)
					{
						currentDistance3 = currentDistance4;
						result3 = result4;
					}
					if (currentDistance > currentDistance3)
					{
						currentDistance = currentDistance3;
						result = result3;
					}

					if (currentDistance < distance2)
					{
						closest = result;
						distance2 = currentDistance;
					}
				
			}
		}


		//end of ring
		if (closest)
		{
			//result found return
			return closest;
		}
	}

	//result not found return null
	return closest;

}


//-----------------------------------------------------------------------------
//
float SpacePartitioning::SpatialHash3D::findClosesVertexInCell(int x, int y, int z, VertexData* out, float nX, float nY, float nZ)
{
	if (z < 0 || z >= m_size[2]) //exceeds boundaries ->skip
		return 99999999;

	if (y < 0 || y >= m_size[1]) //exceeds boundaries ->skip
		return 99999999;

	if (x < 0 || x >= m_size[0]) //exceeds boundaries ->skip
		return 99999999;

	int idx = x + y * m_size[0] + z * m_size[0] * m_size[1];

	float distance2 = 99999999;

	for (auto iter = m_vectorList[idx].begin(); iter != m_vectorList[idx].end(); iter++)
	{
		float currentDistance = L2Distance::compute(nX, nY, nZ, (*iter)->x(), (*iter)->y(), (*iter)->z());
		if (currentDistance < distance2)
		{
            //out = (*iter);
			distance2 = currentDistance;
		}
	}

	return distance2;
}
