#include "object.h"
#include <QDebug>

Object::Object(std::string name, int idx, int ID)
{
    if (name[name.size()-1] == '\n') {
        name.erase(name.size()-1, name.size()-1);
    }

    m_name  = name;
    m_ID    = ID;
    m_idxID = idx;
    m_object_t = getObjectType(m_name);
    qDebug() << "create " << m_name.data();
}

Object::~Object()
{
}

Object_t Object::getObjectType(std::string name)
{
    // make it generic later
    return Object_t::ASTROCYTE;
}

void Object::add_m_vertex(QVector3D vertex)
{
    m_mesh_vertices.push_back(vertex);
}

void Object::add_s_vertex(QVector3D vertex)
{
    m_skeleton_vertices.push_back(vertex);
}

void Object::add_ms_vertex(struct VertexData vertex_data)
{
    m_mesh_skeleton_vertices.push_back(vertex_data);
}

std::vector<QVector3D> Object::get_m_Vertices()
{
    return m_mesh_vertices;
}

std::vector<QVector3D> Object::get_s_Vertices()
{
    return m_skeleton_vertices;
}

std::vector<struct VertexData> Object::get_ms_Vertices()
{
    return m_mesh_skeleton_vertices;
}

std::string Object::getName()
{
    return m_name;
}

int Object::get_m_Size()
{
    return m_mesh_vertices.size();
}

int Object::get_s_Size()
{
    return m_skeleton_vertices.size();
}

int Object::get_ms_Size()
{
    return m_mesh_skeleton_vertices.size();
}

QVector4D Object::getColor()
{
    return m_color;
}

void Object::setColor(QVector4D color)
{
    m_color = color;
}
