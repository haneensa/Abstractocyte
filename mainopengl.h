#ifndef MAINOPENGL_H
#define MAINOPENGL_H

#include <QOpenGLFunctions>
#include <QMouseEvent>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QDebug>
#include <iostream>

// text includes
#include <QtGui/QImage>
#include <QPainter>

#include <QFontMetrics>
#include <QHash>

#include <unordered_map>
#include <array>


class MainOpenGL  : public QOpenGLFunctions
{
public:
    MainOpenGL();
    ~MainOpenGL();
    bool initShader(QOpenGLShaderProgram *program, const char *vshader, const char *gshader, const char *fshader);
    unsigned int nearestPowerOfTwo ( unsigned int _num );
    void initText( const QFont &_f  );
    void renderText( float x, float y, float scaleX, float scaleY, const QString &text );

public:
    struct FontChar {
        int width;
        GLuint textureID;
        QOpenGLVertexArrayObject *vao;
    };

    QHash <char, FontChar>      m_characters;
    QOpenGLShaderProgram        *m_program_text;
    bool                        flag;

};

#endif // MAINOPENGL_H
