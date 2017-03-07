#include "skeleton.h"


// I need away to reconstruct the nodes and points for spines and boutons from their parents
// -> create graph from parent
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
    point.setW(m_ID); // check if this belongs to a child then add child ID here
    QVector4D knot1, knot2;
    m_points.push_back({point, knot1, knot2}); // nodes IDs are from 0 to max, so the index can be used as ID index
}

void Skeleton::markPoint(int pIndex, int IDMark)
{
    if (m_points.size() < pIndex) {
        qDebug() << pIndex << " out of range at mark point";
        return;
    }

    m_points[pIndex].vertex.setW(IDMark);
}

void*  Skeleton::getSkeletonPoints()
{
    return m_points.data();
}


int  Skeleton::getSkeletonPointsSize()
{
    return m_points.size();
}

std::vector<struct SkeletonPoint> Skeleton::getSkeletonPointsVec()
{
    return m_points;
}

 void Skeleton::updatePointAtIndex(int index, struct SkeletonPoint v)
 {
     m_points[index] = v;
 }


 // for each branch we add if spine or bouton then construct nodes and points
void Skeleton::addBranch(SkeletonBranch *branch, Skeleton *parentSkeleton)
{
    // get the point and set its knots to this branch knots
    m_branches.push_back(branch);
    // get this branch points set
    // reiterate over the m_points, and for each index in this branch get the point set its knots
    QVector2D knots = branch->getKnots();
    std::vector<QVector3D> nodes = m_nodes;
    std::vector<struct SkeletonPoint> points  = m_points;

    std::vector<int> pointsIndices = branch->getPointsIndxs();
    for (int i = 0; i < pointsIndices.size(); i++) {
        int index = pointsIndices[i];

        // if child, then map this point index to a point here

        // if spine or bouton use parent skeleton points and nodes
        if (parentSkeleton != NULL) {
            // use parent instead
             nodes = parentSkeleton->getGraphNodes();
             points = parentSkeleton->getSkeletonPointsVec();
             // this is the child skeleton
             // then and m_nodes
             // m_points
             // I need away to map nodes indices to the one in m_nodes
        }

        if (points.size() <= index || nodes.size() < knots.x() || nodes.size() < knots.y()) {
            qDebug() << "index out of range addBranch points";
            return;
        }

        // should I store points here???

        struct SkeletonPoint v = points[index];
        QVector3D n1 = nodes[knots.x()];
        QVector3D n2 = nodes[knots.y()];
        v.knot1 = QVector4D(n1.x(), n1.y(), n1.z(), 1.0);
        v.knot2 = QVector4D(n2.x(), n2.y(), n2.z(), 1.0);

        if (parentSkeleton != NULL)
            parentSkeleton->updatePointAtIndex(index, v);
        else
            updatePointAtIndex(index, v);
    }
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
