#ifndef OCTREE_H_
#define OCTREE_H_


#include <stdint.h>
#include <cassert>
#include <cmath>
#include <cstring> 
#include <limits>
#include <vector>

struct VertexData;

namespace SpacePartitioning
{

	//==========================================================================================
	//
	struct L2Distance
	{
		static inline float compute(float px, float py, float pz, float qx, float qy, float qz)
		{
			float diff1 = px - qx;
			float diff2 = py - qy;
			float diff3 = pz - qz;

			return std::pow(diff1, 2) + std::pow(diff2, 2) + std::pow(diff3, 2);
		}

		static inline float norm(float x, float y, float z)
		{
			return std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2);
		}

		static inline float sqr(float r)
		{
			return r * r;
		}

		static inline float sqrt(float r)
		{
			return std::sqrt(r);
		}
	};

	//==========================================================================================
	//
	struct L1Distance
	{
		static inline float compute(float px, float py, float pz, float qx, float qy, float qz)
		{
			float diff1 = px - qx;
			float diff2 = py - qy;
			float diff3 = pz - qz;

			return std::abs(diff1) + std::abs(diff2) + std::abs(diff3);
		}

		static inline float norm(float x, float y, float z)
		{
			return std::abs(x) + std::abs(y) + std::abs(z);
		}

		static inline float sqr(float r)
		{
			return r;
		}

		static inline float sqrt(float r)
		{
			return r;
		}
	};

	//==========================================================================================
	//
	struct MaxDistance
	{
		static inline float compute(float px, float py, float pz, float qx, float qy, float qz)
		{
			float diff1 = std::abs(px - qx);
			float diff2 = std::abs(py - qy);
			float diff3 = std::abs(pz - qz);

			float maximum = diff1;
			if (diff2 > maximum) maximum = diff2;
			if (diff3 > maximum) maximum = diff3;

			return maximum;
		}

		static inline float norm(float x, float y, float z)
		{
			float maximum = x;
			if (y > maximum) maximum = y;
			if (z > maximum) maximum = z;
			return maximum;
		}

		static inline float sqr(float r)
		{
			return r;
		}

		static inline float sqrt(float r)
		{
			return r;
		}
	};


	//==========================================================================================
	//
	class Octree
	{
	public:
		Octree();
		~Octree();

		//initialize octree with all points
		void initialize(const std::vector<VertexData*>* pts, float minX = 0.0f, float minY = 0.0f, float minZ = 0.0f, float maxX = 1.0f, float maxY = 1.0f, float maxZ = 1.0f, int maxDepth = 5);

		//remove all data inside octree
		void clear();

		/** nearest neighbor queries. Using minDistance >= 0, we explicitly disallow self-matches.
		* @return index of nearest neighbor n with Distance::compute(query, n) > minDistance and otherwise -1.
		**/
		int32_t findNeighbor(float x, float y, float z, float minDistance = -1) const;

		/** radius neighbor queries where radius determines the maximal radius of reported indices of points in
		* resultIndices **/
		void radiusNeighbors( float x, float y, float z, float radius, std::vector<uint32_t>& resultIndices) const;
		
	protected:
		class Octant
		{
		public:
			Octant();
			~Octant();

			//is a leaf octant
			bool m_isLeaf;

			// bounding box of the octant needed for overlap and contains tests...
			float m_center_x, m_center_y, m_center_z;  // center
			float m_extent;   // half of side-length

			//indices of points at this octant
			std::vector<uint32_t>* m_indices;

			//children
			Octant* m_child[8];

			//current depth
			int m_depth;

			//function to get indices
			std::vector<uint32_t>* getIndices() const;

		};

		// not copyable, not assignable ...
		Octree(Octree&);
		Octree& operator=(const Octree& oct);

		/**
		* creation of an octant using the elements starting at startIdx.
		*
		* The method reorders the index such that all points are correctly linked to successors belonging
		* to the same octant.
		*
		* \param x,y,z           center coordinates of octant
		* \param extent          extent of octant
		* \param indices         list of indices to be added to the octant
		* \param depth           the depth down the octree
		*
		* \return  octant with children nodes.
		*/
		Octant* createOctant(float x, float y, float z, float extent, std::vector<uint32_t>* indices, int depth);

		/** @return true, if search finished, otherwise false. **/
		bool findNeighbor(const Octant* octant,  float x, float y, float z, float minDistance, float maxDistance, int32_t& resultIndex) const;

		void radiusNeighbors( const Octant* octant,  float x, float y, float z, float radius, float sqrRadius, std::vector<uint32_t>& resultIndices) const;

		//void radiusNeighbors( Octant* octant,  float x, float y, float z, float radius, float sqrRadius, std::vector<uint32_t>& resultIndices, std::vector<float>& distances) const;

		/** test if search ball S(q,r) overlaps with octant
		*
		* @param x y z   query point
		* @param radius  "squared" radius
		* @param o       pointer to octant
		*
		* @return true, if search ball overlaps with octant, false otherwise.
		*/
		static bool overlaps( float x, float y, float z, float radius, float sqRadius, const Octant* o);

		/** test if search ball S(q,r) is completely inside octant.
		*
		* @param x y z   query point
		* @param radius  radius r
		* @param octant  point to octant.
		*
		* @return true, if search ball is completely inside the octant, false otherwise.
		*/
		static bool inside(float x, float y, float z, float radius, const Octant* octant);

		/** test if search ball S(q,r) contains octant
		*
		* @param x y z    query point
		* @param sqRadius "squared" radius
		* @param octant   pointer to octant
		*
		* @return true, if search ball overlaps with octant, false otherwise.
		*/
		static bool contains( float x, float y, float z, float sqRadius, const Octant* octant);

		
		//root node
		Octant* m_root;
		//pointer to data points
		const std::vector<VertexData*>* m_data;
		//maximum depth of the octree
		int m_maxDepth;
		//maximum number of nodes before dividing the octant again
		int m_maxPointsInOctant;

	};

}

#endif /* OCTREE_H_ */


	
