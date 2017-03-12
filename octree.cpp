#include "octree.h"
#include "mesh.h"

namespace SpacePartitioning
{
	//--------------------------------------------------------------------------------
	//
	Octree::Octree() : m_root(0), m_data(0)
	{
		m_maxDepth = 5;
		m_maxPointsInOctant = 32;
	}

	//--------------------------------------------------------------------------------
	//
	Octree::~Octree()
	{
		delete m_root;
	}

	//--------------------------------------------------------------------------------
	//
	void Octree::initialize(const std::vector<VertexData*>* pts, float minX, float minY, float minZ, float maxX, float maxY, float maxZ, int maxDepth)
	{
		clear();

		m_data = pts;

		uint32_t N = pts->size();
		std::vector<uint32_t>* indices = new std::vector<uint32_t>();

		float min[3], max[3];
		min[0] = minX;
		min[1] = minY;
		min[2] = minZ;
		max[0] = maxX;
		max[1] = maxY;
		max[2] = maxZ;

		for (uint32_t i = 0; i < N; ++i)
		{
			indices->push_back(i);
		}

		m_maxDepth = maxDepth;

		//root center point
		float ctr[3] = { min[0], min[1], min[2] };

		float maxextent = 0.5f * (max[0] - min[0]);
		ctr[0] += maxextent;
		for (uint32_t i = 1; i < 3; ++i)
		{
			float extent = 0.5f * (max[i] - min[i]);
			ctr[i] += extent;
			if (extent > maxextent) maxextent = extent;
		}

		m_root = createOctant(ctr[0], ctr[1], ctr[2], maxextent, indices, 0);

	}

	//--------------------------------------------------------------------------------
	//
	void Octree::clear()
	{
		delete m_root;
		m_root = 0;
		m_data = 0;
		//successors_.clear();
	}

	//--------------------------------------------------------------------------------
	//
	Octree::Octant::Octant()
		: m_isLeaf(true), m_center_x(0.0f), m_center_y(0.0f), m_center_z(0.0f), m_extent(0.0f), m_indices(0), m_depth(0)
	{
		memset(&m_child, 0, 8 * sizeof(Octant*));
	}

	//--------------------------------------------------------------------------------
	//
	Octree::Octant::~Octant()
	{
		for (uint32_t i = 0; i < 8; ++i) delete m_child[i];
		if (m_indices)
			delete m_indices;

	}

	//--------------------------------------------------------------------------------
	//
	Octree::Octant* Octree::createOctant(float x, float y, float z, float extent, std::vector<uint32_t>* indices, int depth)
	{

		Octant* octant = new Octant;

		octant->m_isLeaf = true;

		octant->m_center_x = x;
		octant->m_center_y = y;
		octant->m_center_z = z;
		octant->m_extent = extent;
		octant->m_indices = indices;
		octant->m_depth = depth;

		static const float factor[] = { -0.5f, 0.5f };

		// subdivide subset of points and re-link points according to Morton codes
		if (m_maxDepth >= depth && octant->m_indices->size() > m_maxPointsInOctant)
		{
			octant->m_isLeaf = false;

			std::vector<uint32_t>* child_octant_indices[8];

			//create indices holders for the children
			for (int i = 0; i < 8; i++)
				child_octant_indices[i] = new std::vector<uint32_t>();

			//loop on points of this octant
			for (uint32_t i = 0; i < octant->m_indices->size(); ++i)
			{
				//get index
				int index = (*octant->m_indices)[i];
				//get the point from the index
				VertexData* p = ((*m_data)[index]);

				// determine child octant for each point...
				uint32_t octantIndex = 0;
				if (p->x() > x) octantIndex = 1;
				if (p->y() > y) octantIndex += 2;
				if (p->z() > z) octantIndex += 4;

				//push index to the correct child octant
				child_octant_indices[octantIndex]->push_back(index);
			}

			// now, we can create the child nodes...
			float childExtent = 0.5f * extent; //size of child is half of the parent
			for (uint32_t i = 0; i < 8; ++i)
			{
				//if no points for the child
				if (child_octant_indices[i]->size() == 0)
				{
					delete child_octant_indices[i];
					continue;
				}

				//child center
				float childX = x + factor[(i & 1) > 0] * extent;
				float childY = y + factor[(i & 2) > 0] * extent;
				float childZ = z + factor[(i & 4) > 0] * extent;

				//create child
				octant->m_child[i] = createOctant(childX, childY, childZ, childExtent, child_octant_indices[i], depth + 1);

			}
		}

		return octant;
	}

