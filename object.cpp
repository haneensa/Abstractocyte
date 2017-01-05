#include "object.h"
#include <QDebug>

Object::Object(std::string name)
{
    if (name[name.size()-1] == '\n') {
        qDebug() << "new line! ";
        name.erase(name.size()-1, name.size()-1);
    }

    m_name  = name;
    m_ID    = 0;
    m_object_t = getObjectType(m_name);
    qDebug() << "create " << m_name.data();
}

Object_t Object::getObjectType(std::string name)
{
    return Object_t::ASTROCYTE;
}

void Object::add_vertex(QVector3D vertex)
{
    m_vertices.push_back(vertex);
}

std::vector<QVector3D> Object::getVertices()
{
    return m_vertices;
}
