#ifndef SKELETON_H
#define SKELETON_H

#include "skeletonbranch.h"
#include <QVector3D>

// skeleton vertex
// todo: use union instead to represent fields?
struct SkeletonPoint {
    QVector4D   vertex; // w: ID
    QVector4D   knot1;   // end points of the segment
    QVector4D   knot2;   // end points of the segment
};

// allocate vbo with enough space for all skeletons using this struct
// use this for all skeleton stages (simplified one)
// initialize this when while contructing the skeletons
// with their edges
struct AbstractSkelNode { // -> Abstract_Skel_Node
    QVector4D vertex;   // original position  (simplified)  -> w: hvgx ID
    QVector2D layout1;  // layouted position (all skeletons)
    QVector2D layout2;  // layouted position (no neurites)
    QVector2D layout3;  // layouted position (no astrocyte)
};


class Skeleton
{
public:
    Skeleton(int ID);
    ~Skeleton();
    void addNode(QVector3D coords);
    void addPoint(QVector3D coords);
    void addBranch(SkeletonBranch *branch, Skeleton *parentSkeleton);

    void markPoint(int pIndex, int IDMark);

    void*  getSkeletonPoints();
    int getSkeletonPointsSize();

     // for simplified graph, we need the branches
    // knots IDs and their edges
    std::vector<QVector3D> getGraphNodes();
    std::vector<QVector2D> getGraphEdges();

    void setIndexOffset(int offset) { m_idx_offset = offset; }
    int getIndexOffset()            { return m_idx_offset; }
protected:
    int                             m_ID;

    int                             m_idx_offset;

    // a skeleton consists of: nodes, points, and segments
    // local IDs!!
    // nodes has their own IDs [0, Max] for each object, different segments can share the same nodes
    // points has their own IDs [0, Max] for each object and they are distincts per segment
    // segments has IDs but they are not useful to track anything, so I can use any order

    std::vector<QVector3D>              m_nodes; // vector or map?
    std::vector<struct SkeletonPoint>  m_points;
    std::vector<SkeletonBranch *>       m_branches;
};

#endif // SKELETON_H
