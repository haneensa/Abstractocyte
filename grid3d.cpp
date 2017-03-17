#include "grid3d.h"



//----------------------------------------------------------------------------
//
SpacePartitioning::Grid3D::Grid3D()
{
	m_size[0] = 0;
	m_size[1] = 0;
	m_size[2] = 0;

	data = 0;
}

//----------------------------------------------------------------------------
//
SpacePartitioning::Grid3D::Grid3D(int sizeX, int sizeY, int sizeZ)
{
	m_size[0] = sizeX;
	m_size[1] = sizeY;
	m_size[2] = sizeZ;

	data = new float[sizeX * sizeY * sizeZ];
	memset(data, 0, sizeof(float)* sizeX * sizeY * sizeZ);
}

//----------------------------------------------------------------------------
//
SpacePartitioning::Grid3D::~Grid3D()
{
	delete data;
}

//----------------------------------------------------------------------------
//
void SpacePartitioning::Grid3D::setSize(int sizeX, int sizeY, int sizeZ)
{
	
	m_size[0] = sizeX;
	m_size[1] = sizeY;
	m_size[2] = sizeZ;

	reset();
}

//----------------------------------------------------------------------------
//
void SpacePartitioning::Grid3D::reset()
{
	delete data;
	data = new float[m_size[0] * m_size[1] * m_size[2]];
	memset(data, 0, sizeof(float)* m_size[0] * m_size[1] * m_size[2]);
}

//----------------------------------------------------------------------------
//
void SpacePartitioning::Grid3D::addNormalizedPoint(float nX, float nY, float nZ, float value)
{
	int X = 0;
	int Y = 1;
	int Z = 2;

	int gX = (int)(nX * (m_size[X]-1));
	int gY = (int)(nY * (m_size[Y]-1));
	int gZ = (int)(nZ * (m_size[Z]-1));

	data[gX + gY * m_size[X] + gZ * m_size[Y] * m_size[Z]] += value;

}

//-----------------------------------------------------------------------------
//
float* SpacePartitioning::Grid3D::getData()
{
	return data;
}