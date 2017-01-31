#include "spatialhash.h"

SpatialHash::SpatialHash(int col, float min, float max)
    :   m_min(min),
        m_max(max),
        m_glFunctionsSet(false),
        m_GridVBO( QOpenGLBuffer::VertexBuffer )
{
    m_cellSize = m_max / (float)col;
    m_col = (m_max - m_min) / m_cellSize;
    qDebug() << col << " " << m_col;

    qDebug() << "m_col: " << m_col;

    /* I only need the cell size */
    qDebug() << "m_cellSize: " << m_cellSize;
}


SpatialHash::~SpatialHash()
{
    qDebug() << "Func: ~SpatHash";
}


std::pair<int,int>  SpatialHash::hash(float x, float y)
{
    float factor = 1.0/(float)m_cellSize;
    int i =  (int) std::floor(x * factor);
    int j = (int) std::floor(y * factor);

    return std::make_pair(i,j);
}

/*
 *     ****************o (xMax, yMax)
 *     *       *       *
 *     ******* * *******
 *     *       *       *
 *     o****************
 *   (xMin, yMin)
 **/
std::set< std::pair<int,int>  > SpatialHash::hashAABB(float x, float y, float r)
{
    std::set< std::pair<int,int>  > IdsList;
    float minX = x-r;
    float minY = y-r;
    float maxX = x+r;
    float maxY = y+r;

    std::pair<int,int>  bleft = hash(minX, minY);
    std::pair<int,int>  tright = hash(maxX, maxY);

    //qDebug() << bleft << " " << tright;
    for (int i = bleft.first; i <= tright.first; ++i) {
        for (int j = bleft.second; j <= tright.second; ++j) {
            IdsList.insert( std::make_pair(i,j) );
        }
    }

    return IdsList;
}

void SpatialHash::insert(Node *node)
{
    if (node == NULL)   return;
    qDebug() << "insert";
    float x = node->getLayoutedPosition().x();
    float y = node->getLayoutedPosition().y();
    std::pair<int,int>  cell = hash(x, y);
    int index;
    if (m_hashMap.find(cell) == m_hashMap.end()) { // empty cell
        std::vector<Node*> vec(1, node);
        m_hashMap.insert( std::make_pair(cell, vec) );
        // we are creating the cell for the first time
    } else {
        m_hashMap.at(cell).push_back( node );
    }

    index = m_hashMap.at(cell).size() - 1;
}

void SpatialHash::queryAABB(Node *node, float r, std::vector<Node*> &dataCell)
{
    if (node == NULL)   return;
    float x = node->getLayoutedPosition().x();
    float y = node->getLayoutedPosition().y();
    std::set< std::pair<int,int> > Ids = hashAABB(x, y, r);
    dataCell.clear();
    std::set< std::pair<int,int> >::iterator it;
    for (it = Ids.begin(); it != Ids.end(); it++) {
        if (m_hashMap.find(*it) == m_hashMap.end())
            continue;
        dataCell.insert( dataCell.end(), m_hashMap.at(*it).begin(), m_hashMap.at(*it).end() );
    }

    return;
}

void SpatialHash::updateNode(Node *node)
{
    // check if node moved to another cell
    float x = node->getLayoutedPosition().x();
    float y = node->getLayoutedPosition().y();
    std::pair<int,int>  cell = hash(x, y);
    std::pair<int, int> oldCell = node->getHashMapCell();  // todo
    if (cell == oldCell) {
        return;
    }

    if (m_hashMap.find(oldCell) != m_hashMap.end()) {
          m_hashMap.at(oldCell).erase(std::remove(m_hashMap.at(oldCell).begin(), m_hashMap.at(oldCell).end(), node), m_hashMap.at(oldCell).end());
          insert(node);
      } else {
          qDebug() << "This node " << node->getID() << " at ( " << oldCell.first << ", " << oldCell.second << " ) " << " doesnt exist in hashMap!";
      }
}

void SpatialHash::clear()
{
    m_hashMap.clear();
}


// opengl
bool SpatialHash::initOpenGLFunctions()
{
    m_glFunctionsSet = true;
    initializeOpenGLFunctions();

    return true;
}

void SpatialHash::fillGridDataPoints()
{
    // draw from min, to max, with cell size as incremnt values
    //   m_cellSize;
    //   m_min;
    //   m_max;
    //   m_col;
    std::pair<int,int>  bleft = hash(m_min, m_min);
    std::pair<int,int>  tright = hash(m_max, m_max);

    //qDebug() << bleft << " " << tright;
//    for (int i = bleft.first; i <= tright.first; ++i) {
//        for (int j = bleft.second; j <= tright.second; ++j) {
//            qDebug() << i << " " << j;
//            m_gridDataPoints.push_back(QVector2D(i, j));
//        }
//    }
   m_gridDataPoints.push_back(QVector2D(0, 0));
   m_gridDataPoints.push_back(QVector2D(0, 5));
   m_gridDataPoints.push_back(QVector2D(5, 0));/*
   m_gridDataPoints.push_back(QVector2D(0, -5));
   m_gridDataPoints.push_back(QVector2D(-5, 0));
   m_gridDataPoints.push_back(QVector2D(-5, -5));*/
   m_gridDataPoints.push_back(QVector2D(5, 5));

}

bool SpatialHash::init_Shaders_Buffers()
{
    if (m_glFunctionsSet == false)
        return false;

    qDebug() << "SpatialHash::init_Shaders_Buffers";

    // 1) initialize shaders
    m_program_grid = glCreateProgram();
    bool res = initShader(m_program_grid,  ":/shaders/grid.vert", ":/shaders/grid.geom", ":/shaders/grid.frag");
    GL_Error();

    if (res == false)
        return res;

    m_GridVAO.create();
    m_GridVAO.bind();

    m_GridVBO.create();
    m_GridVBO.setUsagePattern( QOpenGLBuffer::StaticDraw );
    m_GridVBO.bind();
    // allocate buffer values
    fillGridDataPoints();
    m_GridVBO.allocate( m_gridDataPoints.data(),
                        m_gridDataPoints.size() * sizeof(QVector2D) );

    glUseProgram(m_program_grid);

    // enable vertix attribs to access buffer from shaders
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          sizeof(QVector2D),  0);

    m_GridVBO.release();
    m_GridVAO.release();

    GL_Error();

    return true;
}

void SpatialHash::drawGrid(struct GridUniforms grid_uniforms)
{
    if (m_glFunctionsSet == false)
        return;

    m_GridVAO.bind();

    glUseProgram(m_program_grid);
    m_uniforms = grid_uniforms;

    GLuint mMatrix = glGetUniformLocation(m_program_grid, "mMatrix");
    glUniformMatrix4fv(mMatrix, 1, GL_FALSE, m_uniforms.mMatrix);

    GLuint vMatrix = glGetUniformLocation(m_program_grid, "vMatrix");
    glUniformMatrix4fv(vMatrix, 1, GL_FALSE, m_uniforms.vMatrix);

    GLuint pMatrix = glGetUniformLocation(m_program_grid, "pMatrix");
    glUniformMatrix4fv(pMatrix, 1, GL_FALSE, m_uniforms.pMatrix);

    glDrawArrays(GL_POINTS, 0, m_gridDataPoints.size() );

    m_GridVAO.release();
}
