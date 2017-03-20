#include "abstractionspace.h"
#include <QDebug>

//case 1: pos1 = mesh_vertex; break;
//case 2: pos1 = vec4(Vskeleton_vx.xyz, 1.0); break;
//case 3: pos1 = center4d; break;
//case 4: pos1 = projected_point; break;

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
    int max_neurite_nsize = 15;
    int skeleton_point_size = 4;

    QVector2D x_interval, y_interval;
    struct properties ast1, ast2, ast3, ast4, ast5, ast6;
    struct properties neu1, neu2, neu3, neu4, neu5;

    std::map<std::string, QVector2D> ast_properites_list;
    std::map<std::string, QVector2D> neu_properties_list;

    // ##################  Y axis
    // 1) Mesh Triangles, Phong Shading -> Toon Shading
    y_interval = QVector2D(0, 20);
    ast_properites_list["PHONG_TOON"] = y_interval;
    ast1.pos_alpha = QVector2D(0, 1);
    ast1.trans_alpha = QVector2D(1, 1);
    ast1.color_alpha = QVector2D(0, 1);
    ast1.point_size = QVector2D(1, 1);
    ast1.interval = QVector2D(y_interval.x(), y_interval.y());
    ast1.positions = QVector2D(1, 1);
    ast1.render_type = QVector4D(1, 0, 0, 0);
    ast1.extra_info = QVector4D(1, 0, 0, 0);   // x: axis type (0: x_axis, 1: y_axis)
    m_ast_states[std::make_pair(y_interval.x(), y_interval.y())] = ast1;

    // 2) Mesh triangles -> point skeleton
    y_interval = QVector2D(20, 40);
    ast_properites_list["MESH_SKELETON"] = y_interval;
    ast2.pos_alpha = QVector2D(0, 0.6);
    ast2.trans_alpha = QVector2D(1, 0);
    ast2.color_alpha = QVector2D(0, 0);
    ast2.point_size = QVector2D(1, skeleton_point_size);
    ast2.interval = QVector2D(y_interval.x(), y_interval.y());
    ast2.positions = QVector2D(1, 2);
    ast2.render_type = QVector4D(1, 1, 0, 0);
    ast2.extra_info = QVector4D(1, 0, 0, 0);   // x: axis type (0: x_axis, 1: y_axis)
    m_ast_states[std::make_pair(y_interval.x(), y_interval.y())] = ast2;

    // 3) curved skeleto <-> simplified skeleton
    y_interval = QVector2D(40, 60);
    ast_properites_list["PSKELETON_SIMPSKELETON"] = y_interval;
    ast4.pos_alpha = QVector2D(0, 1);
    ast4.trans_alpha = QVector2D(1, 1);
    ast4.color_alpha = QVector2D(1, 1); // curved skeleto <-> simplified skeleton
    ast4.point_size = QVector2D(skeleton_point_size, skeleton_point_size);
    ast4.interval = QVector2D(y_interval.x(), y_interval.y());
    ast4.positions = QVector2D(4, 5);
    ast4.render_type = QVector4D(0, 1, 0, 0);
    ast4.extra_info = QVector4D(1, 0, 0, 0);   // x: axis type (0: x_axis, 1: y_axis)
    m_ast_states[std::make_pair(y_interval.x(), y_interval.y())] = ast4;

    // 4) Skeleton -> no skeleton
    y_interval = QVector2D(60, 100);
    ast_properites_list["SKELETON_MARKERS"] = y_interval;
    ast5.pos_alpha = QVector2D(0, 1);
    ast5.trans_alpha = QVector2D(1, 0);
    ast5.color_alpha = QVector2D(1, 1); // skeleto <-> no skeleton
    ast5.point_size = QVector2D(skeleton_point_size, skeleton_point_size);
    ast5.interval = QVector2D(y_interval.x(), y_interval.y());
    ast5.positions = QVector2D(5, 5);
    ast5.render_type = QVector4D(0, 1, 0, 0);
    ast5.extra_info = QVector4D(1, 0, 0, 0);   // x: axis type (0: x_axis, 1: y_axis)
    m_ast_states[std::make_pair(y_interval.x(), y_interval.y())] = ast5;


    // ################## X axis: Neurites Abstraction
    // 1) Mesh Triangles, Phong Shading -> Toon Shading
    x_interval = QVector2D(0, 20);
    neu_properties_list["PHONG_TOON"] = x_interval;
    neu1.pos_alpha  = QVector2D(0, 1); // doesnt matter since both positions chosen would be the same
    neu1.trans_alpha = QVector2D(1, 1); // also doesnt matter
    neu1.color_alpha = QVector2D(0, 1); // does matter for when interpolating between toon and phone shading
    neu1.point_size = QVector2D(1, 1); // doesnt mater
    neu1.interval = QVector2D(x_interval.x(), x_interval.y());       // alpha limit, div, pos1, pos2
    neu1.positions = QVector2D(1, 1);       // alpha limit, div, pos1, pos2
    neu1.render_type = QVector4D(1, 0, 0, 0); // mesh triangles only
    neu1.extra_info = QVector4D(0, 0, 0, 0);   // x: axis type (0: x_axis, 1: y_axis)
    m_neu_states[std::make_pair(x_interval.x(), x_interval.y())] = neu1;

    // 2) Mesh triangles -> point skeleton
    x_interval = QVector2D(20, 40);
    neu_properties_list["MESH_SKELETON"] = x_interval;
    neu2.pos_alpha  = QVector2D(0, 0.6);
    neu2.trans_alpha = QVector2D(1, 0);
    neu2.color_alpha = QVector2D(0, 0);
    neu2.point_size = QVector2D(1, skeleton_point_size);
    neu2.interval = QVector2D(x_interval.x(), x_interval.y());       // alpha limit, div, pos1, pos2
    neu2.positions = QVector2D(1, 2);       // alpha limit, div, pos1, pos2
    neu2.render_type = QVector4D(1, 1, 0, 0);
    neu2.extra_info = QVector4D(0, 0, 0, 0);   // x: axis type (0: x_axis, 1: y_axis)
    m_neu_states[std::make_pair(x_interval.x(), x_interval.y())] = neu2;

    // 3) skeleton -> simplified skeleton
    x_interval = QVector2D(40, 60);
    neu_properties_list["PSKELETON_SIMPSKELETON"] = x_interval;
    neu3.pos_alpha  = QVector2D(0, 1);
    neu3.trans_alpha = QVector2D(1, 1);
    neu3.color_alpha = QVector2D(1, 1);
    neu3.point_size = QVector2D(skeleton_point_size, skeleton_point_size);
    neu3.interval = QVector2D(x_interval.x(), x_interval.y());       // alpha limit, div, pos1, pos2
    neu3.positions = QVector2D(4, 5);       // alpha limit, div, pos1, pos2
    neu3.render_type = QVector4D(0, 1, 0, 0);
    neu3.extra_info = QVector4D(0, 0, 0, 0);   // x: axis type (0: x_axis, 1: y_axis)
    m_neu_states[std::make_pair(x_interval.x(), x_interval.y())] = neu3;


    // 4) Skeleton -> Point
    x_interval = QVector2D(60, 100);
    neu_properties_list["SIMPSKELETON_POINT"] = x_interval;
    neu4.pos_alpha  = QVector2D(0, 1);
    neu4.trans_alpha = QVector2D(1, 1);
    neu4.color_alpha = QVector2D(1, 1);
    neu4.point_size = QVector2D(skeleton_point_size, max_neurite_nsize);
    neu4.interval = QVector2D(x_interval.x(), x_interval.y());       // alpha limit, div, pos1, pos2
    neu4.positions = QVector2D(5, 6);       // alpha limit, div, pos1, pos2
    neu4.render_type = QVector4D(0, 1, 0, 0);
    neu4.extra_info = QVector4D(0, 0, 0, 0);   // x: axis type (0: x_axis, 1: y_axis)
    m_neu_states[std::make_pair(x_interval.x(), x_interval.y())] = neu4;


    // ################## Creating the 2d space shapes
    int ID = 1;
    struct properties ast_properties, neu_properties;

    //****************** COL 1 **************************
    x_interval = neu_properties_list["PHONG_TOON"];
    y_interval = ast_properites_list["PHONG_TOON"];
    initRect(x_interval, y_interval, ID++);
    ast_properties = m_ast_states[std::make_pair(y_interval.x(), y_interval.y())];
    neu_properties = m_neu_states[std::make_pair(x_interval.x(), x_interval.y())];
    m_IntervalXY.push_back({ast_properties, neu_properties});

    // 2
    y_interval =  ast_properites_list["MESH_SKELETON"];
    initRect(x_interval, y_interval, ID++);
    ast_properties = m_ast_states[std::make_pair(y_interval.x(), y_interval.y())];
    m_IntervalXY.push_back({ast_properties, neu_properties});

    // 3
    y_interval =  ast_properites_list["PSKELETON_SIMPSKELETON"];
    initRect(x_interval, y_interval, ID++);
    ast_properties = m_ast_states[std::make_pair(y_interval.x(), y_interval.y())];
    m_IntervalXY.push_back({ast_properties, neu_properties});

    // 4
    y_interval =  ast_properites_list["SKELETON_MARKERS"];
    initRect(x_interval, y_interval, ID++);
    ast_properties = m_ast_states[std::make_pair(y_interval.x(), y_interval.y())];
    m_IntervalXY.push_back({ast_properties, neu_properties});

    //****************** COL 2 **************************
    x_interval = neu_properties_list["MESH_SKELETON"];
    y_interval = ast_properites_list["PHONG_TOON"];
    initRect(x_interval, y_interval, ID++);
    ast_properties = m_ast_states[std::make_pair(y_interval.x(), y_interval.y())];
    neu_properties = m_neu_states[std::make_pair(x_interval.x(), x_interval.y())];
    m_IntervalXY.push_back({ast_properties, neu_properties});

    // 6
    y_interval =  ast_properites_list["MESH_SKELETON"];
    initRect(x_interval, y_interval, ID++);
    ast_properties = m_ast_states[std::make_pair(y_interval.x(), y_interval.y())];
    m_IntervalXY.push_back({ast_properties, neu_properties});

    // 7
    y_interval =  ast_properites_list["PSKELETON_SIMPSKELETON"];
    initRect(x_interval, y_interval, ID++);
    ast_properties = m_ast_states[std::make_pair(y_interval.x(), y_interval.y())];
    m_IntervalXY.push_back({ast_properties, neu_properties});

    // 8
    y_interval =  ast_properites_list["SKELETON_MARKERS"];
    initRect(x_interval, y_interval, ID++);
    ast_properties = m_ast_states[std::make_pair(y_interval.x(), y_interval.y())];
    m_IntervalXY.push_back({ast_properties, neu_properties});


    //****************** COL 3 **************************
    x_interval = neu_properties_list["PSKELETON_SIMPSKELETON"];
    y_interval = ast_properites_list["PHONG_TOON"];
    initRect(x_interval, y_interval, ID++);
    ast_properties = m_ast_states[std::make_pair(y_interval.x(), y_interval.y())];
    neu_properties = m_neu_states[std::make_pair(x_interval.x(), x_interval.y())];
    m_IntervalXY.push_back({ast_properties, neu_properties});

    // 10
    y_interval =  ast_properites_list["MESH_SKELETON"];
    initRect(x_interval, y_interval, ID++);
    ast_properties = m_ast_states[std::make_pair(y_interval.x(), y_interval.y())];
    m_IntervalXY.push_back({ast_properties, neu_properties});

    // 11
    y_interval =  ast_properites_list["PSKELETON_SIMPSKELETON"];
    initRect(x_interval, y_interval, ID++);
    ast_properties = m_ast_states[std::make_pair(y_interval.x(), y_interval.y())];
    m_IntervalXY.push_back({ast_properties, neu_properties});

    y_interval =  ast_properites_list["SKELETON_MARKERS"];
    initRect(x_interval, y_interval, ID++);
    ast_properties = m_ast_states[std::make_pair(y_interval.x(), y_interval.y())];
    m_IntervalXY.push_back({ast_properties, neu_properties});


    //****************** COL 4 **************************
    x_interval = neu_properties_list["SIMPSKELETON_POINT"];
    y_interval = ast_properites_list["PHONG_TOON"];
    initRect(x_interval, y_interval, ID++);
    ast_properties = m_ast_states[std::make_pair(y_interval.x(), y_interval.y())];
    neu_properties = m_neu_states[std::make_pair(x_interval.x(), x_interval.y())];
    m_IntervalXY.push_back({ast_properties, neu_properties});

    // 10
    y_interval =  ast_properites_list["MESH_SKELETON"];
    initRect(x_interval, y_interval, ID++);
    ast_properties = m_ast_states[std::make_pair(y_interval.x(), y_interval.y())];
    m_IntervalXY.push_back({ast_properties, neu_properties});

    // 11
    y_interval =  ast_properites_list["PSKELETON_SIMPSKELETON"];
    initRect(x_interval, y_interval, ID++);
    ast_properties = m_ast_states[std::make_pair(y_interval.x(), y_interval.y())];
    m_IntervalXY.push_back({ast_properties, neu_properties});



    struct properties ast7, neu6 /* non repeatable properties -> no need map */;

    //******************  2D  Space **************************
    // no need for position vec
    // 4)   (y axis) Skeleton -> no skeleton
    //      (y axis) layout 1  -> layout 2
    y_interval = QVector2D(80, 100);
    ast7.pos_alpha = QVector2D(0, 1);
    ast7.trans_alpha = QVector2D(0.5, 0);
    ast7.color_alpha = QVector2D(1, 1); // skeleto <-> no skeleton
    ast7.point_size = QVector2D(skeleton_point_size, skeleton_point_size);
    ast7.interval = QVector2D(y_interval.x(), y_interval.y());
    ast7.positions = QVector2D(1, 1);
    ast7.render_type = QVector4D(0, 0,  0, 1);
    ast7.extra_info = QVector4D(0, 0, 80, 100);   // x: axis type (0: x_axis, 1: y_axis)
                                                  // y: both x and y axis
                                                  // z: left interval, w: right interval for y axis

    // 4)   (x axis) Skeleton -> point layouted
    //      (y axis) Layout <-> Layout
    x_interval = QVector2D(80, 100);
    neu6.pos_alpha  = QVector2D(0, 1);
    neu6.trans_alpha = QVector2D(1, 1);
    neu6.color_alpha = QVector2D(1, 1);
    neu6.point_size = QVector2D(skeleton_point_size, max_neurite_nsize);
    neu6.interval = QVector2D(x_interval.x(), x_interval.y());       // alpha limit, div, pos1, pos2
    neu6.positions = QVector2D(1, 1);       // alpha limit, div, pos1, pos2
    neu6.render_type = QVector4D(0, 0, 0, 1);
    neu6.extra_info = QVector4D(0, 0, 80, 100);   // x: axis type (0: x_axis, 1: y_axis)

    initRect(x_interval, y_interval, ID++);
    m_IntervalXY.push_back({ast7, neu6});


    //****************** 3D <-> 2D Transitional Space **************************

    //******************  1) (60, 60) <-> (60, 100)  **************************
        // interpolate between 3D Simplified astrocyte and no astrocyte
    // Neurites 3D simplified Skeleton
    x_interval = neu_properties_list["PSKELETON_SIMPSKELETON"];
    y_interval =  ast_properites_list["SKELETON_MARKERS"];
    ast_properties = m_ast_states[std::make_pair(y_interval.x(), y_interval.y())];
    neu_properties = m_neu_states[std::make_pair(x_interval.x(), x_interval.y())];
    m_IntervalXY.push_back({ast_properties, neu_properties});
    initLine(QVector2D(60, 60), QVector2D(60, 100), ID++);

    //******************  2) (60, 60) <-> (100, 60) **************************
    // interpolate between 3D neurite skeleton to 3D points
    // astrocyte same simplified skeleton 3d
    x_interval = neu_properties_list["SIMPSKELETON_POINT"];
    y_interval =  ast_properites_list["PSKELETON_SIMPSKELETON"];
    ast_properties = m_ast_states[std::make_pair(y_interval.x(), y_interval.y())];
    neu_properties = m_neu_states[std::make_pair(x_interval.x(), x_interval.y())];
    m_IntervalXY.push_back({ast_properties, neu_properties});
    initLine(QVector2D(60, 60), QVector2D(100, 60), ID++);


    //******************  3) (60, 60) <-> (80, 80) **************************
    // Skeleton Simplified 3D -> Skeleton Simplified 2D (layouted)
    y_interval = QVector2D(60, 80);
    ast7.pos_alpha = QVector2D(0, 1); // 3D to 2D lyout
    ast7.trans_alpha = QVector2D(1, 1); // nothing disappears
    ast7.color_alpha = QVector2D(1, 1); // same color
    ast7.point_size = QVector2D(skeleton_point_size, skeleton_point_size); // same point size
    ast7.interval = QVector2D(y_interval.x(), y_interval.y()); // 3D -> layout 2 for astrocyte
    ast7.positions = QVector2D(1, 2); // 3D -> layout 2 for astrocyte
    ast7.render_type = QVector4D(0, 0,  1, 0); // graph
    ast7.extra_info = QVector4D(1, 0, 0, 0);   // x: axis type (0: x_axis, 1: y_axis)


    // Neurite Node 3D -> Neurite Node 2D (layouted) with astrocyte
    x_interval = QVector2D(60, 80);
    neu6.pos_alpha = QVector2D(0, 1); // 3D to 2D lyout
    neu6.trans_alpha = QVector2D(1, 1); // nothing disappears
    neu6.color_alpha = QVector2D(1, 1); // same color
    neu6.point_size = QVector2D(skeleton_point_size, skeleton_point_size); // same point size
    // vertically instead of horizantally (Need indication)
    // flag: if 0, then verticall, else horizantally (generalization)
    neu6.interval = QVector2D(y_interval.x(), y_interval.y()); // 3D -> layout 2 for astrocyte
    neu6.positions = QVector2D(1, 2); // 3D -> layout 2 for astrocyte
    neu6.render_type = QVector4D(0, 0,  1, 0); // graph
    neu6.extra_info = QVector4D(0, 0, 0, 0);   // x: axis type (0: x_axis, 1: y_axis)

    // interpolate between graph 3D and 2D
    m_IntervalXY.push_back({ast7, neu6});
    initLine(QVector2D(60, 60), QVector2D(83, 83), ID++);

    //****************** 4) (60, 100) <-> (80, 100) **************************
    // interpolate between neurites skeleton 3D and 2D
    // No skeleton!!
    y_interval = QVector2D(95, 100);
    ast7.pos_alpha = QVector2D(1, 1); // 3D to 2D lyout
    ast7.trans_alpha = QVector2D(1, 1); // nothing disappears
    ast7.color_alpha = QVector2D(1, 1); // same color
    ast7.point_size = QVector2D(1, 1); // same point size
    ast7.interval = QVector2D(y_interval.x(), y_interval.y()); // 3D -> layout 2 for astrocyte
    ast7.positions = QVector2D(1, 2); // 3D -> layout 2 for astrocyte
    ast7.render_type = QVector4D(0, 0, 0, 0); // graph
    ast7.extra_info = QVector4D(0, 0, 0, 0);   // x: axis type (0: x_axis, 1: y_axis)

    // Neurite Node 3D -> Neurite Node 2D (layouted) with astrocyte
    x_interval = QVector2D(60, 80);
    neu6.pos_alpha = QVector2D(0, 1); // 3D to 2D lyout
    neu6.trans_alpha = QVector2D(0, 1); // nothing disappears
    neu6.color_alpha = QVector2D(1, 1); // same color
    neu6.point_size = QVector2D(skeleton_point_size, skeleton_point_size); // same point size
    // vertically instead of horizantally (Need indication)
    // flag: if 0, then verticall, else horizantally (generalization)
    neu6.interval = QVector2D(x_interval.x(), x_interval.y()); // 3D -> layout 2 for astrocyte
    neu6.positions = QVector2D(1, 4); // 3D -> layout 2 for astrocyte
    neu6.render_type = QVector4D(0, 0,  1, 0); // graph
    neu6.extra_info = QVector4D(0, 0, 0, 0);   // x: axis type (0: x_axis, 1: y_axis)

    // interpolate between graph 3D and 2D
    m_IntervalXY.push_back({ast7, neu6});
    initLine(QVector2D(60, 100), QVector2D(80, 100), ID++);
    // interpolate between astrocyte and nodes 3D to 2D

    //******************   5) (100, 60) <-> (100, 80) **************************
    // interpolate between marked nodes and
    y_interval = QVector2D(60, 80);
    ast7.pos_alpha = QVector2D(0, 1); // 3D to 2D lyout
    ast7.trans_alpha = QVector2D(1, 1); // nothing disappears
    ast7.color_alpha = QVector2D(1, 1); // same color
    ast7.point_size = QVector2D(skeleton_point_size, skeleton_point_size); // same point size
    ast7.interval = QVector2D(y_interval.x(), y_interval.y()); // 3D -> layout 2 for astrocyte
    ast7.positions = QVector2D(1, 3); // 3D -> layout 2 for astrocyte
    ast7.render_type = QVector4D(0, 0,  1, 0); // graph
    ast7.extra_info = QVector4D(1, 0, 0, 0);   // x: axis type (0: x_axis, 1: y_axis)

    // 6) Neurite Node 3D -> Neurite Node 2D (layouted) with astrocyte
    x_interval = QVector2D(60, 80);
    neu6.pos_alpha = QVector2D(0, 1); // 3D to 2D lyout
    neu6.trans_alpha = QVector2D(1, 1); // nothing disappears
    neu6.color_alpha = QVector2D(1, 1); // same color
    neu6.point_size = QVector2D(max_neurite_nsize, max_neurite_nsize); // same point size
    // vertically instead of horizantally (Need indication)
    // flag: if 0, then verticall, else horizantally (generalization)
    neu6.interval = QVector2D(x_interval.x(), x_interval.y()); // 3D -> layout 2 for astrocyte
    neu6.positions = QVector2D(5, 6); // 3D -> layout 2 for astrocyte
    neu6.render_type = QVector4D(0, 0,  1, 0); // graph
    neu6.extra_info = QVector4D(1, 0, 100, 100);   // x: axis type (0: x_axis, 1: y_axis)

    // interpolate between graph 3D and 2D
    m_IntervalXY.push_back({ast7, neu6});
    initLine(QVector2D(100, 60), QVector2D(100, 80), ID++);
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
    qDebug() << " AbstractionSpace::updateBuffer : " << m_intervalID;

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

void AbstractionSpace::initLine(QVector2D end1, QVector2D end2, int ID)
{
    // get the normal of the line
    // add to the end points -/+
    // construct two triangles for this line
    float dx = end2.x() - end1.x();
    float dy = end2.y() - end1.y();
    QVector2D normal = QVector2D(-dy, dx);
    normal.normalize();

    QVector2D p1, p2, p3, p4; // positions for one tirangle
    int thickness = 2;
    p1 = (end1 + normal * thickness)/100.0;
    p2 = (end1 - normal * thickness)/100.0;
    p3 = (end2 - normal * thickness)/100.0;
    p4 = (end2 + normal * thickness)/100.0;


    struct abstractionPoint p = {p1, ID};
    int offset = m_vertices.size();

    m_vertices.push_back(p);
    p.point = p2;
    m_vertices.push_back(p);
    p.point = p3;
    m_vertices.push_back(p);
    p.point = p4;
    m_vertices.push_back(p);


    m_indices.push_back(offset + 0);
    m_indices.push_back(offset + 1);
    m_indices.push_back(offset + 2);

    m_indices.push_back(offset + 2);
    m_indices.push_back(offset + 3);
    m_indices.push_back(offset + 0);
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
