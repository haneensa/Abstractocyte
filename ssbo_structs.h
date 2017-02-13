#ifndef SSBO_STRUCTS_H
#define SSBO_STRUCTS_H

#include <QVector4D>

// data structure for ssbo of abstraction space
// index (0) -> astrocyte
// index (1) -> neurites
struct ssbo_2DState {
    QVector4D  states[2][6]; // val, alpha, color_intp, point_size, additional infos
};


// data structure for ssbo of objects
// one element per obejct
struct ssbo_mesh {
    QVector4D color;
    QVector4D center;   // center.w = neurite/astrocyte
    QVector4D info;     // volume, type (axon, bouton, spine, dendrite, ..), neurite/astr, ?
};

#endif // SSBO_STRUCTS_H
