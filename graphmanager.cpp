#include "graphmanager.h"

GraphManager::GraphManager()
{
    m_graph1 = new Graph();
    m_graph1->loadNodes("://data/skeleton_astrocyte_m3/skeleton_astro_nodes.csv");
    m_graph1->loadEdges("://data/skeleton_astrocyte_m3/skeleton_astro_segments.csv");

}
GraphManager::~GraphManager()
{

}

void GraphManager::drawNodes()
{

}

void GraphManager::drawEdges()
{

}
