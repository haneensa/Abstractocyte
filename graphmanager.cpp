// this should only handle the 4 graphs, their construction, layouting algorithms
// and updating their data, and drawing
#include "graphmanager.h"

GraphManager::GraphManager(ObjectManager *objectManager)
    : m_FDL_running(false),
      m_2D(false)
{
    m_obj_mngr = objectManager;
}

GraphManager::~GraphManager()
{
    qDebug() << "~GraphManager()";
    stopForceDirectedLayout(0);
    if (m_layout_thread1.joinable()) {
        m_layout_thread1.join();
    }

    delete m_graph[0]; // todo: if more than one iterate over all
}

void GraphManager::update2Dflag(bool is2D)
{
    m_2D = is2D;
    if (!m_2D) {
        // reset the cooridnates of the graphs
        QMatrix4x4 identitiy;
        m_graph[0]->resetCoordinates(identitiy); // for now later I will have two separate variables one for 2D one for 3D

    } else {
        // reset graph
        m_graph[0]->resetCoordinates(m_uniforms.rMatrix);
    }
}
// I have 4 graphs:
// so here extract the object nodes in a list because more than a graph use them
// extract skeletons graph in another list
// extract connectivity between neurites in a list
// extract connectivity between astrocyte and neurties in another list
// 1) all skeleton with astrocyte
    // (nodes are object skeleton nodes, and edges are the edges that connect them)
    // + connection points between astrocyte and neurites if that vertex is close to the astrocyte
// 2) skeletons without astrocyte
    // only nods of object and what connect the object nodes together
// 3) astrocyte skeleton and object nodes
    // object nodes
    // astrocyte graph
    // connectivity edges between them
// 4) object nodes and their connectivity information
    // object nodes
    // connectivity info from them
void GraphManager::ExtractGraphFromMesh()
{

     // render skeletons then interpolate to nodes by accessing the nodes positions from ssbo
     // use object manager to initialize them

     // init the edges and they never change except when to display them

     m_graph[0] = new Graph( Graph_t::NODE_SKELETON ); // neurite-neurite
//     m_graph[1] = new Graph( Graph_t::NODE_SKELETON ); // neurite-astrocyte skeleton
//     m_graph[2] = new Graph( Graph_t::ALL_SKELETONS ); //  neurites skeletons - astrocyte skeleton
//     m_graph[3] = new Graph( Graph_t::NEURITE_SKELETONS ); // neuries skeletons

     m_graph[0]->createGraph(m_obj_mngr);
//     m_graph[1]->createGraph(m_obj_mngr);
//     m_graph[2]->createGraph(m_obj_mngr);
//     m_graph[3]->createGraph(m_obj_mngr);
}

void GraphManager::stopForceDirectedLayout(int graphIdx)
{
    // problem the rotation is still on the nodes

    m_graph[graphIdx]->terminateFDL();
    m_FDL_running = false; // todo: get this from the graph itself, since it is per graph, even the thread?

    if (m_layout_thread1.joinable()) {
        m_layout_thread1.join();
    }

}

void GraphManager::startForceDirectedLayout(int graphIdx)
{
    // update the 2D node position at the start and whenever the m_mvp change, -> when m_value == 1.0 and m_mvp changed
    stopForceDirectedLayout(graphIdx);

    m_FDL_running = true;
    m_layout_thread1 = std::thread(&Graph::runforceDirectedLayout, m_graph[graphIdx]);

}

void GraphManager::updateGraphParam1(double value)
{
    m_graph[0]->updateGraphParam1(value);
}

void GraphManager::updateGraphParam2(double value)
{
    m_graph[0]->updateGraphParam2(value);
}

void GraphManager::updateGraphParam3(double value)
{
    m_graph[0]->updateGraphParam3(value);
}

void GraphManager::updateGraphParam4(double value)
{
    m_graph[0]->updateGraphParam4(value);
}

void GraphManager::updateGraphParam5(double value)
{
    m_graph[0]->updateGraphParam5(value);
}

void GraphManager::updateGraphParam6(double value)
{
    m_graph[0]->updateGraphParam6(value);
}

void GraphManager::updateGraphParam7(double value)
{
    m_graph[0]->updateGraphParam7(value);
}
