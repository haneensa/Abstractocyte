#ifndef MAINOPENGL_H
#define MAINOPENGL_H

#include <QOpenGLFunctions>
#include <QMouseEvent>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QDebug>
#include <iostream>


class MainOpenGL  : public QOpenGLFunctions
{
public:
    MainOpenGL();
    bool initShader(QOpenGLShaderProgram *program, const char *vshader, const char *gshader, const char *fshader);
    void test();
    unsigned int nearestPowerOfTwo ( unsigned int _num );

};

#endif // MAINOPENGL_H
