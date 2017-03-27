/*
 * FinalMatrix = Projection * View * Model
 * Model = RotationAroundOrigin * TranslationFromOrigin * RotationAroundObjectCenter
 */
#include <QResource>
#include <QTimer>

#include "glwidget.h"
#include "colors.h"

GLWidget::GLWidget(QWidget *parent)
    :   QOpenGLWidget(parent),
        m_yaxis(0),
        m_xaxis(0),
        m_FDL_running(false),
        m_2D(false),
        m_hover(false)
{
    // 2D abstraction space, with intervals properties intializaiton and geometry
    m_2dspace = new AbstractionSpace(100, 100);

    // objects manager with all objects data
    m_data_containter = new DataContainer();
    m_opengl_mngr = new OpenGLManager(m_data_containter, m_2dspace);

    // graph manager with 4 graphs and 2D space layouted data
    m_graphManager = new GraphManager( m_data_containter, m_opengl_mngr );

	//glycogen analysis manager with clustering
	m_glycogenAnalysisManager = new GlycogenAnalysisManager(m_data_containter->getGlycogenMapPtr(), m_data_containter->getGlycogenVertexDataPtr(), m_data_containter->getGlycogenOctree(), m_data_containter->getMeshPointer()->getVerticesList());
	m_glycogenAnalysisManager->setBoutonAndSpineOctrees(m_data_containter->getBoutonHash(), m_data_containter->getSpineHash());
	m_glycogenAnalysisManager->setMitochondriaSpatialHash(m_data_containter->getNeuroMitoHash());

    m_distance = 1.0;
    m_rotation = QQuaternion();
    //reset rotation
    m_rotation.setScalar(1.0f);
    m_rotation.setX(0.0f);
    m_rotation.setY(0.0f);
    m_rotation.setZ(0.0f);
    //reset translation
    m_translation = QVector3D(0.0, 0.0, 0.0);


    m_refresh_timer = new QTimer(this);
    connect(m_refresh_timer, SIGNAL(timeout()), this, SLOT(update()));
    m_refresh_timer->start(0);

    m_rotation_timer = new QTimer(this);
    connect(m_rotation_timer, SIGNAL(timeout()), this, SLOT(lockRotation2D()));


    m_active_graph_tab = 0;
    setFocusPolicy(Qt::StrongFocus);
}

GLWidget::~GLWidget()
{
    qDebug() << "~GLWidget()";

    makeCurrent();
    delete m_2dspace;
    delete m_graphManager;
    delete m_data_containter;
	delete m_glycogenAnalysisManager;
    doneCurrent();
}

void GLWidget::updateMVPAttrib()
{
    // calculate model view transformation
    // world/model matrix: determines the position and orientation of an object in 3D space
    m_mMatrix.setToIdentity();

    // Center Zoom
    m_mMatrix.translate(m_cameraPosition);
    m_mMatrix.scale(m_distance);
    m_mMatrix.translate(-1.0 * m_cameraPosition);

    // Translation
    m_mMatrix.translate(m_translation);

    // Model Matrix without rotation
    m_model_noRotation.setToIdentity();
    m_model_noRotation = m_mMatrix;

    // Rotation
    m_rotationMatrix.setToIdentity();
    m_rotationMatrix.translate(m_cameraPosition);
    m_rotationMatrix.rotate(m_rotation);
    m_rotationMatrix.translate(-1.0 * m_cameraPosition);
    m_mMatrix *= m_rotationMatrix;

    const qreal retinaScale = devicePixelRatio();
    QVector4D viewport = QVector4D(0, 0, width()  * retinaScale, height() * retinaScale);

    int max_astro_coverage = m_data_containter->getMaxAstroCoverage();
    int max_volume = m_data_containter->getMaxVolume();
    // graph model matrix without rotation, apply rotation to nodes directly
    m_uniforms = {m_yaxis, m_xaxis, m_mMatrix.data(), m_vMatrix.data(), m_projection.data(),
                        m_model_noRotation.data(), m_rotationMatrix, viewport, max_volume, max_astro_coverage, 0.0001};
    m_opengl_mngr->updateUniformsData(m_uniforms);
}

