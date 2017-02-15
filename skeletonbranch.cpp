#include "skeletonbranch.h"

SkeletonBranch::SkeletonBranch()
{

}

void SkeletonBranch::addKnots(int knot1, int knot2)
{
    m_nodes[0] = knot1;
    m_nodes[1] = knot2;
}

QVector2D SkeletonBranch::getKnots()
{
    return QVector2D(m_nodes[0], m_nodes[1]);
}

void SkeletonBranch::addPoints(QStringList pointslist)
{
    for (int i = 0; i < pointslist.size(); ++i) {
        m_points.push_back(pointslist.at(i).toInt());
    }
}
