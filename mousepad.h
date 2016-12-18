#ifndef MOUSEPAD_H
#define MOUSEPAD_H

#include <QOpenGLWidget>
#include <QWidget>
#include <QOpenGLFunctions>
#include <QMouseEvent>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>

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

private:
    QOpenGLVertexArrayObject m_vao;
    QOpenGLShaderProgram *m_program;
};

#endif // MOUSEPAD_H
