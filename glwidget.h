#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QWidget>
#include <QtOpenGL>

#include "mainopengl.h"
#include "graphmanager.h"
#include "abstractionspace.h"
#include "openglmanager.h"

class GLWidget : public QOpenGLWidget, MainOpenGL
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent = 0);
    ~GLWidget();

public slots:
    void getSliderX(int value);
    void getSliderY(int value);
    void getIntervalID(int ID);
    void getGraphParam1(double value);
    void getGraphParam2(double value);
    void getGraphParam3(double value);
    void getGraphParam4(double value);
    void getGraphParam5(double value);
    void getGraphParam6(double value);
    void getGraphParam7(double value);

signals:
    void setAbstractionData(AbstractionSpace *space_instance);

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


    AbstractionSpace                    *m_2dspace;
    bool                                m_2D;

    /* mesh */
    DataContainer                       *m_object_mngr;
    OpenGLManager                       *m_opengl_mngr;
    GraphManager                        *m_graphManager;

    struct GlobalUniforms               m_uniforms;

    /* matrices */
    QMatrix4x4                          m_projection;
    QMatrix4x4                          m_mMatrix;
    QMatrix4x4                          m_vMatrix;
    QMatrix4x4                          m_model_noRotation;
    QMatrix4x4                          m_rotationMatrix;

    QVector3D                           m_cameraPosition;


    /* rotation */
    QPoint                              m_lastPos;
    double                              m_distance;
    bool                                m_isRotatable;
    QQuaternion                         m_rotation;
    QVector3D                           m_rotationAxis;
    QVector3D                           m_translation;

    /* mouse pad */
    int                                 m_yaxis;
    int                                 m_xaxis;

    // force directed layout
    bool                                m_FDL_running;

    QTimer                              *timer;

};


#endif // GLWIDGET_H
