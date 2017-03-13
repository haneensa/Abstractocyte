#ifndef GLSLUNIFORM_STRUCTS_H
#define GLSLUNIFORM_STRUCTS_H

#include <QMatrix4x4>

struct GlobalUniforms {
 /* 0 */    int y_axis;
 /* 1 */    int x_axis;
 /* 2 */    float* mMatrix;
 /* 3 */    float* vMatrix;
 /* 4 */    float* pMatrix;
 /* 5 */    float* modelNoRotMatrix;
 /* 6 */    QMatrix4x4 rMatrix;
 /* 7 */    QVector4D viewport;
 /* 8 */    int max_volume;  // maximum volume from displayed objects
 /* 9 */    int max_astro_coverage; // maximum vertices from neurites covered by astrocyte
 /* 10*/    float AST_DIST_THRSH; // distance to strocyte threshold
};

#endif // GLSLUNIFORM_STRUCTS_H
