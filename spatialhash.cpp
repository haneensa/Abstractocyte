#include "spatialhash.h"

SpatialHash::SpatialHash(int col, float min, float max)
    :   m_min(min),
        m_max(max)
{
    m_cellSize = m_max / (float)col;
    m_col = (m_max - m_min) / m_cellSize;
    m_buckets = m_col * m_col;

    qDebug() << "m_col: " << m_col;
    qDebug() << "m_bukcets: " << m_buckets;

    /* I only need the cell size */
    qDebug() << "m_cellSize: " << m_cellSize;
}


SpatialHash::~SpatialHash()
{
    qDebug() << "Func: ~SpatHash";
}


std::pair<int,int>  SpatialHash::hash(float x, float y)
{
    float factor = 1.0/(float)m_cellSize;
    int i =  (int) std::floor(x * factor);
    int j = (int) std::floor(y * factor);

    return std::make_pair(i,j);
}

/*
 *     ****************o (xMax, yMax)
 *     *       *       *
 *     ******* * *******
 *     *       *       *
 *     o****************
 *   (xMin, yMin)
 **/
std::set< std::pair<int,int>  > SpatialHash::hashAABB(float x, float y, float r)
{
    std::set< std::pair<int,int>  > IdsList;
    float minX = x-r;
    float minY = y-r;
    float maxX = x+r;
    float maxY = y+r;

    std::pair<int,int>  bleft = hash(minX, minY);
    std::pair<int,int>  tright = hash(maxX, maxY);

    for (int i = bleft.first; i <= tright.first; ++i) {
        for (int j = bleft.second; j <= tright.second; ++j) {
            IdsList.insert( std::make_pair(i,j) );
        }
    }

    return IdsList;
}

void SpatialHash::insert(Node *node)
{
    if (node == NULL)   return;
    float x = node->getLayoutedPosition().x();
    float y = node->getLayoutedPosition().y();
    std::pair<int,int>  cell = hash(x, y);
    int index, shift;
    if (m_hashMap.find(cell) == m_hashMap.end()) { // empty cell
        std::vector<Node*> vec(1, node);
        m_hashMap.insert( std::make_pair(cell, vec) );
        // we are creating the cell for the first time
    } else {
        m_hashMap.at(cell).push_back( node );
    }

    index = m_hashMap.at(cell).size() - 1;
}

void SpatialHash::queryAABB(Node *node, float r, std::vector<Node*> &dataCell)
{
    if (node == NULL)   return;
    float x = node->getLayoutedPosition().x();
    float y = node->getLayoutedPosition().y();
    std::set< std::pair<int,int> > Ids = hashAABB(x, y, r);
    dataCell.clear();
    std::set< std::pair<int,int> >::iterator it;
    for (it = Ids.begin(); it != Ids.end(); it++) {
        if (m_hashMap.find(*it) == m_hashMap.end())
            continue;
        dataCell.insert( dataCell.end(), m_hashMap.at(*it).begin(), m_hashMap.at(*it).end() );
    }

    return;
}

void SpatialHash::updateNode(Node *node)
{
    // check if node moved to another cell
    float x = node->getLayoutedPosition().x();
    float y = node->getLayoutedPosition().y();
    std::pair<int,int>  cell = hash(x, y);
    std::pair<int, int> oldCell = node->getHashMapCell();
    if (cell == oldCell) {
        return;
    }

    if (m_hashMap.find(oldCell) != m_hashMap.end()) {
          m_hashMap.at(oldCell).erase(std::remove(m_hashMap.at(oldCell).begin(), m_hashMap.at(oldCell).end(), node), m_hashMap.at(oldCell).end());
          insert(node);
      } else {
          qDebug() << "This node " << node->getId() << " at ( " << oldCell.first << ", " << oldCell.second << " ) " << " doesnt exist in hashMap!";
      }
}

void SpatialHash::clear()
{
    m_hashMap.clear();
}