	//--------------------------------------------------------------------------------
	//
	std::vector<uint32_t>* Octree::Octant::getIndices() const
	{
		return m_indices;
	}



	//--------------------------------------------------------------------------------
	//
	int32_t Octree::findNeighbor(float x, float y, float z, float minDistance, int self_index) const
	{
		float maxDistance = std::numeric_limits<float>::infinity();
		int32_t resultIndex = -1;
		findNeighbor(m_root, x, y, z, minDistance, maxDistance, resultIndex, self_index);
		
		int globalIndex = (*m_data)[resultIndex]->index;

		return globalIndex;
	}

	//--------------------------------------------------------------------------------
	//
	bool Octree::findNeighbor(const Octant* octant, float x, float y, float z, float minDistance, float maxDistance, int32_t& resultIndex, int self_index) const
	{
		// 1. first descend to leaf and check in leafs points.
		if (octant->m_isLeaf)
		{
			float sqrMaxDistance = L2Distance::sqr(maxDistance);
			float sqrMinDistance = (minDistance < 0) ? minDistance : L2Distance::sqr(minDistance);

			const std::vector<uint32_t>* octant_indices = octant->getIndices();

			for (uint32_t i = 0; i < octant_indices->size(); ++i)
			{
				uint32_t index = (*octant_indices)[i];
				VertexData* d = (*m_data)[index];
				if (d->index == self_index)
					continue;

				float dist = L2Distance::compute(d->x(), d->y(), d->z(), x, y, z);
				if (dist > sqrMinDistance && dist < sqrMaxDistance)
				{
					resultIndex = index;
					sqrMaxDistance = dist;
				}
			}

			maxDistance = L2Distance::sqrt(sqrMaxDistance);
			return inside(x, y, z, maxDistance, octant);
		}

		// determine child index for each point...
		uint32_t childIndex = 0;
		if (x > octant->m_center_x) childIndex += 1;
		if (y > octant->m_center_y) childIndex += 2;
		if (z > octant->m_center_z) childIndex += 4;

		if (octant->m_child[childIndex] != 0)
		{
			if (findNeighbor(octant->m_child[childIndex], x, y , z, minDistance, maxDistance, resultIndex, self_index)) return true;
		}

		// 2. if current best point completely inside, just return.
		float sqrMaxDistance = L2Distance::sqr(maxDistance);

		// 3. check adjacent octants for overlap and check these if necessary.
		for (uint32_t c = 0; c < 8; ++c)
		{
			if (c == childIndex) continue;
			if (octant->m_child[c] == 0) continue;
			if (!overlaps(x, y, z, maxDistance, sqrMaxDistance, octant->m_child[c])) continue;
			if (findNeighbor(octant->m_child[c], x,y,z, minDistance, maxDistance, resultIndex, self_index))
				return true;  // early pruning
		}

		// all children have been checked...check if point is inside the current octant...
		return inside(x, y, z, maxDistance, octant);
	}

	//--------------------------------------------------------------------------------
	//
	bool Octree::inside(float xi, float yi, float zi, float radius, const Octant* octant)
	{
		// we exploit the symmetry to reduce the test to test
		// whether the farthest corner is inside the search ball.
		float x = xi - octant->m_center_x;
		float y = yi - octant->m_center_y;
		float z = zi - octant->m_center_z;

		x = std::abs(x) + radius;
		y = std::abs(y) + radius;
		z = std::abs(z) + radius;

		if (x > octant->m_extent) return false;
		if (y > octant->m_extent) return false;
		if (z > octant->m_extent) return false;

		return true;
	}

