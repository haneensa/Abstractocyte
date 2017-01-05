#ifndef MAINOPENGL_H
#define MAINOPENGL_H

// OpenGL
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

// IO
#include <QDebug>
#include <iostream>
#include <QMouseEvent>

// text includes
#include <QtGui/QImage>
#include <QPainter>

// data structures
#include <QFontMetrics>
#include <QHash>
#include <unordered_map>
#include <array>
#include <QVector3D>

// file manipulations
#include <QString>
#include <QFile>

#include "object.h"

class MainOpenGL  : public QOpenGLFunctions
{
public:
    MainOpenGL();
    ~MainOpenGL();
    bool initShader(QOpenGLShaderProgram *program, const char *vshader, const char *gshader, const char *fshader);
    unsigned int nearestPowerOfTwo ( unsigned int _num );
    void initText( const QFont &_f  );
    void renderText( float x, float y, float scaleX, float scaleY, const QString &text );
    unsigned int loadOBJ(QString path, std::vector<Object*> & objects);

public:
    struct FontChar {
        int width;
        GLuint textureID;
        QOpenGLVertexArrayObject *vao;
    };

    QHash <char, FontChar>      m_characters;
    QOpenGLShaderProgram        *m_program_text;
    QMatrix4x4                  m_projection;
    bool                        flag;

};

#endif // MAINOPENGL_H
