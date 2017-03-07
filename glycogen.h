#ifndef GLYCOGEN_H
#define GLYCOGEN_H

#include <QVector3D>

class Glycogen
{
public:
    Glycogen(int ID, std::string name, QVector3D center, float diameter);
    int getID()         { return m_ID; }
    QVector3D getCenter() { return m_center; }
    float getDiameter() { return m_diameter; }

protected:
    int             m_ID;
    std::string     m_name;
    QVector3D       m_center;
    float           m_diameter;

};

#endif // GLYCOGEN_H
