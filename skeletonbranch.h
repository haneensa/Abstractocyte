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
    void addKnots(int knot1, int knot2);
    QVector2D getKnots();
    void addPointsIndxs(QStringList pointslist);
    std::vector<int> getPointsIndxs()    { return m_pointsIndices; }

protected:
    int              m_nodes[2]; // -> how about each skeleton has set of nodes, and in the branch they refrence them
    std::vector<int> m_pointsIndices; // the points of this branch
};

#endif // SKELETONBRANCH_H
