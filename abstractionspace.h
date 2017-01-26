#ifndef ABSTRACTIONSPACE_H
#define ABSTRACTIONSPACE_H


class AbstractionSpace
{
public:
    AbstractionSpace(int xdim, int ydim);
    ~AbstractionSpace();

private:
    int m_xdim;
    int m_ydim;
    //std::vector<int> m_absLevels;
};

#endif // ABSTRACTIONSPACE_H
