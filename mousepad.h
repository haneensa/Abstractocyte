#ifndef MOUSEPAD_H
#define MOUSEPAD_H

#include <QOpenGLWidget>
#include <QWidget>


#include "mainopengl.h"

struct point {
    float x;
    float y;
};

struct abstractionPoint {
    QVector2D point;
    int       ID;
};

class MousePad : public QOpenGLWidget, MainOpenGL
{
    Q_OBJECT

public:
    MousePad(QWidget *parent = 0);
    ~MousePad();
    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;

public slots:
    void setSlotsX(int value);
    void setSlotsY(int value);

signals:
    void setSignalX(int value);
    void setSignalY(int value);
    void setSliderX(int value);
    void setSliderY(int value);

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;


    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent*event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    void renderSelection(void);
    void processSelection(float dx, float dy);

    void initSelectionPointerGL();
    void init2DSpaceGL();

    void initRect(QVector2D p00, float dimX, float dimY);

private:
    QMatrix4x4                  m_projection;
    int                         m_w;
    int                         m_h;

    /* selection pointer */
    struct point                circle;
    QOpenGLVertexArrayObject    m_vao_circle;
    QOpenGLBuffer               m_vbo_circle;
    QOpenGLShaderProgram        *m_program_circle;

    QOpenGLVertexArrayObject    m_vao_selection;
    QOpenGLShaderProgram        *m_program_selection;



    /* abstraction space grid */
    std::vector<struct abstractionPoint>        m_vertices;
    std::vector<GLuint>                         m_indices;

    QOpenGLVertexArrayObject                    m_vao_2DSpace;
    QOpenGLVertexArrayObject                    m_vao_2DSpace_Selection;

    QOpenGLBuffer                               m_vbo_2DSpaceVerts;
    QOpenGLBuffer                               m_vbo_2DSpaceIndix;

    QOpenGLShaderProgram                        *m_program_2DSpace;
    QOpenGLShaderProgram                        *m_program_2DSpace_Selection;
};

#endif // MOUSEPAD_H
