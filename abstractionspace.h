#ifndef ABSTRACTIONSPACE_H
#define ABSTRACTIONSPACE_H

#include <vector>
#include <string>
#include <QVector4D>

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
    QVector4D                           space2d[2][5]; // val, alpha, color_intp, point_size, additional infos
};

#endif // ABSTRACTIONSPACE_H
