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

    void addPoint(QVector2D point, QVector2D selection);
    void resetPath()                        { m_path.clear();  m_selectionPath.clear();}
    void updateRecordingFlag(bool flag)     { m_recording = flag; }
    QVector2D getXY(int x);

    void namePath(QString name)             { m_name = name; }
    void setID(int ID)                      { m_ID = ID; }
    void addNote(QString note)              { m_note = note; }

protected:
    QString                     m_name;
    int                         m_ID;
    QString                     m_note;

    std::vector<QVector2D>      m_path;
    std::vector<QVector2D>      m_selectionPath;

    QOpenGLVertexArrayObject    m_vao;
    QOpenGLBuffer               m_vbo;
    GLuint                      m_program;

    bool                        m_recording;
};

#endif // PATH_H
