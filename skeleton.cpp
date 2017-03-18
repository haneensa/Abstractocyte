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
    SkeletonPoint v =  {point, knot1, knot2};
    m_points[m_points.size()] = std::make_pair(m_points.size(), v); // nodes IDs are from 0 to max, so the index can be used as ID index
    points_vec.push_back({point, knot1, knot2});
}

// make sure a parent is not fully marked - at least one point is marked as parent
void Skeleton::markPoint(int pIndex, int IDMark)
{
    if (m_points.size() < pIndex) {
        qDebug() << pIndex << " out of range at mark point";
        return;
    }

    // use the index ID to get the index in vector
    std::pair< int, struct SkeletonPoint > datum = m_points[pIndex];
    // update the SkeletonPoint
    datum.second.vertex.setW(IDMark);
    m_points[pIndex] = datum;
    points_vec[datum.first] = datum.second;
}


void*  Skeleton::getSkeletonPoints()
{
    // no need for order for points

    return points_vec.data();
}


int  Skeleton::getSkeletonPointsSize()
{
    return m_points.size();
}

 // for each branch we add if spine or bouton then construct nodes and points
void Skeleton::addBranch(SkeletonBranch *branch, Skeleton *parentSkeleton)
{
    // get the point and set its knots to this branch knots
    // get this branch points set
    // reiterate over the m_points, and for each index in this branch get the point set its knots
    QVector2D knots = branch->getKnots();

    std::vector<int> pointsIndices = branch->getPointsIndxs();
    for (int i = 0; i < pointsIndices.size(); i++) {
        int index = pointsIndices[i];

        if (parentSkeleton != NULL) {  //if child, then map this point index to a point here

           //  if spine or bouton use parent skeleton points and nodes
           // use parent instead
            std::map<int, std::pair< int, struct SkeletonPoint> >* parentPoints = parentSkeleton->getPointsMapPtr();
            if (parentPoints->find(index) == parentPoints->end())
            {
                qDebug() << "Point not fount at index " << index;
                return;
            }

            std::pair< int, struct SkeletonPoint > datum = parentPoints->at(index);

            std::vector<QVector3D> parentNodes = parentSkeleton->getGraphNodes();

            if (parentPoints->size() < knots.x() || parentPoints->size() < knots.y() ) {
                qDebug() << "Node index is incorrect" << parentPoints->size() << " " << knots;
                return;
            }

            std::pair< int, struct SkeletonPoint > sk_n1 = parentPoints->at(knots.x());
            std::pair< int, struct SkeletonPoint > sk_n2 = parentPoints->at(knots.y());

            QVector3D n1 = sk_n1.second.vertex.toVector3D();
            QVector3D n2 = sk_n2.second.vertex.toVector3D();

            datum.second.knot1 = QVector4D(n1.x(), n1.y(), n1.z(), 1.0);
            datum.second.knot2 = QVector4D(n2.x(), n2.y(), n2.z(), 1.0);

            branch->addKnots(m_nodes.size(), m_nodes.size() + 1);

            m_nodes.push_back(n1);
            m_nodes.push_back(n2);
            m_branches.push_back(branch);

        } else {
            if (m_points.size() <= index || m_nodes.size() < knots.x() || m_nodes.size() < knots.y()) {
                qDebug() << "index out of range addBranch points";
                return;
            }

            // should I store points here???
            std::pair< int, struct SkeletonPoint > datum = m_points[index];
            QVector3D n1 = m_nodes[knots.x()];
            QVector3D n2 = m_nodes[knots.y()];
            datum.second.knot1 = QVector4D(n1.x(), n1.y(), n1.z(), 1.0);
            datum.second.knot2 = QVector4D(n2.x(), n2.y(), n2.z(), 1.0);

            m_points[index] = datum;
            points_vec[datum.first] = datum.second;

            m_branches.push_back(branch);

        }
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
