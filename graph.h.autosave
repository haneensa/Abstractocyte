#ifndef GRAPH_H
#define GRAPH_H

#include "node.h"
#include "edge.h"

// store graph per object?

class Graph
{
public:
    Graph();
    ~Graph();

    bool loadNodes(QString filename);
    bool loadEdges(QString filename);


protected:
    std::map<unsigned int, Node*> m_nodes;
    std::map<unsigned int, Edge*> m_edges;
};

#endif // GRAPH_H
