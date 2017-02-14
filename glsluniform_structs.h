#ifndef GLSLUNIFORM_STRUCTS_H
#define GLSLUNIFORM_STRUCTS_H

#include <QMatrix4x4>

// todo: combine them into one uniform
struct MeshUniforms {
    int y_axis;
    int x_axis;
    float* mMatrix;
    float* vMatrix;
    float* pMatrix;
};

struct GraphUniforms {
    int y_axis;
    int x_axis;
    float* mMatrix;
    float* vMatrix;
    float* pMatrix;
    float* modelNoRotMatrix;
    QMatrix4x4 rMatrix;
};

#endif // GLSLUNIFORM_STRUCTS_H
