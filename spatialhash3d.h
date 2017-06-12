#ifndef SPATIALHASH3D_H_
#define SPATIALHASH3D_H_


#include <stdint.h>
#include <cassert>
#include <cmath>
#include <cstring> 
#include <limits>
#include <vector>


namespace SpacePartitioning
{
	class SpatialHash3D
	{
	public:
		SpatialHash3D();
		SpatialHash3D(int sizeX, int sizeY, int sizeZ);
		~SpatialHash3D();

		void addNormalizedPoint(float x, float y, float z, VertexData* value);
		void setSize(int sizeX, int sizeY, int sizeZ);
		void reset();
		std::vector<std::vector<VertexData*>>* getData();
		std::vector<VertexData*>* getClosestNeighbors(float nX, float nY, float nZ);
		VertexData* getNeighbor(float nX, float nY, float nZ);
		VertexData* getNeighbor2(float nX, float nY, float nZ);

	protected:

		float findClosesVertexInCell(int x, int y, int z, VertexData* out, float nX, float nY, float nZ);

		int m_size[3];
		int m_totalSize;
		std::vector<std::vector<VertexData*>> m_vectorList;
	};
}

#endif /* SPATIALHASH3D_H_ */
