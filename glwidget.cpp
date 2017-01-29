// todo:    1- render 3d segmentation (done)
//          2- render the skeleton
//          3- mesh normals (to be fixed)


/*
 * FinalMatrix = Projection * View * Model
 * Model = RotationAroundOrigin * TranslationFromOrigin * RotationAroundObjectCenter
 */
#include "glwidget.h"
#include <QResource>
#include "colors.h"

GLWidget::GLWidget(QWidget *parent)
    :   QOpenGLWidget(parent),
        m_isRotatable(true),
        m_yaxis(0),
        m_xaxis(0),
        m_state(0)
{
    m_2dspace = new AbstractionSpace(100, 100);


    // to do: combine all these files in one .obj file
    QString path= "://data/skeleton_astrocyte_m3/mouse3_astro_skelton.obj";
    m_mesh.loadObj(path);
    path = "://data/mouse03_skeleton_centroid.obj";
    m_mesh.loadObj(path);
    path = "://data/mouse03_astro_skeleton.sk";
    m_mesh.loadSkeletonPoints(path); // 11638884, 19131720
    path = "://data/mouse03_skeletons.sk";
    m_mesh.loadSkeletonPoints(path); // 11638884, 19131720

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
}

GLWidget::~GLWidget()
{
    qDebug() << "~GLWidget()";

    makeCurrent();

    delete m_2dspace;

    doneCurrent();
}

void GLWidget::updateMVPAttrib()
{
    // calculate model view transformation
    // world/model matrix: determines the position and orientation of an object in 3D space
    m_mMatrix.setToIdentity();

    // Scale
    m_mMatrix.translate(m_cameraPosition);
    m_mMatrix.scale(m_distance);
    m_mMatrix.translate(-1.0 * m_cameraPosition);

    // Translation
    m_mMatrix.translate(m_translation);

    // Rotation
    m_mMatrix.translate(m_cameraPosition);
    m_mMatrix.rotate(m_rotation);
    m_mMatrix.translate(-1.0 * m_cameraPosition);
}

void GLWidget::initializeGL()
{
    qDebug() << "initializeGL";
    initializeOpenGLFunctions();
    updateMVPAttrib();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /******************** 1 Abstraction Space ********************/
    m_2dspace->initOpenGLFunctions();
    m_2dspace->initBuffer();

    /******************** 2 initialize Mesh **********************/
   // struct MeshUniforms mesh_uniforms = {m_yaxis, m_xaxis, m_mMatrix.data(), m_vMatrix.data(), m_projection.data()};
  //  m_mesh.initOpenGLFunctions(mesh_uniforms);

    /****************** 3 Initialize Graph  *******************/
    m_graphManager = new GraphManager();
    struct GraphUniforms graph_uniforms = {m_yaxis, m_xaxis, m_mMatrix.data(), m_vMatrix.data(), m_projection.data()};
    m_graphManager->initVBO(graph_uniforms, 0);

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
   // struct MeshUniforms mesh_uniforms = {m_yaxis, m_xaxis, m_mMatrix.data(), m_vMatrix.data(), m_projection.data()};
    struct GraphUniforms graph_uniforms = {m_yaxis, m_xaxis, m_mMatrix.data(), m_vMatrix.data(), m_projection.data()};

   // m_mesh.draw(mesh_uniforms);
    m_graphManager->drawNodes(graph_uniforms, 0);
    m_graphManager->drawEdges(graph_uniforms, 0);


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
            m_isRotatable = !m_isRotatable;
            break;
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
