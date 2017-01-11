#ifndef MPSTATE_H
#define MPSTATE_H

#include <QVector3D>

class MPState
{
public:
    MPState();

protected:
    int         m_x;
    int         m_y;
    int         m_ID;   /* state ID */
    QVector3D   m_color;
    QVector3D   m_position;
};

#endif // MPSTATE_H
