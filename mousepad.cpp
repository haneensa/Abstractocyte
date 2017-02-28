// todo:    1) draw the grid with colors first
//          2) enable selection buffer
//          3) use ID for each color to retrieve the properties of the area we are inside

#include "mousepad.h"
#include "colors.h"

MousePad::MousePad(QWidget *parent)
    :  QOpenGLWidget(parent),
       m_vbo_circle( QOpenGLBuffer::VertexBuffer ),
       m_vbo_2DSpaceVerts( QOpenGLBuffer::VertexBuffer ),
       m_vbo_2DSpaceIndix( QOpenGLBuffer::IndexBuffer ),
       m_updatedPointer(true)
{
    m_bindColorIdx = 1;
    qDebug() << "MousePad";
    circle.x = 0.0;
    circle.y = 0.0;
    m_2dspace = NULL;
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

void MousePad::initSelectionPointerGL()
{
    m_program_circle = new QOpenGLShaderProgram(this);
    bool res = initShader(m_program_circle, ":/shaders/space_pointer_vert.glsl", ":/shaders/space_pointer_geom.glsl", ":/shaders/space_pointer_frag.glsl");
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

    /* selection buffer */

    m_program_selection = new QOpenGLShaderProgram(this);
    res = initShader(m_program_selection, ":/shaders/selection_vert.glsl", ":/shaders/selection_geom.glsl", ":/shaders/selection_frag.glsl");
    if(res == false)
        return;

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
}



void MousePad::initData()
{
    // init intervals
    // TODO: refactor this and make it easier to construct the space with fewer parameters
    // TODO: connect abstraction space buffer update data with this one
    // find a place to decide on the data and group them together instead on seperate classes

    if (m_2dspace == NULL) {
        qDebug() << "m_2dspace is NULL";
        return;
    }

    m_vertices = m_2dspace->get2DSpaceVertices();
    m_indices = m_2dspace->get2DSpaceIndices();
}

void MousePad::initBuffer()
{

    m_buffer_color_data.push_back(red);
    m_buffer_color_data.push_back(orange);
    m_buffer_color_data.push_back(blueviolet);
    m_buffer_color_data.push_back(steelblue);
    m_buffer_color_data.push_back(seagreen);
    m_buffer_color_data.push_back(brown);
    m_buffer_color_data.push_back(violet);
    m_buffer_color_data.push_back(peru);
    m_buffer_color_data.push_back(mediumspringgreen);
    m_buffer_color_data.push_back(gainsboro);
    m_buffer_color_data.push_back(honeydew);
    m_buffer_color_data.push_back(darkkhaki);
    m_buffer_color_data.push_back(gray);
    m_buffer_color_data.push_back(lightsalmon);
    m_buffer_color_data.push_back(orchid);
    m_buffer_color_data.push_back(peachpuff);
    m_buffer_color_data.push_back(royalblue);
    m_buffer_color_data.push_back(darkorchid);
    m_buffer_color_data.push_back(lemonchiffon);
    m_buffer_color_data.push_back(orange);
    m_buffer_color_data.push_back(blueviolet);
    m_buffer_color_data.push_back(steelblue);
    m_buffer_color_data.push_back(seagreen);
    m_buffer_color_data.push_back(brown);
    m_buffer_color_data.push_back(violet);
    m_buffer_color_data.push_back(peru);
    m_buffer_color_data.push_back(mediumspringgreen);
    m_buffer_color_data.push_back(gainsboro);

    int bufferSize =  m_buffer_color_data.size() * sizeof(QVector4D);

    glGenBuffers(1, &m_buffer_color);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer_color);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize , NULL, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bindColorIdx, m_buffer_color);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer_color);
    GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
    memcpy(p,   m_buffer_color_data.data(),  bufferSize);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void MousePad::init2DSpaceGL()
{
    m_program_2DSpace = glCreateProgram();
    bool res = initShader(m_program_2DSpace, ":/shaders/space2d_vert.glsl", ":/shaders/space2d_geom.glsl",
                          ":/shaders/points_passthrough_frag.glsl");
    if(res == false)
        return;

    glUseProgram(m_program_2DSpace);
    qDebug() << "init buffers";
    // create vbos and vaos
    m_vao_2DSpace.create();
    m_vao_2DSpace.bind();

    m_vbo_2DSpaceVerts.create();
    m_vbo_2DSpaceVerts.setUsagePattern( QOpenGLBuffer::DynamicDraw);
    m_vbo_2DSpaceVerts.bind();

    m_vbo_2DSpaceVerts.allocate( m_vertices.data(), m_vertices.size() * sizeof(QVector3D) );

    m_vbo_2DSpaceIndix.create();
    m_vbo_2DSpaceIndix.bind();
    m_vbo_2DSpaceIndix.allocate( m_indices.data(), m_indices.size() * sizeof(GLuint) );


    int offset = 0;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(struct abstractionPoint),  0);

    offset += sizeof(QVector2D);
    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(1, 1, GL_INT, sizeof(struct abstractionPoint), (GLvoid*)offset);

    int isSelectionLoc = glGetUniformLocation(m_program_2DSpace, "is_selection_shader");
    int isSelection = 0;
    glUniform1i(isSelectionLoc, isSelection);


    m_vbo_2DSpaceIndix.release();
    m_vbo_2DSpaceVerts.release();
    m_vao_2DSpace.release();
    GL_Error();

    /* selection buffer */
    m_program_2DSpace_Selection  = glCreateProgram();
    res = initShader(m_program_2DSpace_Selection, ":/shaders/space2d_vert.glsl", ":/shaders/space2d_geom.glsl",
                     ":/shaders/points_passthrough_frag.glsl");
    if(res == false)
        return;

    glUseProgram(m_program_2DSpace_Selection);

    // create vbos and vaos
    m_vao_2DSpace_Selection.create();
    m_vao_2DSpace_Selection.bind();
    m_vbo_2DSpaceVerts.bind();

    offset = 0;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(struct abstractionPoint),  0);

    offset += sizeof(QVector2D);
    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(1, 1, GL_INT, sizeof(struct abstractionPoint), (GLvoid*)offset);


    isSelectionLoc = glGetUniformLocation(m_program_2DSpace_Selection, "is_selection_shader");
    isSelection = 1;
    glUniform1i(isSelectionLoc, isSelection);

    m_vbo_2DSpaceVerts.release();
    m_vao_2DSpace_Selection.release();
}

