#ifndef MOUSEPAD_H
#define MOUSEPAD_H

#include <QOpenGLWidget>
#include <QWidget>


#include "mainopengl.h"

struct point {
    float x;
    float y;
};

class MousePad : public QOpenGLWidget, MainOpenGL
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

    void renderSelection(void);
    void processSelection(float dx, float dy);

private:
    QMatrix4x4                  m_projection;

    QOpenGLVertexArrayObject    m_vao_circle;
    QOpenGLBuffer               m_vbo_circle;
    QOpenGLShaderProgram        *m_program_circle;

    QOpenGLVertexArrayObject    m_vao_selection;
    QOpenGLShaderProgram        *m_program_selection;

    struct point                circle;
    int                         m_w;
    int                         m_h;

    bool                        m_slider_enabled;

};

#endif // MOUSEPAD_H
