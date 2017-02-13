#ifndef ABSTRACTIONSPACE_H
#define ABSTRACTIONSPACE_H

#include <vector>
#include <string>
#include <QVector4D>
#include "mainopengl.h"
#include "ssbo_structs.h"

struct ast_neu_properties {
    struct properties ast;
    struct properties neu;
};

class AbstractionSpace : public MainOpenGL
{
public:
    AbstractionSpace(int xdim, int ydim);
    ~AbstractionSpace();

    void defineAbstractionState(int x, int y, std::string name, int dx = -1, int dy = -1);
    void initOpenGLFunctions();
    // ssbo buffer data
    int getBufferSize();
    void* getBufferData();
    bool initBuffer();
    bool updateBuffer();

    void defineQuadrant(QVector2D leftMin, int dim, struct ssbo_2DState data);
    void updateID(int ID);

private:
    struct pair_hash {
        template <class T1, class T2>
        std::size_t operator () (const std::pair<T1,T2> &p) const {
            auto h1 = std::hash<T1>{}(p.first);
            auto h2 = std::hash<T2>{}(p.second);
            return h1 ^ h2;
        }
    };

    int                                 m_xaxis;
    int                                 m_yaxis;

    int                                 m_intervalID;
    // OpenGL
    GLuint                              m_buffer;
    GLuint                              m_bindIdx;
    bool                                m_glFunctionsSet;

    // Data
    struct ssbo_2DState                 m_2DState;

    std::vector< struct ast_neu_properties >  m_IntervalXY;

};

#endif // ABSTRACTIONSPACE_H