void GLWidget::initializeGL()
{
    qDebug() << "initializeGL";
    m_performaceMeasure.startTimer();

    initializeOpenGLFunctions();

    updateMVPAttrib();


    m_2dspace->initOpenGLFunctions();
    m_opengl_mngr->initOpenGLFunctions();
    m_graphManager->ExtractGraphFromMesh();



    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /******************** 1 Abstraction Space ********************/
    m_2dspace->initBuffer();
    emit setAbstractionData(m_2dspace);

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    // to enable transparency
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // for line smoothing
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	

    if (m_FDL_running) {
        stopForecDirectedLayout();
    }

     m_rotation_timer->start(500);

}

void GLWidget::paintGL()
{
    float fps = m_performaceMeasure.getFPS();
    if (fps > 0) {
        updateFPS(QString::number(fps));
        updateFrameTime(QString::number(1000.0/fps));
    }

    // paint the text here
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);
    updateMVPAttrib();
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_opengl_mngr->updateUniformsData(m_uniforms);
    m_opengl_mngr->drawAll();
}

void GLWidget::resizeGL(int w, int h)
{
    // Calculate aspect ratio
    const qreal retinaScale = devicePixelRatio();
    h = (h == 0) ? 1 : h;
    glViewport(0, 0, w * retinaScale, h * retinaScale);

    qDebug() <<  w * retinaScale << " " << h * retinaScale;
    m_uniforms.viewport = QVector4D(0, 0, w * retinaScale, h * retinaScale);

    qreal aspect = retinaScale * qreal(w) / qreal(h ? h : 1);
    m_projection.setToIdentity();
    m_projection.ortho(GLfloat(-w) / GLfloat(h),  GLfloat(w) / GLfloat(h), -1.0,  1.0f, -5.0, 5.0 );

    // m_projection.perspective(45.0,  aspect, -5.0, 5.0 );

    // set up view
    // view matrix: transform a model's vertices from world space to view space, represents the camera
    m_cameraPosition = QVector3D(0.5, 0.5, 0.5);
    QVector3D  cameraUpDirection = QVector3D(0.0, 1.0, 0.0);
    m_vMatrix.setToIdentity();
    m_vMatrix.lookAt(QVector3D(0.5, 0.5, 1.0) /*m_cameraPosition*/, m_cameraPosition /*center*/, cameraUpDirection);

    if (m_opengl_mngr != NULL)
        m_opengl_mngr->updateCanvasDim(w, h, retinaScale);
    update();
}

int GLWidget::pickObject(QMouseEvent *event)
{
    const qreal retinaScale = devicePixelRatio();
    GLint viewport[4]; //  return of glGetIntegerv() -> x, y, width, height of viewport
    glGetIntegerv(GL_VIEWPORT, viewport);

    int x = event->x() * retinaScale;
    int y = viewport[3] - event->y() * retinaScale;
    int hvgxID = m_opengl_mngr->processSelection(x, y);
	if (hvgxID == 16777215)
	{
		hvgxID = 0;
	}
	setHoveredID(hvgxID);
	std::string name = m_data_containter->getObjectName(hvgxID);
	QString oname = QString::fromUtf8(name.c_str());
	setHoveredName(oname);
    return hvgxID;
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastPos = event->pos();
    setFocus();

    makeCurrent();

    int hvgxID = pickObject(event);

    m_opengl_mngr->highlightObject(hvgxID);

    if ( event->modifiers() == Qt::ControlModifier) {

        if (m_selectedObjects.size() == 0) {
            clearRowsTable();
        }

        if ( m_selectedObjects.find(hvgxID) == m_selectedObjects.end() ) {
            m_selectedObjects.insert(hvgxID);
            insertInTable(hvgxID);
        }

    }
    doneCurrent();

    event->accept();
}

