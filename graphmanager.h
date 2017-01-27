#ifndef GRAPHMANAGER_H
#define GRAPHMANAGER_H

#include <QOpenGLFunctions_4_3_Core>
#include "graph.h"

class GraphManager : protected QOpenGLFunctions_4_3_Core
{
public:
    GraphManager();
    ~GraphManager();

    void drawNodes();
    void drawEdges();

protected:
    Graph *m_graph1;



};

#endif // GRAPHMANAGER_H
