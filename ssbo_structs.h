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
    QVector4D extra_info;
    QVector4D render_type; // mesh triangles, mesh points, points skeleton, graph (points, edges)
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


// data structure for ssbo of objects
// one element per obejct
struct ssbo_mesh {
    QVector4D color;
    QVector4D center;   // center.w = neurite/astrocyte
    QVector4D info;     // volume, type (axon, bouton, spine, dendrite, ..), neurite/astr, ?
    QVector2D layout1; // layouted position (only neurties)
    QVector2D layout2; // layouted position (with astrocyte)
};


#endif // SSBO_STRUCTS_H
