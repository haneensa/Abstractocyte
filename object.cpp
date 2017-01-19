#include "object.h"
#include <QDebug>
#include <algorithm>

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
    } else if (code.compare("bou") == 0) {
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
        m_color = QVector4D(1.0f, 0.0f, 0.0f, 1.0f);
        break;
    case Object_t::AXON:
        m_color = QVector4D(1.0f, 0.388f, 0.278f, 1.0f);  // Forest Green
        break;
    case Object_t::DENDRITE:
        m_color = QVector4D(0.125f, 0.698f, 0.667f, 1.0f);  // Tomato
        break;
    case Object_t::SPINE:
        m_color = QVector4D(1.0f, 0.549f, 0.0f, 1.0f);     // Dark Orange
        break;
    case Object_t::BOUTON:
        m_color = QVector4D(0.196f, 0.804f, 0.196f, 1.0f);  // Lime Green
        break;
    case Object_t::MITO:
        m_color = QVector4D(1.0f, 0.078f, 0.576f, 1.0f);
        break;
    case Object_t::SYNAPSE:
        m_color = QVector4D(1.0f, 1.0f, 0.0f, 1.0f);
        break;
    case Object_t::GLYCOGEN:
        m_color = QVector4D(1.0f, 0.0f, 1.0f, 1.0f);
        break;
    default:
        m_color = QVector4D(0.0f, 0.0f, 0.0f, 1.0f);
        break;
    };

    return m_color;
}

void Object::setColor(QVector4D color)
{
    m_color = color;
}
