#ifndef RENDERVERTEXDATA_H
#define RENDERVERTEXDATA_H

#include "mainopengl.h"

class RenderVertexData : public MainOpenGL
{
public:
    RenderVertexData();
    ~RenderVertexData();

    void initOpenGLFunctions();
    GLuint createProgram(std::string program_name);
    bool compileShader(std::string program,  const char *vshader, const char *gshader, const char *fshader);
    GLuint getProgram(std::string program_name)     { return m_programs[program_name]; }
    void useProgram(std::string program_name);
    void printProgramsNames();


    void vboCreate(std::string name, int type);
    void vboBind(std::string name);
    void vboRelease(std::string name);
    void vboAllocate(std::string name, const void *data, int count);

protected:
    std::map<std::string, GLuint>           m_programs;
    std::map<std::string, QOpenGLBuffer>    m_vbo;

};

#endif // RENDERVERTEXDATA_H
