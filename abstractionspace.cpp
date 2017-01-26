#include "abstractionspace.h"

AbstractionSpace::AbstractionSpace(int xdim, int ydim)
    : m_xdim(xdim),
      m_ydim(ydim)
{
    // val, alpha, color_intp, point_size ->
    // float leftMin, float leftMax, float rightMin, float rightMax
    space2d[0][0] = QVector4D(20, 100, 0, 1.0);
    space2d[0][1] = QVector4D(20, 40, 1.0, 0.0);
    space2d[0][2] = QVector4D(0, 20, 0, 1.0);
    space2d[0][3] = QVector4D(20, 40, 1, 6);

    space2d[1][0] = QVector4D(20, 50, 0, 1.0);
    space2d[1][1] = QVector4D(40, 50, 1.0, 0.0);
    space2d[1][2] = QVector4D(0, 20, 0, 1.0);
    space2d[1][3] = QVector4D(20, 50, 1, 7);


}

AbstractionSpace::~AbstractionSpace()
{

}

// *---------**---------*
// |         |          |
// |         dy         |
// |         |          |
// *---------*----dx----*
// |         |(x, y)    |
// |         |          |
// |         |          |
// *---------**---------*

void AbstractionSpace::defineAbstractionState(int x, int y, std::string name, int dx, int dy)
{

}

int AbstractionSpace::getSSBOSize()
{
    return  sizeof(space2d);
}

void* AbstractionSpace::getSSBOData()
{
    return space2d;
}
