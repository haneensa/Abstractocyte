#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QWidget>

#include "mainopengl.h"
#include "graph.h"

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

    void setMVPAttrib(GLuint program);
    void loadMesh();

    /* mesh */
    QOpenGLVertexArrayObject    m_vao_mesh;
    QOpenGLBuffer               m_vbo_mesh;
    unsigned int                m_vertices_size;
    std::vector<Object*>        m_objects;
    bool                        m_flag_mesh_rotation;
    GLuint                      m_program_mesh;


    QOpenGLVertexArrayObject    m_vao_skeleton;
    QOpenGLBuffer               m_vbo_skeleton;
    unsigned int                m_skeleton_vertices_size;
    GLuint                      m_program_skeleton;
    std::vector<Object*>        m_skeleton_obj;


    /* matrices */
    QMatrix4x4                  m_projection;
    QMatrix4x4                  m_mMatrix;
    QMatrix4x4                  m_vMatrix;
    QVector3D                   m_cameraPosition;
    QVector3D                   m_center;
    GLuint                      uboMatrices; /* temp */

    /* rotation */
    QPoint                      m_lastPos;
    double                      m_distance;
    bool                        m_isRotatable;
    QQuaternion                 m_rotation;
    QQuaternion                  m_Oldrotation; /* temp */
    QVector3D                   m_rotationAxis;
    QVector3D                   m_translation;

    /* mouse pad */
    int                         m_yaxis;
    int                         m_xaxis;
    int                         m_state; // determine the attributes to interpolate between
};


#endif // GLWIDGET_H
