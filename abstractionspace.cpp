#include "abstractionspace.h"
#include <QDebug>

/*
 * Todo: add the interpolation states for the abstract graph and the 3D data
 * complete the space
 */
AbstractionSpace::AbstractionSpace(int xdim, int ydim)
    : m_xaxis(xdim),
      m_yaxis(ydim),
      m_bindIdx(3),
      m_glFunctionsSet(false),
      m_intervalID(0)
{
    QVector2D x_interval, y_interval;
    struct properties ast1, ast2, ast3, ast4, ast5, ast6, ast7;
    struct properties neu1, neu2, neu3, neu4, neu5, neu6  ;

    std::vector<QVector2D> y_intervals;
    std::vector<QVector2D> x_intervals;

    // ##################  Y axis
    // 1) Mesh Triangles, Phong Shading -> Toon Shading
    y_interval = QVector2D(0, 20);
    y_intervals.push_back(y_interval); // 0
    ast1.pos_alpha = QVector2D(0, 1);
    ast1.trans_alpha = QVector2D(1, 1);
    ast1.color_alpha = QVector2D(0, 1);
    ast1.point_size = QVector2D(1, 1);
    ast1.interval = QVector2D(y_interval.x(), y_interval.y());
    ast1.positions = QVector2D(1, 1);
    ast1.render_type = QVector4D(1, 0, 0, 0);
    ast1.extra_info = QVector4D(1, 0, 0, 0);   // x: axis type (0: x_axis, 1: y_axis)
    m_y_axis_states[std::make_pair(y_interval.x(), y_interval.y())] = ast1;

    // 2) Mesh triangles -> point cloud -> skeleton
    y_interval = QVector2D(20, 40);
    y_intervals.push_back(y_interval); // 1
    ast2.pos_alpha = QVector2D(0.0, 0.5);
    ast2.trans_alpha = QVector2D(1, 0);     // mesh points would flip this (0->1)
    ast2.color_alpha = QVector2D(1, 1);
    ast2.point_size = QVector2D(1, 3);
    ast2.interval = QVector2D(y_interval.x(), y_interval.y());
    ast2.positions = QVector2D(1, 2);
    ast2.render_type = QVector4D(1, 1, 1, 0);
    ast2.extra_info = QVector4D(1, 0, 0, 0);   // x: axis type (0: x_axis, 1: y_axis)
    m_y_axis_states[std::make_pair(y_interval.x(), y_interval.y())] = ast2;

    // 2) Mesh point cloud -> skeleton
    y_interval = QVector2D(40, 50);
    y_intervals.push_back(y_interval); // 2
    ast3.pos_alpha = QVector2D(0.5, 1);
    ast3.trans_alpha = QVector2D(1, 1);
    ast3.color_alpha = QVector2D(1, 1);   // skeletob <-> mesh
    ast3.point_size = QVector2D(3, 6);
    ast3.interval = QVector2D(y_interval.x(), y_interval.y());
    ast3.positions = QVector2D(1, 2);
    ast3.render_type = QVector4D(0, 1, 1, 0);
    ast3.extra_info = QVector4D(1, 0, 0, 0);   // x: axis type (0: x_axis, 1: y_axis)
    m_y_axis_states[std::make_pair(y_interval.x(), y_interval.y())] = ast3;

    // 3) curved skeleto <-> simplified skeleton
    y_interval = QVector2D(50, 60);
    y_intervals.push_back(y_interval); // 3
    ast4.pos_alpha = QVector2D(0, 1);
    ast4.trans_alpha = QVector2D(1, 1);
    ast4.color_alpha = QVector2D(1, 1); // curved skeleto <-> simplified skeleton
    ast4.point_size = QVector2D(6, 6);
    ast4.interval = QVector2D(y_interval.x(), y_interval.y());
    ast4.positions = QVector2D(2, 4);
    ast4.render_type = QVector4D(0, 0, 1, 0);
    ast4.extra_info = QVector4D(1, 0, 0, 0);   // x: axis type (0: x_axis, 1: y_axis)
    m_y_axis_states[std::make_pair(y_interval.x(), y_interval.y())] = ast4;


    // 3) Skeleton -> no skeleton
    y_interval = QVector2D(60, 100);
    y_intervals.push_back(y_interval); // 4
    ast5.pos_alpha = QVector2D(0, 1);
    ast5.trans_alpha = QVector2D(1, 0);
    ast5.color_alpha = QVector2D(1, 1); // skeleto <-> no skeleton
    ast5.point_size = QVector2D(6, 6);
    ast5.interval = QVector2D(y_interval.x(), y_interval.y());
    ast5.positions = QVector2D(4, 4);
    ast5.render_type = QVector4D(0, 0, 0, 1);
    ast5.extra_info = QVector4D(1, 0, 0, 0);   // x: axis type (0: x_axis, 1: y_axis)
    m_y_axis_states[std::make_pair(y_interval.x(), y_interval.y())] = ast5;

    // 4) Skeleton -> no skeleton
    y_interval = QVector2D(80, 100);
    y_intervals.push_back(y_interval); // 4
    ast6.pos_alpha = QVector2D(0, 1);
    ast6.trans_alpha = QVector2D(1, 0);
    ast6.color_alpha = QVector2D(1, 1); // skeleto <-> no skeleton
    ast6.point_size = QVector2D(6, 6);
    ast6.interval = QVector2D(y_interval.x(), y_interval.y());
    ast6.positions = QVector2D(4, 4);
    ast6.render_type = QVector4D(0, 0, 0, 1);
    ast6.extra_info = QVector4D(1, 0, 0, 0);   // x: axis type (0: x_axis, 1: y_axis)
    m_y_axis_states[std::make_pair(y_interval.x(), y_interval.y())] = ast6;


    // ################## X axis: Neurites Abstraction
    // 1) Mesh Triangles, Phong Shading -> Toon Shading
    x_interval = QVector2D(0, 20);
    x_intervals.push_back(x_interval); // 0
    neu1.pos_alpha  = QVector2D(0, 1); // doesnt matter since both positions chosen would be the same
    neu1.trans_alpha = QVector2D(1, 1); // also doesnt matter
    neu1.color_alpha = QVector2D(0, 1); // does matter for when interpolating between toon and phone shading
    neu1.point_size = QVector2D(1, 1); // doesnt mater
    neu1.interval = QVector2D(x_interval.x(), x_interval.y());       // alpha limit, div, pos1, pos2
    neu1.positions = QVector2D(1, 1);       // alpha limit, div, pos1, pos2
    neu1.render_type = QVector4D(1, 0, 0, 0); // mesh triangles only
    neu1.extra_info = QVector4D(0, 0, 0, 0);   // x: axis type (0: x_axis, 1: y_axis)
    m_x_axis_states[std::make_pair(x_interval.x(), x_interval.y())] = neu1;

    // 2) Mesh triangles -> point cloud -> skeleton
    x_interval = QVector2D(20, 40);
    x_intervals.push_back(x_interval); // 1
    neu2.pos_alpha  = QVector2D(0, 1);
    neu2.trans_alpha = QVector2D(1, 0);
    neu2.color_alpha = QVector2D(1, 1);
    neu2.point_size = QVector2D(1, 7);
    neu2.interval = QVector2D(x_interval.x(), x_interval.y());       // alpha limit, div, pos1, pos2
    neu2.positions = QVector2D(1, 2);       // alpha limit, div, pos1, pos2
    neu2.render_type = QVector4D(1, 1, 1, 0);
    neu2.extra_info = QVector4D(0, 0, 0, 0);   // x: axis type (0: x_axis, 1: y_axis)
    m_x_axis_states[std::make_pair(x_interval.x(), x_interval.y())] = neu2;

    // 3) skeleton -> simplified skeleton
    x_interval = QVector2D(40, 60);
    x_intervals.push_back(x_interval); // 2
    neu3.pos_alpha  = QVector2D(0, 1);
    neu3.trans_alpha = QVector2D(1, 1);
    neu3.color_alpha = QVector2D(1, 1);
    neu3.point_size = QVector2D(7, 7);
    neu3.interval = QVector2D(x_interval.x(), x_interval.y());       // alpha limit, div, pos1, pos2
    neu3.positions = QVector2D(2, 4);       // alpha limit, div, pos1, pos2
    neu3.render_type = QVector4D(0, 0, 1, 0);
    neu3.extra_info = QVector4D(0, 0, 0, 0);   // x: axis type (0: x_axis, 1: y_axis)
    m_x_axis_states[std::make_pair(x_interval.x(), x_interval.y())] = neu3;


    // 4) Skeleton -> Point
    x_interval = QVector2D(60, 100);
    x_intervals.push_back(x_interval);  // 3
    neu4.pos_alpha  = QVector2D(0, 1);
    neu4.trans_alpha = QVector2D(1, 1);
    neu4.color_alpha = QVector2D(1, 1);
    neu4.point_size = QVector2D(7, 35);
    neu4.interval = QVector2D(x_interval.x(), x_interval.y());       // alpha limit, div, pos1, pos2
    neu4.positions = QVector2D(4, 3);       // alpha limit, div, pos1, pos2
    neu4.render_type = QVector4D(0, 0, 0, 1);
    neu4.extra_info = QVector4D(0, 0, 0, 0);   // x: axis type (0: x_axis, 1: y_axis)
    m_x_axis_states[std::make_pair(x_interval.x(), x_interval.y())] = neu4;

    // 5) Skeleton -> Point
    x_interval = QVector2D(80, 100);
    x_intervals.push_back(x_interval);  // 3
    neu5.pos_alpha  = QVector2D(0, 1);
    neu5.trans_alpha = QVector2D(1, 1);
    neu5.color_alpha = QVector2D(1, 1);
    neu5.point_size = QVector2D(7, 30);
    neu5.interval = QVector2D(x_interval.x(), x_interval.y());       // alpha limit, div, pos1, pos2
    neu5.positions = QVector2D(4, 3);       // alpha limit, div, pos1, pos2
    neu5.render_type = QVector4D(0, 0, 0, 1);
    neu5.extra_info = QVector4D(0, 0, 0, 0);   // x: axis type (0: x_axis, 1: y_axis)
    m_x_axis_states[std::make_pair(x_interval.x(), x_interval.y())] = neu5;


    // 2D space definition

    // 5) Skeleton Simplified 3D -> Skeleton Simplified 2D (layouted)
    y_interval = QVector2D(60, 80);
    y_intervals.push_back(y_interval); // 4
    ast7.pos_alpha = QVector2D(0, 1); // 3D to 2D lyout
    ast7.trans_alpha = QVector2D(1, 1); // nothing disappears
    ast7.color_alpha = QVector2D(1, 1); // same color
    ast7.point_size = QVector2D(6, 6); // same point size
    ast7.interval = QVector2D(y_interval.x(), y_interval.y()); // 3D -> layout 2 for astrocyte
    ast7.positions = QVector2D(5, 6); // 3D -> layout 2 for astrocyte
    ast7.render_type = QVector4D(0, 0, 0, 1); // graph
    ast7.extra_info = QVector4D(1, 0, 0, 0);   // x: axis type (0: x_axis, 1: y_axis)
    m_y_axis_states[std::make_pair(y_interval.x(), y_interval.y())] = ast7;

    // 6) Neurite Node 3D -> Neurite Node 2D (layouted) with astrocyte
    x_interval = QVector2D(98, 100);
    x_intervals.push_back(x_interval); // 4
    neu6.pos_alpha = QVector2D(0, 1); // 3D to 2D lyout
    neu6.trans_alpha = QVector2D(1, 1); // nothing disappears
    neu6.color_alpha = QVector2D(1, 1); // same color
    neu6.point_size = QVector2D(6, 6); // same point size
    // vertically instead of horizantally (Need indication)
    // flag: if 0, then verticall, else horizantally (generalization)
    neu6.interval = QVector2D(y_interval.x(), y_interval.y()); // 3D -> layout 2 for astrocyte
    neu6.positions = QVector2D(5, 6); // 3D -> layout 2 for astrocyte
    neu6.render_type = QVector4D(0, 0, 0, 1); // graph
    neu6.extra_info = QVector4D(1, 0, 0, 0);   // x: axis type (0: x_axis, 1: y_axis)
    m_x_axis_states[std::make_pair(x_interval.x(), x_interval.y())] = neu6;


    // ################## Creating the 2d space shapes
    int ID = 1;
    struct properties y_states, x_states;

    //* 1
    x_interval = x_intervals[0];
    y_interval =  y_intervals[0];
    initRect(x_interval, y_interval, ID++);
    y_states = m_y_axis_states[std::make_pair(y_interval.x(), y_interval.y())];
    x_states = m_x_axis_states[std::make_pair(x_interval.x(), x_interval.y())];
    m_IntervalXY.push_back({y_states, x_states});


    // 2
    y_interval =  y_intervals[1];
    initRect(x_interval, y_interval, ID++);
    y_states = m_y_axis_states[std::make_pair(y_interval.x(), y_interval.y())];
    m_IntervalXY.push_back({y_states, x_states});

    // 3
    y_interval =  y_intervals[2];
    initRect(x_interval, y_interval, ID++);
    y_states = m_y_axis_states[std::make_pair(y_interval.x(), y_interval.y())];
    m_IntervalXY.push_back({y_states, x_states});

    // 4
    y_interval =  y_intervals[3];
    initRect(x_interval, y_interval, ID++);
    y_states = m_y_axis_states[std::make_pair(y_interval.x(), y_interval.y())];
    m_IntervalXY.push_back({y_states, x_states});

    y_interval =  y_intervals[4];
    initRect(x_interval, y_interval, ID++);
    y_states = m_y_axis_states[std::make_pair(y_interval.x(), y_interval.y())];
    m_IntervalXY.push_back({y_states, x_states});


    //* 5
    x_interval = x_intervals[1];
    y_interval =  y_intervals[0];
    initRect(x_interval, y_interval, ID++);
    y_states = m_y_axis_states[std::make_pair(y_interval.x(), y_interval.y())];
    x_states = m_x_axis_states[std::make_pair(x_interval.x(), x_interval.y())];
    m_IntervalXY.push_back({y_states, x_states});

    // 6
    y_interval =  y_intervals[1];
    initRect(x_interval, y_interval, ID++);
    y_states = m_y_axis_states[std::make_pair(y_interval.x(), y_interval.y())];
    m_IntervalXY.push_back({y_states, x_states});

    // 7
    y_interval =  y_intervals[2];
    initRect(x_interval, y_interval, ID++);
    y_states = m_y_axis_states[std::make_pair(y_interval.x(), y_interval.y())];
    m_IntervalXY.push_back({y_states, x_states});

    // 8
    y_interval =  y_intervals[3];
    initRect(x_interval, y_interval, ID++);
    y_states = m_y_axis_states[std::make_pair(y_interval.x(), y_interval.y())];
    m_IntervalXY.push_back({y_states, x_states});

    y_interval =  y_intervals[4];
    initRect(x_interval, y_interval, ID++);
    y_states = m_y_axis_states[std::make_pair(y_interval.x(), y_interval.y())];
    m_IntervalXY.push_back({y_states, x_states});


    //* 9
    x_interval = x_intervals[2];
    y_interval =  y_intervals[0];
    initRect(x_interval, y_interval, ID++);
    y_states = m_y_axis_states[std::make_pair(y_interval.x(), y_interval.y())];
    x_states = m_x_axis_states[std::make_pair(x_interval.x(), x_interval.y())];
    m_IntervalXY.push_back({y_states, x_states});

    // 10
    y_interval =  y_intervals[1];
    initRect(x_interval, y_interval, ID++);
    y_states = m_y_axis_states[std::make_pair(y_interval.x(), y_interval.y())];
    m_IntervalXY.push_back({y_states, x_states});

    // 11
    y_interval =  y_intervals[2];
    initRect(x_interval, y_interval, ID++);
    y_states = m_y_axis_states[std::make_pair(y_interval.x(), y_interval.y())];
    m_IntervalXY.push_back({y_states, x_states});

    y_interval =  y_intervals[3];
    initRect(x_interval, y_interval, ID++);
    y_states = m_y_axis_states[std::make_pair(y_interval.x(), y_interval.y())];
    m_IntervalXY.push_back({y_states, x_states});

    y_interval =  y_intervals[4];
    initRect(x_interval, y_interval, ID++);
    y_states = m_y_axis_states[std::make_pair(y_interval.x(), y_interval.y())];
    m_IntervalXY.push_back({y_states, x_states});


    //* 9
    x_interval = x_intervals[3];
    y_interval =  y_intervals[0];
    initRect(x_interval, y_interval, ID++);
    y_states = m_y_axis_states[std::make_pair(y_interval.x(), y_interval.y())];
    x_states = m_x_axis_states[std::make_pair(x_interval.x(), x_interval.y())];
    m_IntervalXY.push_back({y_states, x_states});

    // 10
    y_interval =  y_intervals[1];
    initRect(x_interval, y_interval, ID++);
    y_states = m_y_axis_states[std::make_pair(y_interval.x(), y_interval.y())];
    m_IntervalXY.push_back({y_states, x_states});

    // 11
    y_interval =  y_intervals[2];
    initRect(x_interval, y_interval, ID++);
    y_states = m_y_axis_states[std::make_pair(y_interval.x(), y_interval.y())];
    m_IntervalXY.push_back({y_states, x_states});

    y_interval =  y_intervals[3];
    initRect(x_interval, y_interval, ID++);
    y_states = m_y_axis_states[std::make_pair(y_interval.x(), y_interval.y())];
    m_IntervalXY.push_back({y_states, x_states});

    // 2D space
    x_interval = x_intervals[4];
    y_interval =  y_intervals[5];
    initRect(x_interval, y_interval, ID++);
    y_states = m_y_axis_states[std::make_pair(y_interval.x(), y_interval.y())];
    x_states = m_x_axis_states[std::make_pair(x_interval.x(), x_interval.y())];
    m_IntervalXY.push_back({y_states, x_states});

//    // invalid states
//    ID++;
//    initTriangle(  QVector2D(62,  62),
//                   QVector2D(80, 80),
//                   QVector2D(62, 100), ID);
//    initTriangle(  QVector2D(62, 62),
//                   QVector2D(100, 62),
//                   QVector2D(80, 80), 0);
//    initTriangle(  QVector2D(80, 80),
//                   QVector2D(100, 62),
//                   QVector2D(100, 80), 0);
//    initTriangle(  QVector2D(80, 80),
//                   QVector2D(80, 100),
//                   QVector2D(62, 100), 0);
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

void AbstractionSpace::initRect(QVector2D x_interval, QVector2D y_interval, int ID)
{
    QVector2D p00 = QVector2D(x_interval.x()/100.0, y_interval.x()/100.0);
    float dimX = (x_interval.y() - x_interval.x())/100.0;
    float dimY = (y_interval.y() - y_interval.x())/100.0;


    struct abstractionPoint p = {p00, ID};
    int offset = m_vertices.size();

    m_vertices.push_back(p);        // p00

    p.point.setX(p00.x() + dimX);   // p10
    p.point.setY(p00.y());
    m_vertices.push_back(p);

    p.point.setX(p00.x());          // p01
    p.point.setY(p00.y() + dimY);
    m_vertices.push_back(p);

    p.point.setX(p00.x() + dimX);   // p11
    p.point.setY(p00.y() + dimY);
    m_vertices.push_back(p);


    m_indices.push_back(offset + 0);
    m_indices.push_back(offset + 1);
    m_indices.push_back(offset + 2);

    m_indices.push_back(offset + 1);
    m_indices.push_back(offset + 3);
    m_indices.push_back(offset + 2);
}

void AbstractionSpace::initTriangle(QVector2D coords1, QVector2D coords2,QVector2D coords3, int ID)
{
    int offset = m_vertices.size();
    struct abstractionPoint p1 = {coords1/100.0, ID};
    struct abstractionPoint p2 = {coords2/100.0, ID};
    struct abstractionPoint p3 = {coords3/100.0, ID};

    m_vertices.push_back(p1);
    m_vertices.push_back(p2);
    m_vertices.push_back(p3);

    m_indices.push_back(offset + 0);
    m_indices.push_back(offset + 1);
    m_indices.push_back(offset + 2);
}
