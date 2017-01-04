#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QWidget>

#include "mainopengl.h"



class GLWidget : public QOpenGLWidget, MainOpenGL
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
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

    void setMVPAttrib(QOpenGLShaderProgram *program);
    void loadMesh();

    /* mesh */
    QOpenGLVertexArrayObject    m_vao_mesh;
    QOpenGLBuffer               m_vbo_mesh;
    QOpenGLShaderProgram        *m_program_mesh;
    std::vector<QVector3D>      vertices;

    /* matrices */
    QMatrix4x4                  m_projection;
    QMatrix4x4                  m_mMatrix;
    QMatrix4x4                  m_vMatrix;
    QVector3D                   m_cameraPosition;
    QVector3D                   m_center;

    /* rotation */
    QPoint                      m_lastPos;
    double                      m_distance;
    bool                        m_isRotatable;
    QQuaternion                 m_rotation;
    QVector3D                   m_rotationAxis;
    QVector3D                   m_translation;
};


#endif // GLWIDGET_H
