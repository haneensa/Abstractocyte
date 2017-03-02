#ifndef SSBO_STRUCTS_H
#define SSBO_STRUCTS_H

#include <QVector4D>

// data structure for ssbo of abstraction space
// index (0) -> astrocyte
// index (1) -> neurites
struct properties {
    QVector2D pos_alpha;
    QVector2D trans_alpha;
    QVector2D color_alpha;
    QVector2D point_size;
    QVector2D interval; // x: leftMin, y: leftMax
    QVector2D positions; // x: position 1, y: position 2
    QVector4D render_type; // mesh triangles, mesh points, points skeleton, graph (points, edges)
    QVector4D extra_info; // x: axis type, y: (if 2D space) z, w: interval of y axis type in 2D space

};

struct ast_neu_properties {
    struct properties ast;
    struct properties neu;
};

// mesh_geom.glsl
// mesh_points_geom.glsl
// lines_geom.glsl
// node_geom.glsl
// skeleton_point_geom.glsl
struct ssbo_2DState {
    struct ast_neu_properties  states; // val, alpha, color_intp, point_size, additional infos
};

// how to filter objects?
// we can just not show them in the view
// we have to excluse them from the grap as well

// data structure for ssbo of objects
// one element per obejct
struct ssbo_mesh {
    QVector4D color;
    QVector4D center;  // center.w = neurite/astrocyte
    QVector4D info;    // volume, type (axon, bouton, spine, dendrite, ..), neurite/astr, filtered
    QVector2D layout1; // layouted position (only neurties)
    QVector2D layout2; // layouted position (with astrocyte)
};


#endif // SSBO_STRUCTS_H
