#include "grid3d.h"



//----------------------------------------------------------------------------
//
SpacePartitioning::Grid3D::Grid3D()
{
	m_size[0] = 0;
	m_size[1] = 0;
	m_size[2] = 0;

	m_data = 0;
	m_data_8bit = 0;
	m_maxValue = 0;
}

//----------------------------------------------------------------------------
//
SpacePartitioning::Grid3D::Grid3D(int sizeX, int sizeY, int sizeZ)
{
	m_size[0] = sizeX;
	m_size[1] = sizeY;
	m_size[2] = sizeZ;
	int total_size = m_size[0] * m_size[1] * m_size[2];
	m_data = new float[total_size];
	m_data_8bit = new unsigned char[total_size * 4];
	memset(m_data, 0, sizeof(float)* total_size);
	memset(m_data_8bit, 0, sizeof(unsigned char)* total_size * 4);
}

//----------------------------------------------------------------------------
//
SpacePartitioning::Grid3D::~Grid3D()
{
	delete m_data;
	delete m_data_8bit;
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
	if (m_data)
		delete m_data;
	if (m_data_8bit)
		delete m_data_8bit;
	int total_size = m_size[0] * m_size[1] * m_size[2];
	m_data = new float[total_size];
	m_data_8bit = new unsigned char[total_size * 4];
	memset(m_data, 0, sizeof(float)* total_size);
	memset(m_data_8bit, 0, sizeof(unsigned char)*  total_size * 4);
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

    m_data[gX + gY * m_size[X] + gZ * m_size[Y] * m_size[Z]] += value;
	if (m_data[gX + gY * m_size[X] + gZ * m_size[Y] * m_size[Z]] > m_maxValue)
	{
		m_maxValue = m_data[gX + gY * m_size[X] + gZ * m_size[Y] * m_size[Z]];
	}
	//m_data[gX + gY * m_size[X] + gZ * m_size[Y] * m_size[Z]] = 255;

}

//-----------------------------------------------------------------------------
//
float* SpacePartitioning::Grid3D::getData()
{
	return m_data;
}

//-----------------------------------------------------------------------------
//
unsigned char* SpacePartitioning::Grid3D::getData8Bit()
{
	int total_size = m_size[0] * m_size[1] * m_size[2];
	for (int i = 0; i < total_size; i++)
	{
		m_data_8bit[i * 4] = (unsigned char)((int)(m_data[i] * 255 / m_maxValue));
		m_data_8bit[i * 4 + 1] = 0;
		m_data_8bit[i * 4 + 2] = 0;
		m_data_8bit[i * 4 + 3] = 255;
	}


	return m_data_8bit;
}