void GLWidget::insertInTable(int hvgxID)
{

    std::string name = m_data_containter->getObjectName(hvgxID);
    if (name == "Unknown")
        return;

    QString oname = QString::fromUtf8(name.c_str());

    QList<QStandardItem*> items;
    items.append(new QStandardItem(QString::number(hvgxID)));
    items.append(new QStandardItem(oname));
    object_clicked(items);

}

void GLWidget::lockRotation2D()
{
    if (m_FDL_running == true)
        return;

    m_rotation_timer->stop();
    m_FDL_running = true;   // run force layout


    // start a timer, and reset it whenever we are here
    // once we exceed threshold start force layouted
    // if we are below x < 50 and y < 50


    updateMVPAttrib();      // update uniforms
    m_graphManager->update2Dflag(true, m_uniforms);
    m_opengl_mngr->update2Dflag(true);

}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int deltaX = event->x() - m_lastPos.x();
    int deltaY = event->y() - m_lastPos.y();

    if (event->modifiers() == Qt::ShiftModifier) {
        makeCurrent();

        int hvgxID = pickObject(event); // hovered object

        // mark it by lighter color
        m_opengl_mngr->highlightObject(hvgxID);

        doneCurrent();
    } else if ( event->buttons() == Qt::RightButton ) {
            m_translation = QVector3D( m_translation.x() + deltaX/(float)width(),
                                       m_translation.y() +  -1.0 * (deltaY/(float)height()),
                                       0.0);

            setMouseTracking(false);

    } else if (  m_xaxis < 60 || m_yaxis < 60  ) {

        setMouseTracking(false);
        // Mouse release position - mouse press position
        QVector2D diff = QVector2D(deltaX, deltaY);
        // Rotation axis is perpendicular to the mouse position difference
        qDebug() << width() << " " <<height() << " " << event->x() << " " << event->y();
        QVector3D n;
        if (( width() - event->x()) < 100 || event->x() < 100 )
            n = QVector3D( 0.0, diff.x(), diff.y()).normalized();
        else
            n = QVector3D( diff.y() , diff.x(), 0).normalized();

        // Accelerate angular speed relative to the length of the mouse sweep
        qreal acc = diff.length()/2.0;;

        // Calculate new rotation axis as weighted sum
        m_rotationAxis = (m_rotationAxis + n).normalized();
        // angle in degrees and rotation axis
        m_rotation = QQuaternion::fromAxisAndAngle(m_rotationAxis, acc) * m_rotation;

        // whenever the rotation matrix is changed, reset the force directed layout and the nodes rotation matrix!
        // the 2D view is always locked from rotating
        // and the reset is alway rotatable

        // do wait function, if the user stayed in this view more than t seconds then do this
        // 1) stop previous layouting algorithm if running
        if (m_FDL_running) {
            stopForecDirectedLayout();
        }

         m_rotation_timer->start(500);

    } else {
        setMouseTracking(false);
    }




    m_lastPos = event->pos();
    event->accept();

    update();


    doneCurrent();

}

void GLWidget::wheelEvent(QWheelEvent *event)
{
    int delta = event->delta();

    if (event->orientation() == Qt::Vertical) {
        if (delta < 0) {
            m_distance *= 1.1;
        } else {
            m_distance *= 0.9;
        }
		m_opengl_mngr->setZoom(m_distance);
        update();
    }
}

void GLWidget::keyPressEvent(QKeyEvent *event)
{
    switch( event->key() ) {
        case(Qt::Key_S): // reset
            m_rotation.setScalar(1.0f);
            m_rotation.setX(0.0f);
            m_rotation.setY(0.0f);
            m_rotation.setZ(0.0f);
            //reset translation
            m_translation = QVector3D(0.0, 0.0, 0.0);
            //reset zoom
            m_distance = 1.0f;
			m_opengl_mngr->setZoom(m_distance);
            update();
        break;
        case(Qt::Key_X): // stop layouting algorithm
            stopForecDirectedLayout();
        break;
        case(Qt::Key_Shift): // enable hover
            setMouseTracking(true);
        case(Qt::Key_0): // switch vbo mesh rendering order
            m_opengl_mngr->renderOrderToggle();
            update();
        break;
		case(Qt::Key_P) :
			saveScreenshot();
		break;
    }
}

