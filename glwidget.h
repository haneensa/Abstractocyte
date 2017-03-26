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
#include "performancerate.h"

class GLWidget : public QOpenGLWidget, MainOpenGL
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent = 0);
    ~GLWidget();

	GlycogenAnalysisManager* getGlycogenAnalysisManager() { return m_glycogenAnalysisManager; }
	OpenGLManager*			 getOpenGLManager() { return m_opengl_mngr; }
	DataContainer*			 getDataContainer() { return m_data_containter; }
	float getZoomFactor() { return m_distance; }

    int pickObject(QMouseEvent *event);
    void insertInTable(int);

public slots:
    void getSliderX(int value);
    void getSliderY(int value);
    void getIntervalID(int ID);

    void getActiveGraphTab(int);
    void reset_layouting(bool);
    void getGraphCr(double value);
    void getGraphCa(double value);
    void getGraphAABBdim(double value);
    void getGraphmax_distance(double value);
    void getGraphmax_vertex_movement(double value);
    void getGraphslow_factor(double value);
    void getGraphmax_force(double value);
    void getGraphoriginalPosAttraction(double value);

    void lockRotation2D();

    void getFilteredType(QString value, bool);
    void getFilteredID(QString value);

    void getFilterWithChildren(bool value);
    void getFilterWithParent(bool value);
    void getFilterWithSynapses(bool value);

    void getDepth(int d);

    void getNodeSizeEncoding(QString);
    void getColorEncoding(QString);
    void get2DtextureEncoding(QString);

    void getItemChanged(QListWidgetItem*);
    void getDoubleClickedTableView(QModelIndex);
    void highlightSelected(QModelIndex);

    void getDeletedHVGXID(int);
    void getFitlerButtonClicked(bool);
    void getResetFitlerButtonClicked(bool);
    void clearSelectedObjectsTable();

    void hideSelectedObjects();
    void getHVGXIDAtSelectedRow(int);
    void togglWeightedAstroCoverage();
    void getglycogenMappedSelectedState(QString, bool);

signals:
    void setAbstractionData(AbstractionSpace *space_instance);
    void setHoveredID(int);
    void setHoveredName(QString);
    void object_clicked(QList<QStandardItem*>);
    void clearRowsTable();
    void RemoveRowAt_GL(QModelIndex);
    void GetIDFrom(QModelIndex);
    void checkAllListWidget_GL();
    void updateFPS(QString);
    void updateFrameTime(QString);


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

	void saveScreenshot();

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

    std::set<int>                       m_selectedObjects;

    bool                                m_hover;

    bool                                m_hide_toggle;

    int                                 m_active_graph_tab;

    PerformanceRate                     m_performaceMeasure;

};


#endif // GLWIDGET_H
