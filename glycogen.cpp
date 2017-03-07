#include "glycogen.h"

Glycogen::Glycogen(int ID, std::string name, QVector3D center, float volume)
{
    m_ID = ID;
    m_name = name;
    m_center = center;
    m_volume = volume;
}
