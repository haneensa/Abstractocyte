#include "abstractionspace.h"
#include <QDebug>

AbstractionSpace::AbstractionSpace(int xdim, int ydim)
    : m_xaxis(xdim),
      m_yaxis(ydim),
      m_bindIdx(3),
      m_glFunctionsSet(false)
{
    m_intervalID = 0;
    struct properties ast1, ast2, ast3, ast4, ast5 /*2D*/;
    struct properties neu1, neu2, neu3, neu4 /* 2D */;

    ast1.pos_alpha = QVector2D(0, 1);
    ast1.trans_alpha = QVector2D(1.0, 1);
    ast1.color_alpha = QVector2D(0, 1); // color_intp (toon/phong)
    ast1.point_size = QVector2D(1, 1);
    ast1.extra_info = QVector4D(0, 20,  1, 1);
    ast1.render_type = QVector4D(1, 0, 0, 0);

    ast2.pos_alpha = QVector2D(0.0, 0.5);
    ast2.trans_alpha = QVector2D(1, 0); // mesh points would flip this (0->1)
    ast2.color_alpha = QVector2D(1, 1); // color_intp (toon/phong)
    ast2.point_size = QVector2D(1, 3);
    ast2.extra_info = QVector4D(20, 40,  1, 2);
    ast2.render_type = QVector4D(1, 1, 0, 0);

    ast3.pos_alpha = QVector2D(0.5, 1.0);
    ast3.trans_alpha = QVector2D(1.0, 1.0);
    ast3.color_alpha = QVector2D(1, 1.0); // skeletob <-> mesh
    ast3.point_size = QVector2D(3, 6);
    ast3.extra_info = QVector4D(40, 60,  1, 2);
    ast3.render_type = QVector4D(0, 1, 1, 0);

    ast4.pos_alpha = QVector2D(0.0, 1.0);
    ast4.trans_alpha = QVector2D(1.0, 0.0);
    ast4.color_alpha = QVector2D(1, 1); // skeletob <-> no skeleton
    ast4.point_size = QVector2D(6, 6);
    ast4.extra_info = QVector4D(60, 96, 2, 2);
    ast4.render_type = QVector4D(0, 0, 1, 0);

    ast5.pos_alpha = QVector2D(0.0, 1.0);
    ast5.trans_alpha = QVector2D(1.0, 0.0);
    ast5.color_alpha = QVector2D(1, 1); // skeletob <-> no skeleton
    ast5.point_size = QVector2D(6, 6);
    ast5.extra_info = QVector4D(80, 96, 2, 2);
    ast5.render_type = QVector4D(0, 0, 1, 0);

    neu1.pos_alpha  = QVector2D(0, 1.0);      // position interpolation
    neu1.trans_alpha = QVector2D(1.0, 1.0);    // alpha
    neu1.color_alpha = QVector2D(0, 1.0);       // color intp (toon/phong)
    neu1.point_size = QVector2D(1, 1);        // point size
    neu1.extra_info = QVector4D(0, 20,  1, 1);       // alpha limit, div, pos1, pos2
    neu1.render_type = QVector4D(1, 0, 0, 0);

    neu2.pos_alpha  = QVector2D(0, 1.0);      // position interpolation
    neu2.trans_alpha = QVector2D(1.0, 0.0);    // alpha
    neu2.color_alpha = QVector2D(1, 1.0);       // color intp (toon/phong)
    neu2.point_size = QVector2D(1, 7);        // point size
    neu2.extra_info = QVector4D(20, 60, 1, 2);       // alpha limit, div, pos1, pos2
    neu2.render_type = QVector4D(1, 1, 1, 0);

    neu3.pos_alpha  = QVector2D(0, 1);      // position interpolation
    neu3.trans_alpha = QVector2D(1, 1);    // alpha
    neu3.color_alpha = QVector2D(1, 1);       // color intp (toon/phong)
    neu3.point_size = QVector2D(7, 20);        // point size
    neu3.extra_info = QVector4D(60, 96,  2, 3);       // alpha limit, div, pos1, pos2
    neu3.render_type = QVector4D(0, 1, 0, 0);

    neu4.pos_alpha  = QVector2D(0, 1);      // position interpolation
    neu4.trans_alpha = QVector2D(1, 1);    // alpha
    neu4.color_alpha = QVector2D(1, 1);       // color intp (toon/phong)
    neu4.point_size = QVector2D(7, 20);        // point size
    neu4.extra_info = QVector4D(80, 96,  2, 3);       // alpha limit, div, pos1, pos2
    neu4.render_type = QVector4D(0, 1, 0, 1);

    // (x: 99, y: 99) -> show graph nodes and edges


    // x (0, 20)
    // y (0, 20)
    m_IntervalXY.push_back({ast1, neu1}); // 0

    // x (0, 20)
    // y (20, 40)
    m_IntervalXY.push_back({ast2, neu1}); // 1

    // x (0, 20)
    // y (40, 60)
    m_IntervalXY.push_back({ast3, neu1}); // 2

    // x (0, 20)
    // y (60, 100)
    m_IntervalXY.push_back({ast4, neu1}); // 3

    // x (20, 50)
    // y (0, 20)
    m_IntervalXY.push_back({ast1, neu2}); // 4

    // x (20, 50)
    // y (20, 40)
    m_IntervalXY.push_back({ast2, neu2}); // 6

    // x (20, 50)
    // y (40, 60)
    m_IntervalXY.push_back({ast3, neu2}); // 7

    // x (20, 50)
    // y (60, 100)
    m_IntervalXY.push_back({ast4, neu2}); // 8


    // x (50, 100)
    // y (0, 20)
    m_IntervalXY.push_back({ast1, neu3}); // 5

    // x (50, 100)
    // y (20, 40)
    m_IntervalXY.push_back({ast2, neu3}); // 9


    // x (50, 100)
    // y (40, 60)
    m_IntervalXY.push_back({ast3, neu3}); // 10

    // x (50, 100)
    // y (80, 100)
    m_IntervalXY.push_back({ast5, neu4}); // 11
}

AbstractionSpace::~AbstractionSpace()
{
    qDebug() << "~AbstractionSpace";
}

void AbstractionSpace::updateID(int ID)
{
    if (ID < 0 || ID >= m_IntervalXY.size())
        return;

    if (ID == m_intervalID) {
       return;
    }
    m_intervalID = ID;
    updateBuffer();
    qDebug() << "m_intervalID: " << ID;
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
bool AbstractionSpace::initBuffer()
{
    if (m_glFunctionsSet == false)
        return false;

    glGenBuffers(1, &m_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER,  sizeof(struct ssbo_2DState) , NULL, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bindIdx, m_buffer);
    qDebug() << "AbstractionSpace buffer size: " << sizeof(struct ssbo_2DState);
    updateBuffer();

    return true;
}

bool AbstractionSpace::updateBuffer()
{
    qDebug() << " AbstractionSpace::updateBuffer: " << m_intervalID;

    // update buffer data
    m_2DState = (struct ssbo_2DState*)malloc(sizeof(struct ssbo_2DState));
    m_2DState->states =  m_IntervalXY[m_intervalID];

    // update buffer data
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer);
    GLvoid *p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
    memcpy(p,  m_2DState, sizeof(struct ssbo_2DState));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    return true;
}
