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

std::vector<QVector3D> Skeleton::getGraphNodes()
{
    return m_nodes;
}

std::vector<QVector2D> Skeleton::getGraphEdges()
{
    std::vector<QVector2D> graph_knots;
    // iterate over the branches and get the knots
    qDebug() << "m_branches.size(): " << m_branches.size();
    for (int i = 0; i < m_branches.size(); ++i) {
        QVector2D knots  = m_branches[i]->getKnots();
        graph_knots.push_back(knots);
    }

    return graph_knots;
}
