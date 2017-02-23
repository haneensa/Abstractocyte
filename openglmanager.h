// manage all vbos and ssbos here
// and drawing calls

// 1) Mesh Triangles
// 2) Mesh Points
// 3) Skeleton Points
// 3) Abstract Skeleton Graph (Nodes and Edges)
// 4) Neurites Graph (Nodes and Edges)

#ifndef OPENGLMANAGER_H
#define OPENGLMANAGER_H

#include "mainopengl.h"
#include "objectmanager.h"

class OpenGLManager : public MainOpenGL
{
public:
    OpenGLManager(ObjectManager *obj_mnger);

    // *********** 3) Skeleton Points    ***********
    bool initSkeletonShaders();


protected:
    ObjectManager *m_obj_mnger; // get the data to render from here

    // *********** 3) Skeleton Points    ***********
    QOpenGLVertexArrayObject            m_vao_skeleton;
    QOpenGLBuffer                       m_vbo_skeleton;
    GLuint                              m_program_skeleton;


};

#endif // OPENGLMANAGER_H
