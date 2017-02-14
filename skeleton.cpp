#include "skeleton.h"

Skeleton::Skeleton(int ID)
{
    m_ID = ID;
}

Skeleton::~Skeleton()
{
    // delete any pointers
    qDebug() << "~Skeleton";
}

void Skeleton::addNode(QVector3D coords)
{
    m_nodes.push_back(coords); // nodes IDs are from 0 to max, so the index can be used as ID index
}

void Skeleton::addPoint(QVector3D coords)
{
    QVector4D point = coords.toVector4D();
    point.setW(m_ID);
    m_points.push_back({point}); // nodes IDs are from 0 to max, so the index can be used as ID index
}

void*  Skeleton::getSkeletonPoints()
{
    return m_points.data();
}

int Skeleton::getSkeletonPointsSize()
{
    return m_points.size();
}

void Skeleton::addBranch(SkeletonBranch *branch)
{
    m_branches.push_back(branch);
}
