#ifndef GRID3D_H_
#define GRID3D_H_


#include <stdint.h>
#include <cassert>
#include <cmath>
#include <cstring> 
#include <limits>
#include <vector>

struct VertexData;

namespace SpacePartitioning
{
	class Grid3D
	{
	public:
		Grid3D();
		Grid3D(int sizeX, int sizeY, int sizeZ);
		~Grid3D();

		void addNormalizedPoint(float x, float y, float z, float value);
		void setSize(int sizeX, int sizeY, int sizeZ);
		void reset();
		float* getData();
	protected:

		int m_size[3];

		float* data;


	};
}

#endif /* GRID3D_H_ */