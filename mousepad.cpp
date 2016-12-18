#include "mousepad.h"
#include <iostream>
#include <QDebug>

MousePad::MousePad(QWidget *parent)
    :  QOpenGLWidget(parent)
{
}

MousePad::~MousePad()
{

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

    m_program = new QOpenGLShaderProgram(this);


}

void MousePad::paintGL()
{

}

void MousePad::resizeGL(int w, int h)
{

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
