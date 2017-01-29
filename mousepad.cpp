// todo:    1- circle border using shaders (done)
//          2- select circle (done)
//          2.2- and move around
//          3- text

#include "mousepad.h"

MousePad::MousePad(QWidget *parent)
    :  QOpenGLWidget(parent),
       m_vbo_circle( QOpenGLBuffer::VertexBuffer )
{
    qDebug() << "MousePad";
    circle.x = 0.0;
    circle.y = 0.0;
}

MousePad::~MousePad()
{
    qDebug() << "~MousePad";
    makeCurrent();
    delete m_program_circle;
    delete m_program_selection;
    m_vao_circle.destroy();
    m_vbo_circle.destroy();

    m_vao_selection.destroy();

    doneCurrent();
}

void MousePad::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    qDebug() << "Widget OpenGl: " << format().majorVersion() << "." << format().minorVersion();
    qDebug() << "Context valid: " << context()->isValid();
    qDebug() << "Really used OpenGl: " << context()->format().majorVersion() << "." << context()->format().minorVersion();
    qDebug() << "OpenGl information: VENDOR:       " << (const char*)glGetString(GL_VENDOR);
    qDebug() << "                    RENDERDER:    " << (const char*)glGetString(GL_RENDERER);
    qDebug() << "                    VERSION:      " << (const char*)glGetString(GL_VERSION);
    qDebug() << "                    GLSL VERSION: " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    m_projection.setToIdentity();

    m_program_circle = new QOpenGLShaderProgram(this);
    bool res = initShader(m_program_circle, ":/shaders/shader.vert", ":/shaders/shader.geom", ":/shaders/shader.frag");
    if(res == false)
        return;

    m_program_selection = new QOpenGLShaderProgram(this);
    res = initShader(m_program_selection, ":/shaders/selection.vert", ":/shaders/selection.geom", ":/shaders/selection.frag");
    if(res == false)
        return;

    // create vbos and vaos
    m_vao_circle.create();
    m_vao_circle.bind();

    m_vbo_circle.create();
    m_vbo_circle.setUsagePattern( QOpenGLBuffer::DynamicDraw);
    if ( !m_vbo_circle.bind() ) {
        qDebug() << "Could not bind vertex buffer to the context.";
        return;
    }

    GLfloat points[] = { circle.x, circle.y };

    m_vbo_circle.allocate(points, 1 /*elements*/ * 2 /*corrdinates*/ * sizeof(GLfloat));

    m_program_circle->bind();
    m_program_circle->setUniformValue("pMatrix", m_projection);
    m_program_circle->enableAttributeArray("posAttr");
    m_program_circle->setAttributeBuffer("posAttr", GL_FLOAT, 0, 2);
    m_program_circle->release();

    m_vbo_circle.release();
    m_vao_circle.release();

    // create vbos and vaos
    m_vao_selection.create();
    m_vao_selection.bind();
    if ( !m_vbo_circle.bind() ) {
        qDebug() << "Could not bind vertex buffer to the context.";
        return;
    }

    m_program_selection->bind();
    m_program_selection->enableAttributeArray("posAttr");
    m_program_selection->setAttributeBuffer("posAttr", GL_FLOAT, 0, 2);
    m_program_selection->setUniformValue("pMatrix",   m_projection);
    m_program_selection->release();

    m_vbo_circle.release();
    m_vao_selection.release();

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void MousePad::paintGL()
{
    glViewport( 0, 0, m_w, m_h);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_vao_circle.bind();
    m_program_circle->bind();
    m_program_circle->setUniformValue("pMatrix", m_projection);
    glDrawArrays(GL_POINTS, 0, 1);
    m_program_circle->release();
    m_vao_circle.release();
}

void MousePad::resizeGL(int w, int h)
{
    qDebug() << "Func: resizeGL: " << w << " " <<   width() <<  " " << h << " " << height();
    // Calculate aspect ratio
    const qreal retinaScale = devicePixelRatio();

    m_h = h * retinaScale;
    m_w = w * retinaScale;
    h = (h == 0) ? 1 : h;

    glViewport(0, 0, w * retinaScale, h * retinaScale);
    m_projection.setToIdentity();
    m_projection.ortho( 0.0f,  1.0f, 0.0f, 1.0f, -1.0, 1.0 );

    update();
}