void GLWidget::saveScreenshot()
{
	QString fileName = QFileDialog::getSaveFileName(this,
		tr("Save Screenshot"), "",
		tr("PNG Images (*.png);;All Files (*)"));

	if (!fileName.isEmpty())
	{
		QImage screenshot = grabFramebuffer();
		screenshot.save(fileName, "png");
	}
}

void GLWidget::getSliderX(int value)
{
    if (value > 100 || value < 0) {
        return;
    }

    if (value > 98)
        value = 100;

    m_xaxis = value;

    update();
}

void GLWidget::getSliderY(int value)
{
    if (value > 100 || value < 0) {
        return;
    }

    if (value > 98)
        value = 100;
    m_yaxis = value;

    update();
}

void GLWidget::getIntervalID(int ID)
{
    m_2dspace->updateID(ID);
}

void GLWidget::getActiveGraphTab(int tab_graph)
{
   m_active_graph_tab = tab_graph;
}

void GLWidget::reset_layouting(bool flag)
{
    if (m_FDL_running) {
        stopForecDirectedLayout();
    }

     m_rotation_timer->start(10);
}

//--------------------- Graph  Parameters ---------------------
//
void GLWidget::getGraphCr(double value)
{
    qDebug() << value << " getGraphCr " << m_active_graph_tab;
    struct FDR_param fdr_params = m_graphManager->getFDRParams(m_active_graph_tab);
    if (value != 0) fdr_params.Cr = value;
    m_graphManager->updateFDRParamts(m_active_graph_tab, fdr_params);
}

void GLWidget::getGraphCa(double value)
{
    qDebug() << value << " getGraphCa " << m_active_graph_tab;

    struct FDR_param fdr_params = m_graphManager->getFDRParams(m_active_graph_tab);
    if (value != 0) fdr_params.Ca = value;
    m_graphManager->updateFDRParamts(m_active_graph_tab, fdr_params);
}

void GLWidget::getGraphAABBdim(double value)
{
    qDebug() << value << " getGraphAABBdim " << m_active_graph_tab;

    struct FDR_param fdr_params = m_graphManager->getFDRParams(m_active_graph_tab);
    if (value != 0) fdr_params.AABBdim = value;
    m_graphManager->updateFDRParamts(m_active_graph_tab, fdr_params);
}

void GLWidget::getGraphmax_distance(double value)
{
    qDebug() << value << " getGraphmax_distance " << m_active_graph_tab;

    struct FDR_param fdr_params = m_graphManager->getFDRParams(m_active_graph_tab);
    if (value != 0) fdr_params.max_distance = value;
    m_graphManager->updateFDRParamts(m_active_graph_tab, fdr_params);
}

void GLWidget::getGraphmax_vertex_movement(double value)
{
    qDebug() << value << " getGraphmax_vertex_movement " << m_active_graph_tab;

    struct FDR_param fdr_params = m_graphManager->getFDRParams(m_active_graph_tab);
    if (value != 0) fdr_params.max_vertex_movement = value;
    m_graphManager->updateFDRParamts(m_active_graph_tab, fdr_params);
}

void GLWidget::getGraphslow_factor(double value)
{
    qDebug() << value << " getGraphslow_factor " << m_active_graph_tab;

    struct FDR_param fdr_params = m_graphManager->getFDRParams(m_active_graph_tab);
    if (value != 0) fdr_params.slow_factor = value;
    m_graphManager->updateFDRParamts(m_active_graph_tab, fdr_params);
}

void GLWidget::getGraphmax_force(double value)
{
    qDebug() << value << " getGraphmax_force " << m_active_graph_tab;

    struct FDR_param fdr_params = m_graphManager->getFDRParams(m_active_graph_tab);
    if (value != 0) fdr_params.max_force = value;
    m_graphManager->updateFDRParamts(m_active_graph_tab, fdr_params);
}

