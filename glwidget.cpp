// todo:    1- render 3d segmentation
//          2- render the skeleton
//          3- mesh normals


/*
 * FinalMatrix = Projection * View * Model
 * Model = RotationAroundOrigin * TranslationFromOrigin * RotationAroundObjectCenter
 */
#include "glwidget.h"
#include <QResource>

GLWidget::GLWidget(QWidget *parent)
    :   QOpenGLWidget(parent),
        m_vbo_mesh( QOpenGLBuffer::VertexBuffer ),
        m_isRotatable(true)
{
    qDebug() << vertices.size();
    QString path = "://data/mouse03.obj";
    loadOBJ(path, vertices);
    qDebug() << vertices.size();
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
    delete m_program_mesh;
    m_vao_mesh.destroy();
    m_vbo_mesh.destroy();
    vertices.clear();
    doneCurrent();
}

void GLWidget::setMVPAttrib(QOpenGLShaderProgram *program)
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

    program->setUniformValue("mMatrix",  m_mMatrix );
    program->setUniformValue("vMatrix",  m_vMatrix );
    program->setUniformValue("pMatrix",  m_projection );
}

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//    QFont f;
//    f.setPixelSize(50);
//    initText(f);

    /* start initializing mesh */
    m_program_mesh = new QOpenGLShaderProgram(this);
    bool res = initShader(m_program_mesh, ":/shaders/mesh.vert", ":/shaders/mesh.geom", ":/shaders/mesh.frag");
    if(res == false)
        return;

    qDebug() << "Initializing curser VAO";
    // create vbos and vaos
    m_vao_mesh.create();
    m_vao_mesh.bind();

    m_vbo_mesh.create();
    m_vbo_mesh.setUsagePattern( QOpenGLBuffer::StaticDraw);
    if ( !m_vbo_mesh.bind() ) {
        qDebug() << "Could not bind vertex buffer to the context.";
        return;
    }

    m_vbo_mesh.allocate(&vertices[0], vertices.size() * sizeof(QVector3D));

    m_program_mesh->bind();
    setMVPAttrib(m_program_mesh);
    m_program_mesh->enableAttributeArray("posAttr");
    m_program_mesh->setAttributeBuffer("posAttr", GL_FLOAT, 0, 3);
    m_program_mesh->release();

    m_vbo_mesh.release();
    m_vao_mesh.release();

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void GLWidget::paintGL()
{
    // paint the text here
    qDebug() << "draw";

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

//    const QString text = "Juxtastrocyte";
//    // todo: separate scale factor from position
//    float scaleX = 0.5/width();
//    float scaleY = 0.5/height();
//    float x = 0.5 / scaleX;
//    float y = 0.5 / scaleY;
//    renderText( x, y, scaleX, scaleY, text);

    m_vao_mesh.bind();
    m_program_mesh->bind();
    setMVPAttrib(m_program_mesh);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() );
    m_program_mesh->release();
    m_vao_mesh.release();
}

void GLWidget::resizeGL(int w, int h)
{
    qDebug() << "Func: resizeGL: " << w << " " <<   width() <<  " " << h << " " << height();
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
    qDebug() << value;
}

void GLWidget::getSliderY(int value)
{
    qDebug() << value;
}