QSize MousePad::minimumSizeHint() const
{
    return QSize(200, 200);
}
QSize MousePad::sizeHint() const
{
    return QSize(400, 400);
}

void MousePad::mouseMoveEvent(QMouseEvent *event)
{
    qDebug() << "Func: mouseMoveEvent: " << event->pos().x() << " " << event->pos().y();
    // here update the mouse position as we move!


    makeCurrent();
    const qreal retinaScale = devicePixelRatio();
    GLint viewport[4]; //  return of glGetIntegerv() -> x, y, width, height of viewport
    glGetIntegerv(GL_VIEWPORT, viewport);

    int x = event->x() * retinaScale;
    int y = viewport[3] - event->y() * retinaScale;

    doneCurrent();
    // calculate the offset from press to release, then update the point position
    // get the position were we pressed
    processSelection(x, y);
}


void MousePad::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "Func: mousePressEvent: " << event->pos().x() << " " << event->pos().y();
    setFocus();
    event->accept();
}

void MousePad::mouseReleaseEvent(QMouseEvent *event)
{
    qDebug() << "Func: mouseReleaseEvent " <<  event->x() << " " << event->y();
    setFocus();

    event->accept();
}

void MousePad::setSlotsX(int value)
{
    // minimum = 0, maximum = 99
    emit setSignalX(value);
    m_vbo_circle.bind();
    circle.x = (float)value/100.0;
    GLfloat points[] = { circle.x,  circle.y };
    m_vbo_circle.allocate(points, 1 /*elements*/ * 2 /*corrdinates*/ * sizeof(GLfloat));
    m_vbo_circle.release();
    update();

}


void MousePad::setSlotsY(int value)
{
    // minimum = 0, maximum = 99
    emit setSignalY(value);
    m_vbo_circle.bind();
    circle.y = (float)(value)/100.0;
    GLfloat points[] = { circle.x,  circle.y };
    m_vbo_circle.allocate(points, 1 /*elements*/ * 2 /*corrdinates*/ * sizeof(GLfloat));
    m_vbo_circle.release();
    update();
}

void MousePad::renderSelection(void)
{
    aboutToCompose();
    qDebug() << "Draw Selection!";
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_vao_selection.bind();
    m_program_selection->bind();
    m_program_selection->setUniformValue("pMatrix",  m_projection);

    // set the uniform with the appropriate color code
    glDrawArrays(GL_POINTS, 0, 1);

    m_program_selection->release();
    m_vao_selection.release();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void MousePad::processSelection(float x, float y)
{
    makeCurrent();
    renderSelection();
    // wait until all the pending drawing commands are really done.
    // slow operation, there are usually a long time between glDrawElements() and all the fragments completely radterized
    glFlush();
    glFinish();

    // 4 bytes per pixel (RGBA), 1x1 bitmap
    // width * height * components (RGBA)
    unsigned char res[4];
    qDebug() << "Pixel at: " << x << " " << y;
    glReadBuffer(GL_BACK);
    glReadPixels(x,  y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &res);
    int pickedID = res[0] + res[1] * 256 + res[2] * 256 * 256;

    qDebug() <<  res[0] <<  " " << res[1] <<  " " << res[2];

    if (pickedID == 0) {
        qDebug() << "Background, Picked ID: " << pickedID;
    } else { 
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        circle.x = x/viewport[2];
        circle.y = y/viewport[3];

        m_vbo_circle.bind();
        GLfloat points[] = { circle.x,  circle.y };
        m_vbo_circle.allocate(points, 1 /*elements*/ * 2 /*corrdinates*/ * sizeof(GLfloat));
        m_vbo_circle.release();

        emit setSliderX(circle.x * 100);
        emit setSliderY(circle.y * 100);

        qDebug() << "Picked ID: " << pickedID << "-> " << circle.x << " " << circle.y;
    }

    // update the circle vbo
    update();
    doneCurrent();
}
