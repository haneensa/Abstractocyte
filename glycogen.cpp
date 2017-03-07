#include "glycogen.h"

Glycogen::Glycogen(int ID, std::string name, QVector3D center, float diameter)
{
    m_ID = ID;
    m_name = name;
    m_center = center/5.0;
    m_diameter = diameter;
}
