#include "skeletonbranch.h"

SkeletonBranch::SkeletonBranch()
{

}

void SkeletonBranch::addKnots(int knot1, int knot2)
{
    // in case of a child they refrence these from their parent IDs
    m_nodes[0] = knot1;
    m_nodes[1] = knot2;
}

QVector2D SkeletonBranch::getKnots()
{
    return QVector2D(m_nodes[0], m_nodes[1]);
}

void SkeletonBranch::addPointsIndxs(QStringList pointslist)
{
    for (int i = 0; i < pointslist.size(); ++i) {
        m_pointsIndices.push_back(pointslist.at(i).toInt());
    }
}
