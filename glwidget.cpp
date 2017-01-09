// todo:    1- render 3d segmentation (done)
//          2- render the skeleton
//          3- mesh normals (to be fixed)


/*
 * FinalMatrix = Projection * View * Model
 * Model = RotationAroundOrigin * TranslationFromOrigin * RotationAroundObjectCenter
 */
#include "glwidget.h"
#include <QResource>

GLWidget::GLWidget(QWidget *parent)
    :   QOpenGLWidget(parent),
        m_vbo_mesh( QOpenGLBuffer::VertexBuffer ),
        m_vbo_skeleton( QOpenGLBuffer::VertexBuffer ),
        m_vbo_skeleton2( QOpenGLBuffer::VertexBuffer ),
        m_isRotatable(true),
        m_yaxis(0),
        m_xaxis(0),
        m_state(0)
{
    QString path = "://data/mouse03_clean_faces.obj";
    m_vertices_size = loadOBJ(path, m_objects);
    qDebug() << "mesh size: " << m_objects.size();

    path = "://data/m3_astrocytes_points.csv";
    m_skeleton_vertices_size = loadSkeletonPoints(path, m_skeleton_obj, 0);

    qDebug() << "skeleton size: " << m_skeleton_obj.size();

    path = "://data/m3_points_ids.csv";
    m_skeleton_vertices_size2 = loadSkeletonPoints(path, m_skeleton_obj2, 1);

    qDebug() << "skeleton size2: " << m_skeleton_obj2.size();


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
    for (std::size_t i = 0; i != m_objects.size(); i++) {
        delete m_objects[i];
    }

    for (std::size_t i = 0; i != m_skeleton_obj.size(); i++) {
        delete m_skeleton_obj[i];
    }

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
    m_program = glCreateProgram();
    bool res = initShader(m_program, ":/shaders/mesh.vert", ":/shaders/mesh.geom", ":/shaders/mesh.frag");
    if (res == false)
        return;

    m_program_mesh = new QOpenGLShaderProgram(this);
    res = initShader(m_program_mesh, ":/shaders/mesh.vert", ":/shaders/mesh.geom", ":/shaders/mesh.frag");
    if(res == false)
        return;

    qDebug() << "Initializing MESH";
    // create vbos and vaos
    m_vao_mesh.create();
    m_vao_mesh.bind();

    m_program_mesh->bind();
    setMVPAttrib(m_program_mesh);

    /* Start UBO Example Initialization */

    //---- this has to be per shader who want to use this ubo
    GLuint matrices_index = glGetUniformBlockIndex(m_program_mesh->programId(), "Matrices");
    glUniformBlockBinding(m_program_mesh->programId(), matrices_index, 0 /* binding point */);
    //-----


    glGenBuffers(1, &uboMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(QMatrix4x4), NULL, GL_STATIC_DRAW); // allocate 150 bytes of memory
    glBindBuffer(GL_UNIFORM_BUFFER, 0); // release buffer

    glBindBufferRange(GL_UNIFORM_BUFFER, 2, uboMatrices, 0 /* binding point */, 2 * sizeof(QMatrix4x4));

    //-- fill ubo
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(QMatrix4x4), &m_projection);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(QMatrix4x4), sizeof(QMatrix4x4), &m_vMatrix);
    glBindBuffer(GL_UNIFORM_BUFFER, 0); // release

    /* End UBO Example Initialization */

    m_program_mesh->release();

    m_vbo_mesh.create();
    m_vbo_mesh.setUsagePattern( QOpenGLBuffer::StaticDraw);
    if ( !m_vbo_mesh.bind() ) {
        qDebug() << "Could not bind vertex buffer to the context.";
    }

    m_vbo_mesh.allocate(NULL, m_vertices_size * /* m_objects[0]->getSize() */ sizeof(QVector3D));

    int offset = 0;
    for (std::size_t i = 0; i != 1; i++) {
        qDebug() << "allocating: " << m_objects[i]->getName().data();
        m_vbo_mesh.write(offset, &m_objects[i]->getVertices()[0], m_objects[i]->getSize() * sizeof(QVector3D));
        // should not be uniform!
        // have an ID for each cell type, and determine the color in the fragment based on ID
        // int per vertex
   }

    m_program_mesh->bind();
    m_program_mesh->enableAttributeArray("posAttr");
    m_program_mesh->setAttributeBuffer("posAttr", GL_FLOAT, 0, 3);

    m_program_mesh->setUniformValue("y_axis", m_yaxis);
    m_program_mesh->setUniformValue("x_axis", m_xaxis);
    m_program_mesh->setUniformValue("state", m_state);

    m_program_mesh->release();
    m_vbo_mesh.release();
    m_vao_mesh.release();

    /********** START SKELETON **************/
    qDebug() << "point";

    m_program_skeleton = new QOpenGLShaderProgram(this);
    res = initShader(m_program_skeleton, ":/shaders/skeleton_point.vert", ":/shaders/skeleton_point.geom", ":/shaders/skeleton_point.frag");

    if(res == false)
        return;

    qDebug() << "Initializing SKELETON 1";

    m_vao_skeleton.create();
    m_vao_skeleton.bind();

    m_program_skeleton->bind();
    setMVPAttrib(m_program_skeleton);
    m_program_skeleton->release();

    m_vbo_skeleton.create();
    m_vbo_skeleton.setUsagePattern( QOpenGLBuffer::StaticDraw);
    if ( !m_vbo_skeleton.bind() ) {
        qDebug() << "Could not bind vertex buffer to the context.";
    }

    m_vbo_skeleton.allocate(NULL, /*m_vertices_size*/  m_skeleton_obj[0]->getSize() * sizeof(QVector3D));

    for (std::size_t i = 0; i != 1; i++) {
        qDebug() << "allocating: " << m_skeleton_obj[i]->getName().data();
        m_vbo_skeleton.write(offset, &m_skeleton_obj[i]->getVertices()[0], m_skeleton_obj[i]->getSize() * sizeof(QVector3D));
        // should not be uniform!
        // have an ID for each cell type, and determine the color in the fragment based on ID
        // int per vertex
   }

    m_program_skeleton->bind();
    m_program_skeleton->enableAttributeArray("posAttr");
    m_program_skeleton->setAttributeBuffer("posAttr", GL_FLOAT, 0, 3);

    m_program_skeleton->setUniformValue("y_axis", m_yaxis);
    m_program_skeleton->setUniformValue("x_axis", m_xaxis);
    m_program_skeleton->setUniformValue("state", m_state);

    m_program_skeleton->release();
    m_vbo_skeleton.release();
    m_vao_skeleton.release();

    /********** END SKELETON **************/

    /********** START SKELETON2 **************/
    qDebug() << "point";

    m_program_skeleton2 = new QOpenGLShaderProgram(this);
    res = initShader(m_program_skeleton2, ":/shaders/skeleton_point.vert", ":/shaders/skeleton_point.geom", ":/shaders/skeleton_point.frag");

    if(res == false)
        return;

    qDebug() << "Initializing SKELETON";

    m_vao_skeleton2.create();
    m_vao_skeleton2.bind();

    m_program_skeleton2->bind();
    setMVPAttrib(m_program_skeleton2);
    m_program_skeleton2->release();

    m_vbo_skeleton2.create();
    m_vbo_skeleton2.setUsagePattern( QOpenGLBuffer::StaticDraw);
    if ( !m_vbo_skeleton2.bind() ) {
        qDebug() << "Could not bind vertex buffer to the context.";
    }

    m_vbo_skeleton2.allocate(NULL, /*m_vertices_size*/  m_skeleton_obj2[0]->getSize() * sizeof(QVector3D));

    for (std::size_t i = 0; i != 1; i++) {
        qDebug() << "allocating: " << m_skeleton_obj2[i]->getName().data();
        m_vbo_skeleton2.write(offset, &m_skeleton_obj2[i]->getVertices()[0], m_skeleton_obj2[i]->getSize() * sizeof(QVector3D));
        // should not be uniform!
        // have an ID for each cell type, and determine the color in the fragment based on ID
        // int per vertex
   }

    m_program_skeleton2->bind();
    m_program_skeleton2->enableAttributeArray("posAttr");
    m_program_skeleton2->setAttributeBuffer("posAttr", GL_FLOAT, 0, 3);

    m_program_skeleton2->setUniformValue("y_axis", m_yaxis);
    m_program_skeleton2->setUniformValue("x_axis", m_xaxis);
    m_program_skeleton2->setUniformValue("state", m_state);
    m_program_skeleton2->setUniformValue("color", QVector3D(1.0, 0.0, 0.0));

    m_program_skeleton2->release();
    m_vbo_skeleton2.release();
    m_vao_skeleton2.release();

    /********** END SKELETON **************/
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    // to enable transparency
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void GLWidget::paintGL()
{
    // paint the text here
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
    /*** setMVPAttrib(m_program_mesh); ***/

    m_mMatrix.setToIdentity();

    // Scale
    m_mMatrix.translate(m_cameraPosition);
    m_mMatrix.scale(m_distance);
    m_mMatrix.translate(-1.0 * m_cameraPosition);

    // Translation
    m_mMatrix.translate(m_translation);

    // Rotation
//    if (m_flag_mesh_rotation) {
        m_mMatrix.translate(m_cameraPosition);
        m_mMatrix.rotate(m_rotation);
        m_mMatrix.translate(-1.0 * m_cameraPosition);
        m_Oldrotation = m_rotation;
//    } else {
//        m_mMatrix.translate(m_cameraPosition);
//        m_mMatrix.rotate(m_Oldrotation);
//        m_mMatrix.translate(-1.0 * m_cameraPosition);
//    }

        //-- fill ubo
//        glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
//        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(QMatrix4x4), &m_projection);
//        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(QMatrix4x4), sizeof(QMatrix4x4), &m_vMatrix);
//        glBindBuffer(GL_UNIFORM_BUFFER, 0); // release

    m_program_mesh->setUniformValue("mMatrix",  m_mMatrix );
    m_program_mesh->setUniformValue("vMatrix",  m_vMatrix );
    m_program_mesh->setUniformValue("pMatrix",  m_projection );

    /****/
    m_program_mesh->setUniformValue("y_axis", m_yaxis);
    m_program_mesh->setUniformValue("x_axis", m_xaxis);
    m_program_mesh->setUniformValue("state", m_state);

    glDrawArrays(GL_TRIANGLES, 0,  m_vertices_size );

    m_program_mesh->release();
    m_vao_mesh.release();

    /************************/

    m_vao_skeleton.bind();
    m_program_skeleton->bind();
    /*** setMVPAttrib(m_program_skeleton); ***/

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

    m_program_skeleton->setUniformValue("mMatrix",  m_mMatrix );
    m_program_skeleton->setUniformValue("vMatrix",  m_vMatrix );
    m_program_skeleton->setUniformValue("pMatrix",  m_projection );

    /****/
    m_program_skeleton->setUniformValue("y_axis", m_yaxis);
    m_program_skeleton->setUniformValue("x_axis", m_xaxis);
    m_program_skeleton->setUniformValue("state", m_state);
    m_program_skeleton2->setUniformValue("color", QVector3D(0.0, 1.0, 0.0));

    glDrawArrays(GL_POINTS, 0,  m_skeleton_vertices_size );

    m_program_skeleton->release();
    m_vao_skeleton.release();


//    /************************/
    m_vao_skeleton2.bind();
    m_program_skeleton2->bind();
    /*** setMVPAttrib(m_program_skeleton2); ***/

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

    m_program_skeleton2->setUniformValue("mMatrix",  m_mMatrix );
    m_program_skeleton2->setUniformValue("vMatrix",  m_vMatrix );
    m_program_skeleton2->setUniformValue("pMatrix",  m_projection );

    /****/
    m_program_skeleton2->setUniformValue("y_axis", m_yaxis);
    m_program_skeleton2->setUniformValue("x_axis", m_xaxis);
    m_program_skeleton2->setUniformValue("state", m_state);
    m_program_skeleton2->setUniformValue("color", QVector3D(1.0, 0.0, 0.0));

    glDrawArrays(GL_POINTS, 0,  m_skeleton_vertices_size2 );

    m_program_skeleton2->release();
    m_vao_skeleton2.release();
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
        case(Qt::Key_0):
            m_flag_mesh_rotation = !m_flag_mesh_rotation;
            break;
    }
}

void GLWidget::getSliderX(int value)
{
    m_xaxis = value;
    update();
}

void GLWidget::getSliderY(int value)
{
    m_yaxis = value;
    update();
}
