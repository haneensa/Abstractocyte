#include "rendervertexdata.h"

RenderVertexData::RenderVertexData()
{

}

RenderVertexData::~RenderVertexData()
{
    // delete all programs     glDeleteProgram(m_program_skeleton);

}

void RenderVertexData::initOpenGLFunctions()
{
    initializeOpenGLFunctions();

}

GLuint RenderVertexData::createProgram(std::string program_name)
{
    m_programs[program_name] =  glCreateProgram();
    return m_programs[program_name];
}

bool RenderVertexData::compileShader(std::string program_name,
                                  const char *vshader, const char *gshader, const char *fshader)
{
    bool res =  initShader( m_programs[program_name] , vshader, gshader, fshader);
    return res;
}

void RenderVertexData::useProgram(std::string program_name)
{
    glUseProgram( m_programs[program_name] );
}


void RenderVertexData::vboBind(std::string name)
{
    m_vbo[name].bind();
}

void RenderVertexData::vboRelease(std::string name)
{
    m_vbo[name].release();
}

void RenderVertexData::vboAllocate(std::string name, const void *data, int count)
{
    m_vbo[name].allocate( data, count );
}

void  RenderVertexData::vboWrite(std::string name, int offset ,const void *data, int count)
{
    m_vbo[name].write( offset, data, count );
}

void RenderVertexData::vboCreate(std::string name, Buffer_t type, Buffer_USAGE_t usage)
{
    if (type == Buffer_t::VERTEX)
        m_vbo[name] = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    else if (type == Buffer_t::INDEX)
        m_vbo[name] = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);

    m_vbo[name].create();

    if (usage == Buffer_USAGE_t::DYNAMIC_DRAW )
        m_vbo[name].setUsagePattern( QOpenGLBuffer::DynamicDraw );
    else if (usage == Buffer_USAGE_t::STATIC)
        m_vbo[name].setUsagePattern( QOpenGLBuffer::StaticDraw );

}

QOpenGLBuffer RenderVertexData::getVBO(std::string name)
{
    return m_vbo[name];
}

//************** VAO
void RenderVertexData::vaoCreate(std::string name)
{
    glGenVertexArrays(1, &m_vao[name]);
}

void RenderVertexData::vaoBind(std::string name)
{
    glBindVertexArray(m_vao[name]);
}

void RenderVertexData::vaoRelease()
{
    glBindVertexArray(0);
}
