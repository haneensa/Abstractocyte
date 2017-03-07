#ifndef GLYCOGEN_H
#define GLYCOGEN_H

#include <QVector3D>

class Glycogen
{
public:
    Glycogen(int ID, std::string name, QVector3D center, float volume);


protected:
    int             m_ID;
    std::string     m_name;
    QVector3D       m_center;
    float           m_volume;

};

#endif // GLYCOGEN_H