	//--------------------------------------------------------------------------------
	//
	bool Octree::overlaps(float xi, float yi, float zi, float radius, float sqRadius, const Octant* o)
	{
		// we exploit the symmetry to reduce the test to testing if its inside the Minkowski sum around the positive quadrant.
		float x = xi - o->m_center_x;
		float y = yi - o->m_center_y;
		float z = zi - o->m_center_z;

		x = std::abs(x);
		y = std::abs(y);
		z = std::abs(z);

		float maxdist = radius + o->m_extent;

		// Completely outside, since q' is outside the relevant area.
		if (x > maxdist || y > maxdist || z > maxdist) return false;

		int32_t num_less_extent = (x < o->m_extent) + (y < o->m_extent) + (z < o->m_extent);

		// Checking different cases:

		// a. inside the surface region of the octant.
		if (num_less_extent > 1) return true;

		// b. checking the corner region && edge region.
		x = std::max(x - o->m_extent, 0.0f);
		y = std::max(y - o->m_extent, 0.0f);
		z = std::max(z - o->m_extent, 0.0f);

		return (L2Distance::norm(x, y, z) < sqRadius);
	}

	//--------------------------------------------------------------------------------
	//
	bool Octree::contains(float xi, float yi, float zi, float sqRadius, const Octant* o)
	{
		// we exploit the symmetry to reduce the test to test
		// whether the farthest corner is inside the search ball.
		float x = xi - o->m_center_x;
		float y = yi - o->m_center_y;
		float z = zi - o->m_center_z;

		x = std::abs(x);
		y = std::abs(y);
		z = std::abs(z);
		// reminder: (x, y, z) - (-e, -e, -e) = (x, y, z) + (e, e, e)
		x += o->m_extent;
		y += o->m_extent;
		z += o->m_extent;

		return (L2Distance::norm(x, y, z) < sqRadius);
	}

	//--------------------------------------------------------------------------------
	//
	void Octree::radiusNeighbors(float x, float y, float z, float radius, std::vector<uint32_t>& resultIndices, int self_index) const
	{
		resultIndices.clear();
		if (m_root == 0) return;

		float sqrRadius = L2Distance::sqr(radius);  // "squared" radius
		radiusNeighbors(m_root, x, y, z, radius, sqrRadius, resultIndices, self_index);

		//TODO: convert local indices to global ones
	}

	//--------------------------------------------------------------------------------
	//
	void Octree::radiusNeighbors(const Octant* octant, float x, float y, float z, float radius, float sqrRadius, std::vector<uint32_t>& resultIndices, int self_index) const
	{
		const std::vector<uint32_t>* indices = octant->getIndices();
		// if search ball S(q,r) contains octant, simply add point indexes.
		if (contains(x,y,z, sqrRadius, octant))
		{
			
			for (uint32_t i = 0; i < indices->size(); ++i)
			{
				
				uint32_t idx = (*indices)[i];
				const VertexData* p = (*m_data)[idx];
				if (p->index == self_index)
					continue;
				resultIndices.push_back(idx);
			}

			return;  // early pruning.
		}

		if (octant->m_isLeaf)
		{
			for (uint32_t i = 0; i < indices->size(); ++i)
			{
				uint32_t idx = (*indices)[i];
				const VertexData* p = (*m_data)[idx];
				if (p->index == self_index)
					continue;

				float dist = L2Distance::compute(x, y, z, p->x(), p->y(), p->z());
				if (dist < sqrRadius) 
					resultIndices.push_back(idx);
			}

			return;
		}

		// check whether child nodes are in range.
		for (uint32_t c = 0; c < 8; ++c)
		{
			if (octant->m_child[c] == 0) continue;
			if (!overlaps(x,y,z, radius, sqrRadius, octant->m_child[c])) continue;
			radiusNeighbors(octant->m_child[c], x,y,z, radius, sqrRadius, resultIndices, self_index);
		}
	}


}