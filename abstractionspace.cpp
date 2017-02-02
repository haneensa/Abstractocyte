#include "abstractionspace.h"
#include <QDebug>

AbstractionSpace::AbstractionSpace(int xdim, int ydim)
    : m_xaxis(xdim),
      m_yaxis(ydim),
      m_bindIdx(3),
      m_glFunctionsSet(false)
{


    // position interpolation (pos1, pos2), alpha, color_intp, point_size, additional info
    // float leftMin, float leftMax, float rightMin, float rightMax
    m_2DState.states[0][0] = QVector4D(20, 100, 0, 1.0);
    m_2DState.states[0][1] = QVector4D(20, 40, 1.0, 0.0);
    m_2DState.states[0][2] = QVector4D(0, 20, 0, 1.0);
    m_2DState.states[0][3] = QVector4D(20, 40, 1, 6);
    m_2DState.states[0][4] = QVector4D(0.2f, 0, 1, 2);

    m_2DState.states[1][0] = QVector4D(20, 50, 0, 1.0);      // position interpolation
    m_2DState.states[1][1] = QVector4D(40, 50, 1.0, 0.0);    // alpha
    m_2DState.states[1][2] = QVector4D(0, 20, 0, 1.0);       // color intp (toon/phong)
    m_2DState.states[1][3] = QVector4D(20, 50, 1, 7);        // point size
    m_2DState.states[1][4] = QVector4D(0.1f, 0.05f, 1, 2);       // alpha limit, div, pos1, pos2
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
// |         dy         |h3
// |         |          |
// *---------*----dx----*
// |         |(x, y)    |
// |         |          |
// |         |          |
// *---------**---------*

void AbstractionSpace::defineAbstractionState(int x, int y, std::string name, int dx, int dy)
{

}
void AbstractionSpace::defineQuadrant(QVector2D  leftMin, int dim, struct ssbo_2DState data)
{
    std::pair<int, int> index;
    // using leftMin and dim, compute the index of this qudrant
    m_statesMap.insert( std::make_pair(index, data) );
}

int AbstractionSpace::getBufferSize()
{
    return  sizeof(m_2DState);
}

void* AbstractionSpace::getBufferData()
{
    return m_2DState.states;
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
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer);
    GLvoid *p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
    memcpy(p,  getBufferData(),  getBufferSize());
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    return true;
}

void AbstractionSpace::updateXYaxis(int xaxis, int yaxis)
{
    m_xaxis = xaxis;
    m_yaxis = yaxis;

    // update the 2d space vlues
    int leftMin = 50;
    int leftMax = 100;
    struct properties p00;
    struct properties p10, p20, p30;
    struct properties p01, p02, p03;

    p00  = {1, 1.0, 1.0, 1.0, 1}; // concrete phong shading

    p10 = {1, 1.0, 1.0, 0.0, 1}; // toon shading, skeleton start appearing
    p20 = {2, 1.0, 1.0, 0.0, 7}; // skeleton, toon shading
    p30 = {3, 1.0, 1.0, 0.0, 30}; // skeleton, toon shading

    p01 = {1, 1.0, 1.0, 0.0, 1}; // toon shading, skeleton start appearing
    p02 = {2, 0.5, 0.0, 0.0, 6}; // triangle geometry disappearing, skeleton, toon shading
    p03 = {2, 1.0, 1.0, 0.0, 7}; // skeleton only


    m_2DState.states[1][0] = QVector4D(leftMin, leftMax, 1-p20.pos_alpha, p30.pos_alpha);      // position interpolation
    m_2DState.states[1][1] = QVector4D(leftMin, leftMax, p20.trans_alpha,  p30.trans_alpha);    // alpha
    m_2DState.states[1][2] = QVector4D(leftMin, leftMax, 1, 1);        // color intp (toon/phong)
    m_2DState.states[1][3] = QVector4D(leftMin, leftMax, p20.point_size, p30.point_size);       // point size
    m_2DState.states[1][4] = QVector4D(0.1f, 50, 2, p30.source);       // alpha limit, div, pos1, pos2


    // if we move to new quadrant then:
    updateBuffer();
}
