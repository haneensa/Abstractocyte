#ifndef SKELETON_H
#define SKELETON_H

#include "skeletonbranch.h"
#include <QVector3D>

// skeleton vertex
// todo: use union instead to represent fields?
struct SkeletonVertex {
    QVector4D   vertex; // w: ID
    QVector4D   knot1;   // end points of the segment
    QVector4D   knot2;   // end points of the segment
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

     // for simplified graph, we need the branches
    // knots IDs and their edges
    std::vector<QVector3D> getGraphNodes();
    std::vector<QVector2D> getGraphEdges();

protected:
    int                             m_ID;

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
