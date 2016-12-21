// todo:    1- render 3d segmentation
//          2- render the skeleton

#include "glwidget.h"
#include <iostream>
#include <QDebug>

GLWidget::GLWidget(QWidget *parent)
    :  QOpenGLWidget(parent)
{

}

GLWidget::~GLWidget()
{

}

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

}

void GLWidget::paintGL()
{

}

void GLWidget::resizeGL(int w, int h)
{

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
