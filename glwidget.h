#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QWidget>
#include <QtOpenGL>

#include "mainopengl.h"
#include "graphmanager.h"
#include "abstractionspace.h"
#include "openglmanager.h"
#include "glycogenanalysismanager.h"

class GLWidget : public QOpenGLWidget, MainOpenGL
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent = 0);
    ~GLWidget();

	GlycogenAnalysisManager* getGlycogenAnalysisManager() { return m_glycogenAnalysisManager; }
	OpenGLManager*			 getOpenGLManager() { return m_opengl_mngr; }
	float getZoomFactor() { return m_distance; }

    int pickObject(QMouseEvent *event);

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
    void lockRotation2D();

    void getFilteredType(QString value);
    void getFilteredID(QString value);

    void getFilterWithChildren(bool value);
    void getFilterWithParent(bool value);
    void getFilterWithSynapses(bool value);

    void getDepth(int d);

    void getNodeSizeEncoding(QString);
    void getColorEncoding(QString);

signals:
    void setAbstractionData(AbstractionSpace *space_instance);
    void setHoveredID(int);
    void setHoveredName(QString);

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


    void stopForecDirectedLayout();

    AbstractionSpace                    *m_2dspace;
    bool                                m_2D;

    /* mesh */
    DataContainer                       *m_data_containter;
    OpenGLManager                       *m_opengl_mngr;
    GraphManager                        *m_graphManager;
	GlycogenAnalysisManager				*m_glycogenAnalysisManager;

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

    QTimer                              *m_refresh_timer;
    QTimer                              *m_rotation_timer;

    std::vector<int>                    m_selectedObjects;

    bool                                m_hover;
};


#endif // GLWIDGET_H
