#ifndef PATH_H
#define PATH_H

#include "mainopengl.h"

class Path  : public MainOpenGL
{
public:
    Path();
    ~Path();

    void initPath();
    void drawPath(QMatrix4x4 projection);

    void tracePath(QMatrix4x4 projection, int x);

    void addPoint(QVector2D point);
    void resetPath()                        { m_path.clear(); }
    void updateRecordingFlag(bool flag)     { m_recording = flag; }

protected:
    QString                     m_name;
    int                         m_ID;
    QString                     m_description;

    std::vector<QVector2D>      m_path;

    QOpenGLVertexArrayObject    m_vao;
    QOpenGLBuffer               m_vbo;
    GLuint                      m_program;

    bool                        m_recording;
};

#endif // PATH_H