void MousePad::initializeGL()
{
    qDebug() << "MousePad::initializeGL()";
    initializeOpenGLFunctions();
    glClearColor(0.0f,0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    qDebug() << "MousePad::initData()";
    initData();
    GL_Error();

    qDebug() << "MousePad::initBuffer()";
    initBuffer();
    GL_Error();

    qDebug() << "MousePad::initSelectionPointerGL()";
    initSelectionPointerGL();
    GL_Error();

    qDebug() << "MousePad::initializeGL()";
    init2DSpaceGL();

    qDebug() << "Widget OpenGl: " << format().majorVersion() << "." << format().minorVersion();
    qDebug() << "Context valid: " << context()->isValid();
    qDebug() << "Really used OpenGl: " << context()->format().majorVersion() << "." << context()->format().minorVersion();
    qDebug() << "OpenGl information: VENDOR:       " << (const char*)glGetString(GL_VENDOR);
    qDebug() << "                    RENDERDER:    " << (const char*)glGetString(GL_RENDERER);
    qDebug() << "                    VERSION:      " << (const char*)glGetString(GL_VERSION);
    qDebug() << "                    GLSL VERSION: " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    m_projection.setToIdentity();

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f,0.0f, 0.0f, 1.0f);
}

void MousePad::paintGL()
{
    glViewport( 0, 0, m_w, m_h);
    glClearColor(0.0f,0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_vao_circle.bind();
    m_program_circle->bind();
    m_program_circle->setUniformValue("pMatrix", m_projection);
    glDrawArrays(GL_POINTS, 0, 1);
    m_program_circle->release();
    m_vao_circle.release();

    m_vao_2DSpace.bind();
    m_vbo_2DSpaceIndix.bind();
    glUseProgram(m_program_2DSpace);

    GLuint pMatrix = glGetUniformLocation(m_program_2DSpace, "pMatrix");
    glUniformMatrix4fv(pMatrix, 1, GL_FALSE, m_projection.data());

    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);

    m_vbo_2DSpaceIndix.release();
    m_vao_2DSpace.release();

    if (m_updatedPointer == false)
        return;

    qDebug() << "HERER!";
    m_updatedPointer = false;
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
    //qDebug() << "Func: mouseMoveEvent: " << event->pos().x() << " " << event->pos().y();
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
    //qDebug() << "Func: mousePressEvent: " << event->pos().x() << " " << event->pos().y();
    setFocus();
    event->accept();
}

