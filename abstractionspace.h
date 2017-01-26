#ifndef ABSTRACTIONSPACE_H
#define ABSTRACTIONSPACE_H

#include <vector>
#include <string>

struct ssbo_absSpace {

};

class AbstractionSpace
{
public:
    AbstractionSpace(int xdim, int ydim);
    ~AbstractionSpace();

    void defineAbstractionState(int x, int y, std::string name, int dx = -1, int dy = -1);
    // ssbo buffer data
    int getSSBOSize();
    void* getSSBOData();

private:
    int                                 m_xdim;
    int                                 m_ydim;
    std::vector<int>                    m_absStates;
    int                                 space2d[2][100];
};

#endif // ABSTRACTIONSPACE_H
