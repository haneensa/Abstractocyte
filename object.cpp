#include "object.h"
#include <QDebug>

Object::Object(std::string name)
{
    if (name[name.size()-1] == '\n') {
        name.erase(name.size()-1, name.size()-1);
    }

    m_name  = name;
    m_ID    = 0;
    m_object_t = getObjectType(m_name);
    qDebug() << "create " << m_name.data();
}

Object::~Object()
{
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

bool Object::allocate_vbo(QOpenGLShaderProgram  *program_mesh)
{


    return true;
}

void Object::draw()
{
}

std::string Object::getName()
{
    return m_name;
}

int Object::getSize()
{
    return m_vertices.size();
}

QVector4D Object::getColor()
{
    return m_color;
}

void Object::setColor(QVector4D color)
{
    m_color = color;
}
