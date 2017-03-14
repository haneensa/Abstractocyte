#ifndef RENDERVERTEXDATA_H
#define RENDERVERTEXDATA_H

#include "mainopengl.h"

enum class Buffer_t { VERTEX, INDEX, NONE };
enum class Buffer_USAGE_t { DYNAMIC_DRAW, STATIC, NONE };

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


    void vboCreate(std::string name, Buffer_t type, Buffer_USAGE_t usage);
    void vboBind(std::string name);
    void vboRelease(std::string name);
    void vboAllocate(std::string name, const void *data, int count);
    void vboWrite(std::string name, int offset ,const void *data, int count);
    QOpenGLBuffer getVBO(std::string name);

    void vaoCreate(std::string name);
    void vaoBind(std::string name);
    void vaoRelease();

protected:
    std::map<std::string, GLuint>           m_programs;
    std::map<std::string, QOpenGLBuffer>    m_vbo;
    std::map<std::string, GLuint>    m_vao;

};

#endif // RENDERVERTEXDATA_H
