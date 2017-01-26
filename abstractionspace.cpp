#include "abstractionspace.h"

AbstractionSpace::AbstractionSpace(int xdim, int ydim)
    : m_xdim(xdim),
      m_ydim(ydim)
{
    for ( int i = 0; i < 2; i++) {
        for (int j = 0; j < 100; j++ ) {
            space2d[i][j] = 0;
        }
    }
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
