// todo:    1- circle border using shaders (done)
//          2- select circle and move around (today)
//          3- text

#include "mousepad.h"
#include <iostream>
#include <QDebug>

MousePad::MousePad(QWidget *parent)
    :  QOpenGLWidget(parent),
       m_vbo_circlue( QOpenGLBuffer::VertexBuffer )
{

}

MousePad::~MousePad()
{
    makeCurrent();
    delete m_program_circle;
    delete m_program_selection;
    m_vao_circlue.destroy();
    m_vbo_circlue.destroy();
    doneCurrent();
}

void MousePad::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    qDebug() << "Widget OpenGl: " << format().majorVersion() << "." << format().minorVersion();
    qDebug() << "Context valid: " << context()->isValid();
    qDebug() << "Really used OpenGl: " << context()->format().majorVersion() << "." << context()->format().minorVersion();
    qDebug() << "OpenGl information: VENDOR:       " << (const char*)glGetString(GL_VENDOR);
    qDebug() << "                    RENDERDER:    " << (const char*)glGetString(GL_RENDERER);
    qDebug() << "                    VERSION:      " << (const char*)glGetString(GL_VERSION);
    qDebug() << "                    GLSL VERSION: " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    m_program_circle = new QOpenGLShaderProgram(this);
    bool res = initShader(m_program_circle, ":/shaders/shader.vert", ":/shaders/shader.geom", ":/shaders/shader.frag");
    if(res == false)
        return;

    m_program_selection = new QOpenGLShaderProgram(this);
    res = initShader(m_program_selection, ":/shaders/selection.vert", ":/shaders/selection.geom", ":/shaders/selection.frag");
    if(res == false)
        return;

    qDebug() << "Initializing curser VAO";
    // create vbos and vaos
    m_vao_circlue.create();
    m_vao_circlue.bind();

    m_vbo_circlue.create();
    m_vbo_circlue.setUsagePattern( QOpenGLBuffer::StaticDraw);
    if ( !m_vbo_circlue.bind() ) {
        qDebug() << "Could not bind vertex buffer to the context.";
        return;
    }

    // "posAttr", "colAttr", "matrix", "volumeAttr"
    GLfloat points[] = { 0.0f, 0.0f};

    m_vbo_circlue.allocate(points, 1 /*elements*/ * 2 /*corrdinates*/ * sizeof(GLfloat));

    m_program_circle->bind();
    m_program_circle->enableAttributeArray("posAttr");
    m_program_circle->setAttributeBuffer("posAttr", GL_FLOAT, 0, 2);
    m_program_circle->release();

    m_vbo_circlue.release();
    m_vao_circlue.release();


    qDebug() << "Initializing selection VAO";
    // create vbos and vaos
    m_vao_selection.create();
    m_vao_selection.bind();


    m_vbo_selection.create();
    m_vbo_selection.setUsagePattern( QOpenGLBuffer::StaticDraw);
    if ( !m_vbo_selection.bind() ) {
        qDebug() << "Could not bind vertex buffer to the context.";
        return;
    }

    m_vbo_selection.allocate(points, 1 /*elements*/ * 2 /*corrdinates*/ * sizeof(GLfloat));

    m_program_selection->bind();
    m_program_selection->enableAttributeArray("posAttr");
    m_program_selection->setAttributeBuffer("posAttr", GL_FLOAT, 0, 2);
    m_program_selection->release();

    m_vbo_selection.release();
    m_vao_selection.release();

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

bool MousePad::initShader(QOpenGLShaderProgram *program, const char *vshader, const char *gshader, const char *fshader)
{
    qDebug() << "Initializing shaders";

    // Compile vertex shader
    if (!program->addShaderFromSourceFile(QOpenGLShader::Vertex, vshader))
    {
        qDebug() << "Error in vertex shader:" << program->log();
        return false;
    }

    // Compile geometry shader
    if (!program->addShaderFromSourceFile(QOpenGLShader::Geometry, gshader))
    {
        qDebug() << "Error in Geometry shader:" << program->log();
        return false;
    }

    // Compile fragment shader
    if (!program->addShaderFromSourceFile(QOpenGLShader::Fragment, fshader))
    {
        qDebug() << "Error in fragment shader:" << program->log();
        return false;
    }

    // Link shader pipeline
    if (!program->link())
    {
        qDebug() << "Error in linking shader program:" << program->log();
        return false;
    }

    return true;
}

void MousePad::paintGL()
{
    qDebug() << "Draw!";
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_vao_circlue.bind();
    m_program_circle->bind();
    glDrawArrays(GL_POINTS, 0, 1);
    m_program_circle->release();
    m_vao_circlue.release();
}

void MousePad::resizeGL(int w, int h)
{
    m_projection.setToIdentity();
}

void MousePad::mouseMoveEvent(QMouseEvent *event)
{
    emit setSliderX(event->x());
    emit setSliderY(100 - event->y());
}


void MousePad::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "Func: mousePressEvent: " << event->pos().x() << " " << event->pos().y();
    setFocus();
    event->accept();
}

void MousePad::mouseReleaseEvent(QMouseEvent *event)
{
    qDebug() << "Func: mouseReleaseEvent";
    setFocus();

    float x = event->x();
    float y = event->y();
    // 4 bytes per pixel (RGBA), 1x1 bitmap
    // width * height * components (RGBA)
    unsigned char color[4];
    GLfloat depth;
    GLuint index;

    GLint viewport [4];
    glGetIntegerv (GL_VIEWPORT, viewport);
    glReadPixels(x, viewport[3] - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, color);
    glReadPixels(x, viewport[3] - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
    glReadPixels(x, viewport[3] - y, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &index);

    std::cout << "Clicked on pixel: " << x << " " << y << std::endl;
    std::cout << "depth: " << depth << " index: " << index << std::endl;
    std::cout << "viewport: " << viewport[3] << std::endl;
    std::cout << "r: " << (int)color[0] << std::endl;
    std::cout << "g: " << (int)color[1] << std::endl;
    std::cout << "b: " << (int)color[2] << std::endl;
    std::cout << "a: " << (int)color[3] << std::endl;
    std::cout << "coordinates in object space : ?" << std::endl;
    std::cout << std::endl;

    processSelection(x, y);

    event->accept();
}

void MousePad::setSlotsX(int value)
{
    emit setSignalX(value);
    update();
}


void MousePad::setSlotsY(int value)
{
    emit setSignalY(value);
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

    // set the uniform with the appropriate color code
    glDrawArrays(GL_POINTS, 0, 1);

    m_program_selection->release();
    m_vao_selection.release();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void MousePad::processSelection(float xx, float yy)
{
    makeCurrent();
    renderSelection();
    // wait until all the pending drawing commands are really done.
    // slow operation, there are usually a long time between glDrawElements() and all the fragments completely radterized
    glFlush();
    glFinish();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    GLint viewport[4]; //  return of glGetIntegerv() -> x, y, width, height of viewport
    glGetIntegerv(GL_VIEWPORT, viewport);
    qDebug() <<  viewport[0] <<  " " << viewport[1] <<  " " << viewport[2] << " " << viewport[3];

    unsigned char res[4];
    glReadPixels(xx, viewport[3] - yy, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &res);
    int pickedID = res[0] + res[1] * 256 + res[2] * 256 * 256;

    qDebug() <<  res[0] <<  " " << res[1] <<  " " << res[2];

    if (pickedID == 0x00ffffff) {
        qDebug() << "Background, Picked ID: " << pickedID;
    } else {
        qDebug() << "Picked ID: " << pickedID;
    }
    update();
    doneCurrent();
}