void MousePad::mouseReleaseEvent(QMouseEvent *event)
{
    //qDebug() << "Func: mouseReleaseEvent " <<  event->x() << " " << event->y();
    setFocus();

    event->accept();
}

void MousePad::getSlotsX(int value)
{
    if (m_x == value)
        return;

    m_x = value;
    // minimum = 0, maximum = 99
    emit setSignalX(value);
//    m_vbo_circle.bind();
//    circle.x = (float)value/100.0;
//    GLfloat points[] = { circle.x,  circle.y };
//    m_vbo_circle.allocate(points, 1 /*elements*/ * 2 /*corrdinates*/ * sizeof(GLfloat));
//    m_vbo_circle.release();
  //  m_updatedPointer = true;
    update();

}

void MousePad::getSlotsY(int value)
{
    if (m_y == value)
        return;

    m_y = value;
    // minimum = 0, maximum = 99
    emit setSignalY(value);
  //  m_vbo_circle.bind();
  //  circle.y = (float)(value)/100.0;
//    GLfloat points[] = { circle.x,  circle.y };
//    m_vbo_circle.allocate(points, 1 /*elements*/ * 2 /*corrdinates*/ * sizeof(GLfloat));
//    m_vbo_circle.release();
  //  m_updatedPointer = true;
    update();
}

void MousePad::getAbstractionData(AbstractionSpace *space_instance)
{
    qDebug() << "************ SLOT ";
    m_2dspace = space_instance;
}

void MousePad::renderSelection(void)
{
    aboutToCompose();
    qDebug() << "Draw Selection!";
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DITHER);
    m_vao_2DSpace.bind();
    m_vbo_2DSpaceIndix.bind();
    glUseProgram(m_program_2DSpace_Selection);

    GLuint pMatrix = glGetUniformLocation(m_program_2DSpace_Selection, "pMatrix");
    glUniformMatrix4fv(pMatrix, 1, GL_FALSE, m_projection.data());

    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);

    m_vbo_2DSpaceIndix.release();
    m_vao_2DSpace.release();


    m_vao_selection.bind();
    m_program_selection->bind();
    m_program_selection->setUniformValue("pMatrix",  m_projection);

    // set the uniform with the appropriate color code
    glDrawArrays(GL_POINTS, 0, 1);

    m_program_selection->release();
    m_vao_selection.release();

    glEnable(GL_DITHER);
    glClearColor(0.0f,0.0f, 0.0f, 0.0f);
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
    //qDebug() << "Pixel at: " << x << " " << y;
    glReadBuffer(GL_BACK);
    glReadPixels(x,  y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &res);
    int pickedID = res[0] + res[1] * 256 + res[2] * 256 * 256;

    qDebug() <<  res[0] <<  " " << res[1] <<  " " << res[2] << " " << res[3];

    if (pickedID == 255 || pickedID == 0) {
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
        emit setIntervalID(pickedID - 1);
        //qDebug() << "Picked ID: " << pickedID << "-> " << circle.x << " " << circle.y;
    }

    // update the circle vbo
    update();
    doneCurrent();
}
