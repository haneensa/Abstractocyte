// todo:    1- render 3d segmentation (done)
//          2- render the skeleton
//          3- mesh normals (to be fixed)


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
        m_isRotatable(true),
        m_yaxis(0),
        m_xaxis(0),
         m_FDL_running(false)
{
    m_2dspace = new AbstractionSpace(100, 100);
    m_mesh = new Mesh();
    m_graphManager = new GraphManager();

    // todo: one graph manager, with all the graphs manipulations

    m_distance = 0.2;
    m_rotation = QQuaternion();
    //reset rotation
    m_rotation.setScalar(1.0f);
    m_rotation.setX(0.0f);
    m_rotation.setY(0.0f);
    m_rotation.setZ(0.0f);
    //reset translation
    m_translation = QVector3D(0.0, 0.0, 0.0);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(0);

    setFocusPolicy(Qt::StrongFocus);
}

GLWidget::~GLWidget()
{
    qDebug() << "~GLWidget()";

//    makeCurrent();
    delete m_2dspace;
    delete m_graphManager;
    delete m_mesh;

//    doneCurrent();
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

    // graph model matrix without rotation, apply rotation to nodes directly
    m_graph_uniforms = {m_yaxis, m_xaxis, m_mMatrix.data(), m_vMatrix.data(), m_projection.data(),
                        m_model_noRotation.data(), m_rotationMatrix};


   // m_graph_uniforms.mMatrix = m_mMatrix.data();
    m_mesh_uniforms = {m_yaxis, m_xaxis, m_mMatrix.data(), m_vMatrix.data(), m_projection.data()};

    // todo: whenver the rotation matrix changes then update
    // the nodes buffer after reseting the nodes with rotation matrix
    m_graphManager->updateUniforms(m_graph_uniforms);
    m_mesh->updateUniforms(m_mesh_uniforms);
}

void GLWidget::initializeGL()
{
    qDebug() << "initializeGL";
    initializeOpenGLFunctions();
    m_2dspace->initOpenGLFunctions();
    m_mesh->initOpenGLFunctions();
    m_graphManager->initOpenGLFunctions();

    updateMVPAttrib();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /******************** 1 Abstraction Space ********************/
    m_2dspace->initBuffer();
    /******************** 2 initialize Mesh **********************/
    m_mesh->iniShadersVBOs();
    /****************** 3 Initialize Graph  *******************/
    m_graphManager->initVBO(0);
    /**************** End data initialization *****************/

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    // to enable transparency
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void GLWidget::paintGL()
{
    // paint the text here
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);
    updateMVPAttrib();
    m_mesh->draw();

    m_graphManager->drawNodes(0);
    m_graphManager->drawEdges(0);


}

void GLWidget::resizeGL(int w, int h)
{
    // Calculate aspect ratio
    const qreal retinaScale = devicePixelRatio();
    h = (h == 0) ? 1 : h;
    glViewport(0, 0, w * retinaScale, h * retinaScale);

    m_projection.setToIdentity();
    m_projection.ortho(GLfloat(-w) / GLfloat(h),  GLfloat(w) / GLfloat(h), -1.0,  1.0f, -10.0, 10.0 );

    // set up view
    // view matrix: transform a model's vertices from world space to view space, represents the camera
    m_center = QVector3D(0.0, 0.0, 0.0);
    m_cameraPosition = QVector3D(2.5, 2.5, 2.5);
    QVector3D  cameraUpDirection = QVector3D(0.0, 1.0, 0.0);
    m_vMatrix.setToIdentity();
    m_vMatrix.lookAt(m_cameraPosition, QVector3D(0.0, 0.0, 0.0), cameraUpDirection);

    update();
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastPos = event->pos();
    setFocus();
    event->accept();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int deltaX = event->x() - m_lastPos.x();
    int deltaY = event->y() - m_lastPos.y();

    if (m_isRotatable) {
        // Mouse release position - mouse press position
        QVector2D diff = QVector2D(deltaX, deltaY);
        // Rotation axis is perpendicular to the mouse position difference
        QVector3D n = QVector3D(diff.y(), diff.x(), 0.0).normalized();
        // Accelerate angular speed relative to the length of the mouse sweep
        qreal acc = diff.length()/2.0;;

        // Calculate new rotation axis as weighted sum
        m_rotationAxis = (m_rotationAxis + n).normalized();
        // angle in degrees and rotation axis
        m_rotation = QQuaternion::fromAxisAndAngle(m_rotationAxis, acc) * m_rotation;
    } else {
        m_translation = QVector3D( m_translation.x() + deltaX/(float)width(), m_translation.y() +  -1.0 * (deltaY/(float)height()), 0.0);
    }

    m_lastPos = event->pos();
    event->accept();
    update();
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
            m_distance = 0.2f;
            update();
            break;
        case(Qt::Key_T):
            // if force directed layout running then cant change this
            if(!m_FDL_running)
                m_isRotatable = !m_isRotatable;
            break;
        case(Qt::Key_F):
            // pass rotation matrix
            m_isRotatable = false;
            m_FDL_running = true;
            updateMVPAttrib();
            m_graphManager->startForceDirectedLayout(0);
            break;
        case(Qt::Key_X):
            m_isRotatable = true;
            m_FDL_running = false;
            m_graphManager->stopForceDirectedLayout(0);
    }
}

void GLWidget::getSliderX(int value)
{
    m_xaxis = value;
    update();
}

void GLWidget::getSliderY(int value)
{
    m_yaxis = value;
    update();
}
