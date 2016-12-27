#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QWidget>
#include <QOpenGLFunctions>
#include <QMouseEvent>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

// text includes
#include <QFontMetrics>
#include <QHash>

struct FontChar {
    int width;
    GLuint textureID;
    QOpenGLVertexArrayObject *vao;
};


class GLWidget : public QOpenGLWidget, QOpenGLFunctions
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent = 0);
    ~GLWidget();
public slots:
    void getSliderX(int value);
    void getSliderY(int value);

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    bool initShader(QOpenGLShaderProgram *program, const char *vshader, const char *gshader, const char *fshader);

    void initText( const QFont &_f  );
    void renderText( float x, float y, float scaleX, float scaleY, const QString &text );
    QHash <char, FontChar> m_characters;

    QOpenGLShaderProgram        *m_program_text;

    /* testing */
    QMatrix4x4                  m_projection;

    QOpenGLVertexArrayObject    m_vao_circle;
    QOpenGLBuffer               m_vbo_circle;
    QOpenGLShaderProgram        *m_program_circle;
};


#endif // GLWIDGET_H