void GLWidget::getGraphoriginalPosAttraction(double value)
{
    qDebug() << value << " getGraphoriginalPosAttraction " << m_active_graph_tab;

    struct FDR_param fdr_params = m_graphManager->getFDRParams(m_active_graph_tab);
    if (value != 0) fdr_params.originalPosAttraction = value;
    m_graphManager->updateFDRParamts(m_active_graph_tab, fdr_params);
}

//------------------------------------------------------
//
void GLWidget::getFilteredID(QString value)
{
    qDebug() << "Filter: " << value;
    if (m_opengl_mngr == NULL)
        return;
    // check if there are more than one ID
    QList<QString> tokens = value.split(',');
    qDebug() << tokens;

    stopForecDirectedLayout();

    bool invisibility = false; // meaning not filtered (visible)
    m_opengl_mngr->FilterByID(tokens, invisibility);

    // start force layout
    m_rotation_timer->start(0);

    update();
}

//------------------------------------------------------
//
void GLWidget::stopForecDirectedLayout()
{
    // stop force layout
//    m_refresh_timer->stop();
    for (int i = 0; i < max_graphs; ++i)
        m_graphManager->stopForceDirectedLayout(i);

    m_FDL_running = false;
}

//------------------------------------------------------
//
void GLWidget::getFilterWithChildren(bool value)
{
    // update this in openglmanager
    m_opengl_mngr->updateDisplayChildFlag(value);
}

//------------------------------------------------------
//
void GLWidget::getFilterWithParent(bool value)
{
    m_opengl_mngr->updateDisplayParentFlag(value);
}

//------------------------------------------------------
//
void GLWidget::getFilterWithSynapses(bool value)
{
    m_opengl_mngr->updateDisplaySynapseFlag(value);
}

//------------------------------------------------------
//
void GLWidget::getDepth(int d)
{
    if (d < 0)
        return;

    m_opengl_mngr->updateDepth(d);
}

//------------------------------------------------------
//
void GLWidget::getNodeSizeEncoding(QString encoding)
{
    qDebug() << encoding;
    if (encoding == "Volume")
        m_opengl_mngr->updateNodeSizeEncoding(Size_e::VOLUME);
    else if (encoding == "Astrocyte Coverage")
        m_opengl_mngr->updateNodeSizeEncoding(Size_e::ASTRO_COVERAGE);
    else if (encoding == "Synapse Size")
        m_opengl_mngr->updateNodeSizeEncoding(Size_e::SYNAPSE_SIZE);
    update();
}

//------------------------------------------------------
//
void GLWidget::getColorEncoding(QString encoding)
{
    qDebug() << encoding;
    if (encoding == "Type")
        m_opengl_mngr->updateColorEncoding(Color_e::TYPE);
    else if (encoding == "Astrocyte Coverage")
        m_opengl_mngr->updateColorEncoding(Color_e::ASTRO_COVERAGE);
    else if (encoding == "Function")
        m_opengl_mngr->updateColorEncoding(Color_e::FUNCTION);
	else if (encoding == "Glycogen Distribution")
		m_opengl_mngr->updateColorEncoding(Color_e::GLYCOGEN_MAPPING);
    update();
}

//------------------------------------------------------
//
void GLWidget::get2DtextureEncoding(QString encoding)
{
    qDebug() << encoding;
    if (encoding == "Astrocyte Coverage")
        m_opengl_mngr->update2DTextureEncoding(HeatMap2D_e::ASTRO_COVERAGE);
    else if (encoding == "Glycogen Distribution")
        m_opengl_mngr->update2DTextureEncoding(HeatMap2D_e::GLYCOGEN_MAPPING);
    update();
}


//------------------------------------------------------
//
void GLWidget::getItemChanged(QListWidgetItem* item)
{
    Qt::CheckState state =  item->checkState();
    bool flag = false;
    if (state == Qt::Unchecked) // do filter them from view
        flag = true;


   getFilteredType( item->text(), flag);

   update();
}

