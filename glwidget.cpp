// todo:    1- render 3d segmentation
//          2- render the skeleton

#include "glwidget.h"
#include <QResource>

GLWidget::GLWidget(QWidget *parent)
    :   QOpenGLWidget(parent),
        m_vbo_mesh( QOpenGLBuffer::VertexBuffer )
{
    qDebug() << vertices.size();
    QString path = "://data/mouse03.obj";
    loadOBJ(path, vertices);
    qDebug() << vertices.size();
}

GLWidget::~GLWidget()
{
    qDebug() << "~GLWidget()";

    makeCurrent();
    delete m_program_mesh;
    m_vao_mesh.destroy();
    m_vbo_mesh.destroy();
    doneCurrent();
}


void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QFont f;
    f.setPixelSize(50);
    initText(f);

    /* start initializing mesh */
    m_projection.setToIdentity();

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

    // GLfloat points[] = { 0.5f, 0.5f };
    // m_vbo_mesh.allocate(points, 1 /*elements*/ * 2 /*corrdinates*/ * sizeof(GLfloat));
    m_vbo_mesh.allocate(&vertices[0], vertices.size() * sizeof(QVector3D));

    m_program_mesh->bind();
    m_program_mesh->setUniformValue("pMatrix", m_projection);
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

    const QString text = "Juxtastrocyte";
    float scaleX = 1.0/width();
    float scaleY = 1.0/height();
    float x = 0.0;
    float y = 0.0;
    renderText( x, y, scaleX, scaleY, text);

    m_vao_mesh.bind();
    m_program_mesh->bind();
    m_program_mesh->setUniformValue("pMatrix", m_projection);
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
    m_projection.ortho( 0.0f,  1.0f, 0.0f, 1.0f, -10.0, 10.0 );
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
}

void GLWidget::getSliderX(int value)
{
    qDebug() << value;
}

void GLWidget::getSliderY(int value)
{
    qDebug() << value;
}
