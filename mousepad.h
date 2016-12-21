#ifndef MOUSEPAD_H
#define MOUSEPAD_H

#include <QOpenGLWidget>
#include <QWidget>
#include <QOpenGLFunctions>
#include <QMouseEvent>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

class MousePad : public QOpenGLWidget, QOpenGLFunctions
{
    Q_OBJECT

public:
    MousePad(QWidget *parent = 0);
    ~MousePad();

public slots:
    void setSlotsX(int value);
    void setSlotsY(int value);

signals:
    void setSignalX(int value);
    void setSignalY(int value);
    void setSliderX(int value);
    void setSliderY(int value);

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;

    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent*event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    bool initShader(QOpenGLShaderProgram *program, const char *vshader, const char *gshader, const char *fshader);
    void renderSelection(void);
    void processSelection(float xx, float yy);

private:
    QMatrix4x4                  m_projection;
    QOpenGLVertexArrayObject    m_vao_circlue;
    QOpenGLBuffer               m_vbo_circlue;
    QOpenGLShaderProgram        *m_program_circle;

    QOpenGLVertexArrayObject    m_vao_selection;
    QOpenGLBuffer               m_vbo_selection;
    QOpenGLShaderProgram        *m_program_selection;

};

#endif // MOUSEPAD_H
