#include "graphmanager.h"

GraphManager::GraphManager(DataContainer *objectManager, OpenGLManager *opengl_mnger)
    : m_FDL_running(false),
      m_2D(false)
{
    m_data_containter = objectManager;
    m_opengl_mngr = opengl_mnger;
}

GraphManager::~GraphManager()
{
    qDebug() << "~GraphManager()";
    for (int i = 0; i < max_graphs; i++) {
        stopForceDirectedLayout(i);
        if (m_layout_threads[i].joinable()) {
            m_layout_threads[i].join();
        }

        delete m_graph[i]; // todo: if more than one iterate over all
    }
}

void GraphManager::update2Dflag(bool is2D, struct GlobalUniforms uniforms)
{
    m_2D = is2D;

    if (m_2D) { // if 3D but graph is never 3D

        m_opengl_mngr->multiplyWithRotation(uniforms.rMatrix);
        // reset graph
        for (int i = 0; i < max_graphs; i++) {
            m_FDL_running = true;
            m_graph[i]->updateUniforms(uniforms);
            m_layout_threads[i] = std::thread(&Graph::resetCoordinates, m_graph[i]);
        }
        // pass rotation matrix
    }
}
// I have 4 graphs:
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

    // iterate over mesh''s objects, and add all the center nodes except astrocyte
    // create the a node for each object and store it in neurites_nodes

    // std::pair<int, int> id_tuple, float x, float y, float z
    // int eID, int hvgxID, int nID1, int nID2
    std::vector<Node*> neurites_nodes;
    std::vector<QVector2D> edges_info = m_data_containter->getNeuritesEdges();

    std::vector<Node*> neurites_skeletons_nodes;
    std::vector<QVector4D> neurites_skeletons_edges;

    std::vector<Node*> astrocyte_skeleton_nodes;
    std::vector<QVector4D> astrocyte_skeleton_edges;

    std::map<int, Object*> objects_map = m_data_containter->getObjectsMap();
    // create skeleton for each obeject and add it to skeleton_segments

    // create connectivity information (neurite-neurite) and add it to neurites_conn_edges
    // add nodes
    for ( auto iter = objects_map.begin(); iter != objects_map.end(); iter++) {
        Object *objectP = (*iter).second;
        int hvgxID = objectP->getHVGXID();
        Object_t type = objectP->getObjectType();
        if (type != Object_t::ASTROCYTE && type != Object_t::SYNAPSE && type != Object_t::MITO) {
            // neurite
            QVector4D center = objectP->getCenter();
            Node* newNode = new Node(hvgxID, -1,  center.x(), center.y(), center.z());
            neurites_nodes.push_back(newNode);
        }

        // get skeleton of the object
        Skeleton *skeleton = objectP->getSkeleton();
        std::vector<QVector3D> nodes3D = skeleton->getGraphNodes();
        std::vector<QVector2D> edges2D = skeleton->getGraphEdges();
        // add nodes
        int skeleton_offset = objectP->getSkeletonOffset();
        for ( int i = 0; i < nodes3D.size(); i++) {
            unsigned long nIndx = i + skeleton_offset;
            Node* newNode = new Node(hvgxID, nIndx,  nodes3D[i].x(), nodes3D[i].y(), nodes3D[i].z());
            if (type == Object_t::ASTROCYTE) {
                astrocyte_skeleton_nodes.push_back(newNode);
            } else {
                neurites_skeletons_nodes.push_back(newNode);
            }

        }

        // add edges
        for (int i = 0; i < edges2D.size(); ++i) {
            int nID1 = edges2D[i].x() + skeleton_offset;
            int nID2 = edges2D[i].y() + skeleton_offset;
            QVector4D edge_info = QVector4D(i, hvgxID, nID1, nID2);

            if (type == Object_t::ASTROCYTE) {
                astrocyte_skeleton_edges.push_back(edge_info);
            } else {
                neurites_skeletons_edges.push_back(edge_info);
            }
        }
    }

     m_graph[0] = new Graph( Graph_t::NODE_NODE, m_opengl_mngr, 1 /* grid col */); // neurite-neurite
     m_graph[0]->parseNODE_NODE(neurites_nodes, edges_info);

     m_graph[1] = new Graph( Graph_t::NODE_SKELETON , m_opengl_mngr, 1 /* grid col */); // neurite-astrocyte skeleton
     m_graph[1]->parseSKELETON(astrocyte_skeleton_nodes, astrocyte_skeleton_edges);
     m_graph[1]->parseNODE_NODE(neurites_nodes, edges_info);

     m_graph[2] = new Graph( Graph_t::ALL_SKELETONS, m_opengl_mngr, 1 /* grid col */ ); //  neurites skeletons - astrocyte skeleton
     m_graph[2]->parseSKELETON(astrocyte_skeleton_nodes, astrocyte_skeleton_edges);
     m_graph[2]->parseSKELETON(neurites_skeletons_nodes, neurites_skeletons_edges);

     m_graph[3] = new Graph( Graph_t::NEURITE_SKELETONS, m_opengl_mngr, 1 /* grid col */  ); // neuries skeletons
     m_graph[3]->parseSKELETON(neurites_skeletons_nodes, neurites_skeletons_edges);

     // delete the nodes and edges
    for (std::size_t i = 0; i != neurites_nodes.size(); i++) {
        delete neurites_nodes[i];
    }

    for (std::size_t i = 0; i != neurites_skeletons_nodes.size(); i++) {
        delete neurites_skeletons_nodes[i];
    }

    for (std::size_t i = 0; i != astrocyte_skeleton_nodes.size(); i++) {
        delete astrocyte_skeleton_nodes[i];
    }

}

void GraphManager::stopForceDirectedLayout(int graphIdx)
{
    // problem the rotation is still on the nodes

    m_graph[graphIdx]->terminateFDL();
    m_FDL_running = false; // todo: get this from the graph itself, since it is per graph, even the thread?

    if (m_layout_threads[graphIdx].joinable()) {
        m_layout_threads[graphIdx].join();
    }

}

void GraphManager::startForceDirectedLayout(int graphIdx)
{
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
