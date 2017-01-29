#include "abstractionspace.h"
#include <QDebug>

AbstractionSpace::AbstractionSpace(int xdim, int ydim)
    : m_xdim(xdim),
      m_ydim(ydim),
      m_bindIdx(3),
      m_glFunctionsSet(false)
{
    // val, alpha, color_intp, point_size, additional info
    // float leftMin, float leftMax, float rightMin, float rightMax
    space2d[0][0] = QVector4D(20, 100, 0, 1.0);
    space2d[0][1] = QVector4D(20, 40, 1.0, 0.0);
    space2d[0][2] = QVector4D(0, 20, 0, 1.0);
    space2d[0][3] = QVector4D(20, 40, 1, 6);
    space2d[0][4] = QVector4D(0.2, 0, 1, 2);

    space2d[1][0] = QVector4D(20, 50, 0, 1.0);      // position interpolation
    space2d[1][1] = QVector4D(40, 50, 1.0, 0.0);    // alpha
    space2d[1][2] = QVector4D(0, 20, 0, 1.0);       // point size
    space2d[1][3] = QVector4D(20, 50, 1, 7);        // color intp (toon/phong)
    space2d[1][4] = QVector4D(0.1, 50, 1, 2);       // alpha limit, div, pos1, pos2
}

AbstractionSpace::~AbstractionSpace()
{
    qDebug() << "~AbstractionSpace";
}

void AbstractionSpace::initOpenGLFunctions()
{
    m_glFunctionsSet = true;
    initializeOpenGLFunctions();
}

// *---------**---------*
// |         |          |
// |         dy         |
// |         |          |
// *---------*----dx----*
// |         |(x, y)    |
// |         |          |
// |         |          |
// *---------**---------*

void AbstractionSpace::defineAbstractionState(int x, int y, std::string name, int dx, int dy)
{

}

int AbstractionSpace::getBufferSize()
{
    return  sizeof(space2d);
}

void* AbstractionSpace::getBufferData()
{
    return space2d;
}

bool AbstractionSpace::initBuffer()
{
    if (m_glFunctionsSet == false)
        return false;

    glGenBuffers(1, &m_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER,  getBufferSize() , NULL, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bindIdx, m_buffer);
    qDebug() << "AbstractionSpace buffer size: " << getBufferSize();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer);
    GLvoid *p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
    memcpy(p,  getBufferData(),  getBufferSize());
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    return true;
}

bool AbstractionSpace::updateBuffer()
{
    // update buffer data
    return true;
}
