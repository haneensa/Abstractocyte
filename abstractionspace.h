#ifndef ABSTRACTIONSPACE_H
#define ABSTRACTIONSPACE_H

#include <vector>
#include <string>
#include <QVector4D>
#include "mainopengl.h"
#include "intervaltree.h"


struct ssbo_2DState {
    QVector4D  states[2][6]; // val, alpha, color_intp, point_size, additional infos
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

    void updateXaxis(int xaxis);
    void updateYaxis(int yaxis);

    void defineQuadrant(QVector2D leftMin, int dim, struct ssbo_2DState data);

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

    // OpenGL
    GLuint                              m_buffer;
    GLuint                              m_bindIdx;
    bool                                m_glFunctionsSet;

    // Data
    struct ssbo_2DState                 m_2DState;
    std::unordered_map< std::pair<int, int>, struct ssbo_2DState , pair_hash > m_statesMap;


    IntervalTree                        m_intervalX;
    IntervalTree                        m_intervalY;

    int                                 m_prevIntvMaxX;
    int                                 m_prevIntvMaxY;
};

#endif // ABSTRACTIONSPACE_H
