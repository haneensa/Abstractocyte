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

    qDebug() << "Initializing VAO";
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

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

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
