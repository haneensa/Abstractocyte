#include "abstractionspace.h"
#include <QDebug>

AbstractionSpace::AbstractionSpace(int xdim, int ydim)
    : m_xaxis(xdim),
      m_yaxis(ydim),
      m_bindIdx(3),
      m_glFunctionsSet(false),
      m_prevIntvMaxX(0),
      m_prevIntvMaxY(0)
{

    struct properties ast1, ast2, ast3, ast4;
    struct properties neu1, neu2, neu3;

    ast1.pos_alpha = QVector4D(0, 20, 0.0, 1.0);
    ast1.trans_alpha = QVector4D(0, 20, 1.0, 1.0);
    ast1.color_alpha = QVector4D(0, 20, 0, 1.0); // color_intp (toon/phong)
    ast1.point_size = QVector4D(0, 20, 1, 1);
    ast1.extra_info = QVector4D(0.0f, 0, 1, 1);
    ast1.render_type = QVector4D(1, 0, 0, 0);

    ast2.pos_alpha = QVector4D(20, 40, 0.0, 0.5);
    ast2.trans_alpha = QVector4D(20, 40, 1.0, 0.0); // mesh points would flip this (0->1)
    ast2.color_alpha = QVector4D(20, 40, 1, 1.0); // color_intp (toon/phong)
    ast2.point_size = QVector4D(20, 40, 1, 3);
    ast2.extra_info = QVector4D(0.0f, 0, 1, 2);
    ast2.render_type = QVector4D(1, 1, 0, 0);

    ast3.pos_alpha = QVector4D(40, 90, 0.5, 1.0);
    ast3.trans_alpha = QVector4D(40, 90, 1.0, 1.0);
    ast3.color_alpha = QVector4D(40, 90, 1, 1.0); // color_intp (toon/phong)
    ast3.point_size = QVector4D(40, 90, 3, 6);
    ast3.extra_info = QVector4D(0.0f, 0, 1, 2);
    ast3.render_type = QVector4D(0, 1, 1, 0);

    ast4.pos_alpha = QVector4D(90, 99, 0.0, 1.0);
    ast4.trans_alpha = QVector4D(90, 99, 1.0, 0.0);
    ast4.color_alpha = QVector4D(90, 99, 1, 1); // color_intp (toon/phong)
    ast4.point_size = QVector4D(90, 99, 6, 6);
    ast4.extra_info = QVector4D(0.05f, 0, 2, 2);
    ast4.render_type = QVector4D(0, 0, 1, 0);

    neu1.pos_alpha  = QVector4D(0, 20, 0, 1.0);      // position interpolation
    neu1.trans_alpha = QVector4D(0, 20, 1.0, 1.0);    // alpha
    neu1.color_alpha = QVector4D(0, 20, 0, 1.0);       // color intp (toon/phong)
    neu1.point_size = QVector4D(0, 20, 1, 1);        // point size
    neu1.extra_info = QVector4D(0.0f, 0.0f, 1, 1);       // alpha limit, div, pos1, pos2
    neu1.render_type = QVector4D(1, 0, 0, 0);

    neu2.pos_alpha  = QVector4D(20, 50, 0, 1.0);      // position interpolation
    neu2.trans_alpha = QVector4D(20, 50, 1.0, 0.0);    // alpha
    neu2.color_alpha = QVector4D(20, 50, 1, 1.0);       // color intp (toon/phong)
    neu2.point_size = QVector4D(20, 50, 1, 7);        // point size
    neu2.extra_info = QVector4D(0.0f, 0.0f, 1, 2);       // alpha limit, div, pos1, pos2
    neu2.render_type = QVector4D(1, 1, 1, 0);

    neu3.pos_alpha  = QVector4D(50, 100, 0, 1);      // position interpolation
    neu3.trans_alpha = QVector4D(50, 100, 1, 1);    // alpha
    neu3.color_alpha = QVector4D(50, 100, 1, 1);       // color intp (toon/phong)
    neu3.point_size = QVector4D(50, 100, 7, 25);        // point size
    neu3.extra_info = QVector4D(0.0f, 0.0f, 2, 3);       // alpha limit, div, pos1, pos2
    neu3.render_type = QVector4D(0, 1, 0, 0);

    Interval x_intervals[] = { {0, 20, neu1}, {20, 50, neu2}, {50, 100, neu3} };
    m_intervalX.insertIntervals(x_intervals, 3);

    Interval y_intervals[] = { {0, 20, ast1}, {20, 40, ast2}, {40, 90, ast3}, {90, 100, ast4} };
    m_intervalY.insertIntervals(y_intervals, 4);
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

    updateYaxis(0);
    updateXaxis(0);

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

void AbstractionSpace::updateXaxis(int xaxis)
{
    m_xaxis = xaxis;

    // update the 2d space vlues
    Interval *res = m_intervalX.getInterval(xaxis);
    if (res == NULL) {
         qDebug()  << "\nNo overlapping interval";
         return;
    }

    if (m_prevIntvMaxX == res->high)
        return;

    qDebug()  << "\nOverlaps with [" << res->low << ", " << res->high << "]";

    struct properties states = res->int_properties;

    m_2DState.states[1][0] = states.pos_alpha;      // position interpolation
    m_2DState.states[1][1] = states.trans_alpha;    // alpha
    m_2DState.states[1][2] = states.color_alpha;       // color intp (toon/phong)
    m_2DState.states[1][3] = states.point_size;        // point size
    m_2DState.states[1][4] = states.extra_info;       // alpha limit, div, pos1, pos2
    m_2DState.states[1][5] = states.render_type;


    // if we move to new quadrant then:
    updateBuffer();
}

void AbstractionSpace::updateYaxis(int yaxis)
{
    m_yaxis = yaxis;

    // update the 2d space vlues

    Interval *res;
    res = m_intervalY.getInterval(yaxis);
    if (res == NULL) {
         qDebug()  << "\nNo overlapping interval";
         return;
    }

    if (m_prevIntvMaxY == res->high)
        return;

    qDebug()  << "\nOverlaps with [" << res->low << ", " << res->high << "]";

    struct properties states = res->int_properties;
    m_2DState.states[0][0] = states.pos_alpha;
    m_2DState.states[0][1] = states.trans_alpha;
    m_2DState.states[0][2] = states.color_alpha; // color_intp (toon/phong)
    m_2DState.states[0][3] = states.point_size;
    m_2DState.states[0][4] = states.extra_info;
    m_2DState.states[0][5] = states.render_type;

    // if we move to new quadrant then:
    updateBuffer();
}
