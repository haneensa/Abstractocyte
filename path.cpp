#include "path.h"

Path::Path(int ID)
    :   m_vbo( QOpenGLBuffer::VertexBuffer )
{
    qDebug() << "Create new path";
    m_recording = false;
    m_ID = ID;
    m_name = QString("Path_%1").arg(m_ID);
    m_note = "None";
}

Path::Path(Path *p)
{
   updatePath(p);
}

Path::~Path()
{

}

void Path::updatePath(Path *p)
{
    m_recording = p->getRecordingFlag();
    m_ID = p->getID();
    m_name = p->getName();
    m_note = p->getNote();
    m_path = p->getPath();
    m_selectionPath = p->getSelectionPath();
}

void Path::addPoint(QVector2D point, QVector2D selection)
{
    if (m_recording == false)
        return;

    m_path.push_back(point);
    m_selectionPath.push_back(selection);

}

void Path::initPath()
{
    if (m_path.size() == 0) {
        qDebug() << "Path has 0 trace records";
    }

    initializeOpenGLFunctions();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_program  = glCreateProgram();
    bool res = initShader(m_program,
                          ":/shaders/path_vert.glsl",
                          ":/shaders/path_geom.glsl",
                          ":/shaders/path_frag.glsl");

    if(res == false)
        return;

    glUseProgram(m_program);


    m_vao.create();
    m_vao.bind();

    m_vbo.create();
    m_vbo.bind();

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0,  0);

    m_vbo.release();

    m_vao.release();

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void Path::drawPath(QMatrix4x4 projection)
{

    if (m_path.size() == 0) {
        qDebug() << "Path has 0 trace records";
        return;
    }

    qDebug() << "Draw Path";

    m_vao.bind();

    glUseProgram(m_program);

    m_vbo.bind();
    m_vbo.allocate( m_path.data(), m_path.size() * sizeof(QVector2D) );

    GLuint pMatrix = glGetUniformLocation(m_program, "pMatrix");
    glUniformMatrix4fv(pMatrix, 1, GL_FALSE,  projection.data());

    glDrawArrays(GL_POINTS, 0, m_path.size());

    m_vbo.release();
    m_vao.release();
}

void Path::tracePath(QMatrix4x4 projection, int x)
{
    if (m_recording == true)
        return;

    float percentage = (float)x/100.0;
    int range = m_path.size() * percentage;

    qDebug() << "retrace " << x << " " << range << " " << m_path.size();

    if (m_path.size() == 0) {
        qDebug() << "Path has 0 trace records";
        return;
    } else if (m_path.size() < range) {
        range = m_path.size();
    }

    qDebug() << "Draw Path " << m_name;


    std::vector<QVector2D>::const_iterator first = m_path.begin();
    std::vector<QVector2D>::const_iterator last = m_path.begin() + range;
    std::vector<QVector2D> subPath(first, last);

    if ( subPath.size() == 0) {
        qDebug() << "subpath == 0";
        return;
    }

    m_vao.bind();

    glUseProgram(m_program);

    m_vbo.bind();
    m_vbo.allocate( subPath.data(), subPath.size() * sizeof(QVector2D) );

    GLuint pMatrix = glGetUniformLocation(m_program, "pMatrix");
    glUniformMatrix4fv(pMatrix, 1, GL_FALSE,  projection.data());

    glDrawArrays(GL_POINTS, 0, subPath.size());

    m_vbo.release();
    m_vao.release();

}

QVector2D Path::getXY(int x)
{
    QVector2D point;

    if (m_recording == true)
        return point;

    float percentage = (float)x/100.0;
    int range = m_selectionPath.size() * percentage;

    qDebug() << "retrace " << x << " " << range << " " << m_selectionPath.size();

    if (m_selectionPath.size() == 0) {
        qDebug() << "Path has 0 trace records";
        return point;
    } else if (m_selectionPath.size() < range) {
        range = m_selectionPath.size();
    }

    return m_selectionPath[range];
}

void Path::dumpPath()
{
    qDebug() << "write path data to a file to load it again later";
    // name, note, path, filtered objects IDs
    // write it to as a text file?
}

void Path::loadPath()
{
    qDebug() << "read path from file to load data";
}
