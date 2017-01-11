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
    m_mesh_vertices.push_back(vertex);
}

void Object::add_ms_vertex(QVector3D mesh_vertex, QVector3D skeleton_vertex)
{
    struct VertexData vertex_data;
    vertex_data.mesh_vertex = mesh_vertex;
    vertex_data.skeleton_vertex = skeleton_vertex;
    m_mesh_skeleton_vertices.push_back(vertex_data);
}

std::vector<QVector3D> Object::getVertices()
{
    return m_mesh_vertices;
}

std::string Object::getName()
{
    return m_name;
}

int Object::getSize()
{
    return m_mesh_vertices.size();
}

QVector4D Object::getColor()
{
    return m_color;
}

void Object::setColor(QVector4D color)
{
    m_color = color;
}
