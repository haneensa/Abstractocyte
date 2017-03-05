#ifndef GLSLUNIFORM_STRUCTS_H
#define GLSLUNIFORM_STRUCTS_H

#include <QMatrix4x4>

struct GlobalUniforms {
    int y_axis;
    int x_axis;
    float* mMatrix;
    float* vMatrix;
    float* pMatrix;
    float* modelNoRotMatrix;
    QMatrix4x4 rMatrix;
    QVector4D viewport;
};

#endif // GLSLUNIFORM_STRUCTS_H