//------------------------------------------------------
//
void GLWidget::getFilteredType(QString value, bool flag)
{
    qDebug() << "Filter: " << value << " " << flag;
    if (m_opengl_mngr == NULL)
        return;

    stopForecDirectedLayout();

    Object_t object_type = Object_t::UNKNOWN;
    if (value == "Axons")
        object_type = Object_t::AXON;
    else if (value == "Dendrites")
        object_type = Object_t::DENDRITE;
    else if (value == "Boutons")
        object_type = Object_t::BOUTON;
    else if (value == "Spines")
        object_type = Object_t::SPINE;
    else if (value == "Mitochondria")
        object_type = Object_t::MITO;
    else if (value == "Synapse")
        object_type = Object_t::SYNAPSE;
    else if (value == "Astrocyte")
        object_type = Object_t::ASTROCYTE;

    m_opengl_mngr->FilterByType(object_type, flag);

    // start force layout
    m_rotation_timer->start(0);


    update();
}

//------------------------------------------------------
//
void GLWidget::getDoubleClickedTableView(QModelIndex index)
{
    RemoveRowAt_GL(index);
}

//------------------------------------------------------
//
void GLWidget::getDeletedHVGXID(int hvgxID)
{
    m_selectedObjects.erase(hvgxID);
}

//------------------------------------------------------
//
void GLWidget::getFitlerButtonClicked(bool)
{
    bool invisibility = false; // meaning not filtered (visible)
    m_opengl_mngr->FilterByID(m_selectedObjects, invisibility);
    update();
}

//------------------------------------------------------
//
void GLWidget::getResetFitlerButtonClicked(bool)
{
    m_opengl_mngr->showAll();
    checkAllListWidget_GL();
    update();
}

//------------------------------------------------------
//
void GLWidget::clearSelectedObjectsTable()
{
    clearRowsTable();
    m_selectedObjects.clear();
}

//------------------------------------------------------
//
void GLWidget::hideSelectedObjects()
{
    m_hide_toggle = !m_hide_toggle;
    m_opengl_mngr->VisibilityToggleSelectedObjects(m_selectedObjects, m_hide_toggle);
    update();
}

void GLWidget::highlightSelected(QModelIndex index)
{
    // signal to get the ID of selected object
    qDebug() << "highlightSelected";
    GetIDFrom(index);
}

void GLWidget::getHVGXIDAtSelectedRow(int ID)
{
    qDebug() << "** " << ID;
    m_opengl_mngr->highlightObject(ID);

}

void GLWidget::togglWeightedAstroCoverage()
{
    m_opengl_mngr->toggleWeightedCoverage();
}

void GLWidget::getglycogenMappedSelectedState(QString ID_str, bool state)
{
    int ID = ID_str.toInt();
    m_opengl_mngr->highlightObject(ID);
    m_opengl_mngr->recursiveFilter(ID, !state);
}

void GLWidget::getProximityTypeState(QString type, bool flag)
{
    int flag_value = (flag) ? 1: 0;
    if (type == "Astrocyte") {
        m_filterByProximityType.setX(flag_value);
    } else if (type == "Glycogen") {
        m_filterByProximityType.setY(flag_value);
    } else if (type == "Astrocytic Mitochondria") {
        m_filterByProximityType.setZ(flag_value);
    }
}

void GLWidget::getFilteredListByProximity()
{
    m_opengl_mngr->updateFilterByProximityType(m_filterByProximityType);

    qDebug() << "Insert in selected Objects";
    std::vector<int> ssbo_filtered_by_proximity = m_opengl_mngr->getSSBOFilterByProximity();

    for (int i = 0; i < ssbo_filtered_by_proximity.size(); ++i) {
        int hvgxID = ssbo_filtered_by_proximity[i];
        if (m_selectedObjects.find(hvgxID) == m_selectedObjects.end()) {
            m_selectedObjects.insert(hvgxID);
            insertInTable(hvgxID);
        }
    }
}

void GLWidget::updateMinProximity(double min)
{
    m_opengl_mngr->updateMinProximity(min);
}
