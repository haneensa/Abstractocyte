#include "object.h"
#include <QDebug>
#include <algorithm>

#include "colors.h"

Object::Object(std::string name, int idx, int ID)
{
    if (name[name.size()-1] == '\n') {
        name.erase(name.size()-1, name.size()-1);
    }

    m_name  = name;
    m_idxID = idx;
    m_ID    = ID;
    m_object_t = getObjectType(m_name);
    m_color = QVector4D(1.0, 1.0, 0.0, 1.0);
    qDebug() << "create " << m_name.data();
}

Object::~Object()
{
}

Object_t Object::getObjectType(std::string name)
{
    // make it generic later
    std::string code = m_name.substr(0, 3);
    std::transform(code.begin(), code.end(), code.begin(), ::tolower);

    if (code.compare("ast") == 0) {
       return Object_t::ASTROCYTE;
    } else if (code.compare("axo") == 0) {
        return Object_t::AXON;
    } else if (code.compare("den") == 0) {
        return Object_t::DENDRITE;
    } else if (code.compare("spi") == 0) {
        return Object_t::SPINE;
    } else if (code.compare("bou") == 0 || code.compare("buo") == 0 ) {
        return Object_t::BOUTON;
    } else if (code.compare(0, 1, "m") == 0) {
        return Object_t::MITO;
    } else if ((code.compare(0, 1, "d") == 0 && code.compare(0, 1, "e")) || code.compare("syn") == 0 ) {
        return Object_t::SYNAPSE;
    } else {
        return Object_t::UNKNOWN;
    }
}

void Object::add_m_vertex(QVector3D vertex)
{
    m_mesh_vertices.push_back(vertex);
}

void Object::add_s_vertex(struct SkeletonVertex vertex)
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

std::vector<struct SkeletonVertex> Object::get_s_Vertices()
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

size_t Object::get_m_Size()
{
    return m_mesh_vertices.size();
}

size_t Object::get_s_Size()
{
    return m_skeleton_vertices.size();
}

size_t Object::get_ms_Size()
{
    return m_mesh_skeleton_vertices.size();
}

QVector4D Object::getColor()
{
    switch (m_object_t) {
    case Object_t::ASTROCYTE:
        m_color = firebrick;
        break;
    case Object_t::AXON:
        m_color = lightseagreen;
        break;
    case Object_t::DENDRITE:
        m_color = gold;
        break;
    case Object_t::SPINE:
        m_color = orange;
        break;
    case Object_t::BOUTON:
        m_color = mediumaquamarine;
        break;
    case Object_t::MITO:
        m_color = mediumslateblue;
        break;
    case Object_t::SYNAPSE:
        m_color = lightgoldenrodyellow;
        break;
    case Object_t::GLYCOGEN:
        m_color = red;
        break;
    default:
        qDebug() << m_name.data();
        m_color = gray;
        break;
    };

    return m_color;
}

void Object::setColor(QVector4D color)
{
    m_color = color;
}
