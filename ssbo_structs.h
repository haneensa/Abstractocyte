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
    //x: mesh, y: skeleton points, z: transitional graph, w:  2D graph
    QVector4D render_type;
    // x: axis type (0: x_axis, 1: y_axis), y: EMPTY, z and w: interval of y axis type in 2D space (not used)
    QVector4D extra_info;

};

struct ast_neu_properties {
    struct properties ast;
    struct properties neu;
};

// mesh_geom.glsl
// lines_geom.glsl
// node_geom.glsl
// skeleton_point_geom.glsl
struct ssbo_2DState {
    struct ast_neu_properties  states; // val, alpha, color_intp, point_size, additional infos
};

// data structure for ssbo of objects
// one element per obejct
struct ssbo_mesh {
    QVector4D color;   // w: is astro and neurtie nearest distance
    QVector4D center;  // center.w = type (axon, bouton, spine, dendrite, ..)
    QVector4D info;    // x: normalized volume, y: how many vertices close by astro , z: parent ID, w: filtered
    QVector2D layout1; // layouted position (only neurties)
    QVector2D layout2; // layouted position (with astrocyte)
};


#endif // SSBO_STRUCTS_H
