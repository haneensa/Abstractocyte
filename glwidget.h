#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QWidget>
#include <QtOpenGL>

#include "graphmanager.h"
#include "mainopengl.h"
#include "mesh.h"
#include "abstractionspace.h"

#define GL_GLEXT_PROTOTYPES

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

    void updateMVPAttrib();
    void loadMesh();

    AbstractionSpace            *m_2dspace;

    /* mesh */
    Mesh                        m_mesh;
    GraphManager                *m_graphManager;
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
