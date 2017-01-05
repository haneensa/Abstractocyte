#include "object.h"
#include <QDebug>

Object::Object(std::string name)
    :  m_vbo( QOpenGLBuffer::VertexBuffer )
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
    m_vbo.destroy();
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

bool Object::update_vbo()
{
    if ( !m_vbo.bind() ) {
        qDebug() << "Could not bind vertex buffer to the context.";
        return false;
    }

    m_vbo.allocate(&m_vertices[0], m_vertices.size() * sizeof(QVector3D));
    m_vbo.release();

    return true;
}

bool Object::allocate_vbo(QOpenGLShaderProgram  *program_mesh)
{
    m_vbo.create();
    m_vbo.setUsagePattern( QOpenGLBuffer::StaticDraw);
    if ( !m_vbo.bind() ) {
        qDebug() << "Could not bind vertex buffer to the context.";
        return false;
    }


    m_vbo.allocate(&m_vertices[0], m_vertices.size() * sizeof(QVector3D));
    program_mesh->bind();
    program_mesh->enableAttributeArray("posAttr");
    program_mesh->setAttributeBuffer("posAttr", GL_FLOAT, 0, 3);

    program_mesh->release();
    m_vbo.release();

    return true;
}

void Object::draw()
{
    glDrawArrays(GL_TRIANGLES, 0, m_vertices.size() );
}

std::string Object::getName()
{
    return m_name;
}

int Object::getSize()
{
    return m_vertices.size();
}
