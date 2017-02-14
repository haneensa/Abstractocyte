#ifndef SKELETON_H
#define SKELETON_H

#include "skeletonbranch.h"
#include <QVector3D>

// skeleton vertex
struct SkeletonVertex {
    QVector4D   vertex; // w: ID
};

class Skeleton
{
public:
    Skeleton(int ID);
    ~Skeleton();
    void addNode(QVector3D coords);
    void addPoint(QVector3D coords);
    void*  getSkeletonPoints();
    int getSkeletonPointsSize();
    void addBranch(SkeletonBranch *branch);

protected:
    int                             m_ID;
    // skeleton branches
    // o branch_id
    // v x y z
    // ....
    // l v1 v2
    // ...

    // a skeleton consists of: nodes, points, and segments
    // local IDs!!
    // nodes has their own IDs [0, Max] for each object, different segments can share the same nodes
    // points has their own IDs [0, Max] for each object and they are distincts per segment
    // segments has IDs but they are not useful to track anything, so I can use any order

    std::vector<QVector3D>              m_nodes; // vector or map?
    std::vector<struct SkeletonVertex>  m_points;
    std::vector<SkeletonBranch *>       m_branches;
};

#endif // SKELETON_H
