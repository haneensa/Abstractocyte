#ifndef ABSTRACTIONSPACE_H
#define ABSTRACTIONSPACE_H

#include <vector>
#include <string>
#include <QVector4D>
#include "mainopengl.h"
#include "ssbo_structs.h"

struct abstractionPoint {
    QVector2D point;
    int       ID;
};


class AbstractionSpace : public MainOpenGL
{
public:
    AbstractionSpace(int xdim, int ydim);
    ~AbstractionSpace();

    void defineAbstractionState(int x, int y, std::string name, int dx = -1, int dy = -1);
    void initOpenGLFunctions();
    // ssbo buffer data
    bool initBuffer();
    bool updateBuffer();

    void defineQuadrant(QVector2D leftMin, int dim, struct ssbo_2DState data);
    void updateID(int ID);


    void initRect(QVector2D x_interval, QVector2D y_interval, int ID);
    void initLine(QVector2D end1, QVector2D end2, int ID);
    void initTriangle(QVector2D coords1, QVector2D coords2,QVector2D coords3, int ID);


    std::vector<struct abstractionPoint> get2DSpaceVertices() { return m_vertices; }
    std::vector<GLuint> get2DSpaceIndices() { return m_indices; }

    struct ast_neu_properties getSpaceProper()  { return m_IntervalXY[m_intervalID]; }

private:
    struct pair_hash {
        template <class T1, class T2>
        std::size_t operator () (const std::pair<T1,T2> &p) const {
            auto h1 = std::hash<T1>{}(p.first);
            auto h2 = std::hash<T2>{}(p.second);
            return h1 ^ h2;
        }
    };

    int                                         m_xaxis;
    int                                         m_yaxis;

    int                                         m_intervalID;
    // OpenGL
    GLuint                                      m_buffer;
    GLuint                                      m_bindIdx;
    bool                                        m_glFunctionsSet;

    // Datas
    struct ssbo_2DState                         *m_2DState;

    std::vector< struct ast_neu_properties >    m_IntervalXY;

    std::vector<struct abstractionPoint>        m_vertices;
    std::vector<GLuint>                         m_indices;
    std::map<std::pair<int, int>, struct properties>
                                                m_neu_states;
    std::map<std::pair<int, int>, struct properties>
                                                m_ast_states;
};

#endif // ABSTRACTIONSPACE_H
