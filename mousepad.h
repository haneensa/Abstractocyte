#ifndef MOUSEPAD_H
#define MOUSEPAD_H

#include <QOpenGLWidget>
#include <QWidget>

#include "abstractionspace.h"
#include "mainopengl.h"

struct point {
    float x;
    float y;
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
    void getSlotsX(int value);
    void getSlotsY(int value);
    void getAbstractionData(AbstractionSpace *space_instance);

signals:
    void setSignalX(int value);
    void setSignalY(int value);
    void setSliderX(int value);
    void setSliderY(int value);
    void setIntervalID(int ID);

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

    void init2D_DebugSpaceGL();
    void init2D_GridSpaceGL();
    void init2D_SelectionSpaceGL();

    void render2D_DebugSpace();
    void render2D_GridSpace();

    void initBuffer();
    void initData();
    bool updateBuffer();

private:
    QMatrix4x4                              m_projection;
    int                                     m_w;
    int                                     m_h;

    int                                     m_x;
    int                                     m_y;
    bool                                    m_updatedPointer;

    /* selection pointer */
    struct point                            circle;
    QOpenGLVertexArrayObject                m_vao_circle;
    QOpenGLBuffer                           m_vbo_circle;
    QOpenGLShaderProgram                    *m_program_circle;

    QOpenGLVertexArrayObject                m_vao_selection;
    QOpenGLShaderProgram                    *m_program_selection;


     int                                    m_intervalID;

    /* abstraction space grid */
    std::vector<QVector4D>                  m_buffer_color_data; // Color, Cenert, Type
    GLuint                                  m_buffer_color;
    GLuint                                  m_bindColorIdx;

    std::vector<struct abstractionPoint>    m_vertices;
    std::vector<GLuint>                     m_indices;

    std::vector<struct abstractionPoint>    m_grid_vertices;
    std::vector<GLuint>                     m_grid_indices;

    QOpenGLVertexArrayObject                m_vao_2DSpace_debug;
    QOpenGLVertexArrayObject                m_vao_2DSpace_grid;
    QOpenGLVertexArrayObject                m_vao_2DSpace_Selection;

    QOpenGLBuffer                           m_vbo_2DSpaceVerts;
    QOpenGLBuffer                           m_vbo_2DSpaceTrianglesIndix;

    QOpenGLBuffer                           m_vbo_2DSpaceGridVerts;
    QOpenGLBuffer                           m_vbo_2DSpaceGridIndix;

    GLuint                                  m_program_2DSpace_degbug;
    GLuint                                  m_program_2DSpace_grid;
    GLuint                                  m_program_2DSpace_Selection;

    AbstractionSpace                        *m_2dspace;

};

#endif // MOUSEPAD_H
