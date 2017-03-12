#ifndef RENDERVERTEXDATA_H
#define RENDERVERTEXDATA_H

#include "mainopengl.h"

class RenderVertexData : public MainOpenGL
{
public:
    RenderVertexData();


protected:
    std::map<std::string, QOpenGLBuffer>    m_QOpenGLBuffers;

};

#endif // RENDERVERTEXDATA_H
