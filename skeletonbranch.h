#ifndef SKELETONBRANCH_H
#define SKELETONBRANCH_H

#include "mainopengl.h"

// each branch has two knots (nodes) and set of points between them
// more than a branch can share the same knots
// but the set of point are unique per branch
class SkeletonBranch
{
public:
    SkeletonBranch();

protected:
    std::vector<int> m_nodes; // -> how about each skeleton has set of nodes, and in the branch they refrence them
    std::vector<QVector2D> m_points; // the points of this branch
};

#endif // SKELETONBRANCH_H
