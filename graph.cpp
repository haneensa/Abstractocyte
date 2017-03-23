#include "graph.h"
#include <cmath>

Graph::Graph(Graph_t graphType, OpenGLManager *opengl_mnger, int gridCol)
    : m_FDL_terminate(true),
      m_dupEdges(0)


{
    m_gType = graphType;
    m_opengl_mngr = opengl_mnger;

    FDL_initParameters();

    // spatial hashing
    float gridMin = 0.0f;
    float gridMax = 1.0f;
    hashGrid = new SpatialHash(gridCol, gridMin, gridMax);
}

void Graph::FDL_initParameters()
{
    // force directed layout parameters
    m_fdr_params.Cr = 1.5;
    m_fdr_params.Ca = 0.5;
    m_fdr_params.AABBdim = 0.15f; // used for spatial hashing query dim
    m_fdr_params.max_distance = 0.1f;
    m_fdr_params.iterations = 10000;
    m_fdr_params.max_vertex_movement = 0.01f;
    m_fdr_params.slow_factor = 0.01f;
    m_fdr_params.max_force = 1.0f;
    m_fdr_params.originalPosAttraction = 0.1;
}

void Graph::GEM_initParameters()
{
    m_nrm = 0.0015;  // normalization factor
    m_gravity = 1/16;  // gravitational constant
    m_edge_size = 128 * m_nrm;    // desired edge size
    m_edge_sizesquared *= m_edge_sizesquared;
    m_Tmin = 3;         // min temperature
    m_Tmax = 256;
    m_a_r = PI/6;          // PI/6  (opening angle for rotation detection)
    m_a_o = PI/2;          // PI/2  (opening angle for oscilation detection)
    m_s_r = 1/(2*m_nodes.size() + 1);  // sensitivity towards rotation
    m_s_o = 1/3;          // 1/3    sensitivity towards oscilation
    m_Tinit = std::sqrt(m_nodes.size());        // initial temperature for a vertex
    m_Tglobal = m_Tinit * m_nodes.size();      // temperature sum, average of te local temp over all vertices
    m_rounds = 40 * m_nodes.size();       // max number of rounds
    m_barycentric = QVector2D(0, 0);
}

Graph::~Graph()
{
    qDebug() << "Func: ~Graph";
    for( auto iter = m_nodes.begin(); iter != m_nodes.end(); iter++) {
        delete (*iter).second;
    }

    delete hashGrid;
}

// to construct nodes list
// I need: hvgxID, type, center,
// init m_neurites_nodes ssbo here
// edges simple just end points
bool Graph::parseNODE_NODE(std::vector<Node*> neurites_nodes, std::vector<QVector2D> neurites_edges)
{
    for (int i = 0; i < neurites_nodes.size(); i++) {
        Node *node = neurites_nodes[i];
        int hvgxID = node->getID();
        QVector3D position = node->get3DPosition();
        std::pair<int, int> id_tuple =  std::make_pair(node->getID(), -1);
        Object_t object_type = m_opengl_mngr->getObjectTypeByID(hvgxID);
        Node_t node_type =  Node_t::NONE;
        if (object_type == Object_t::ASTROCYTE)
            node_type = Node_t::ASTROCYTE;

        this->addNode(id_tuple, position.x(), position.y(), position.z(), node_type);
    }

    for (int i = 0; i < neurites_edges.size(); i++) {
        int nID1 = neurites_edges[i].x();
        int nID2 = neurites_edges[i].y();
        this->addEdge(i, -1, nID1, nID2);
    }

    return true;
}

// I need away to mark all nodes skeleton with the same ID,
// and give each node unique ID
// combination? <hvgxID, local node ID>

// to construct skeleton list
// I need: hvgxID, type, center,
// init m_neurites_nodes ssbo here

// edges simple just end points

// std::pair<int, int> id_tuple, float x, float y, float z
// int eID, int hvgxID, int nID1, int nID2
bool Graph::parseSKELETON(std::vector<Node*> neurites_skeletons_nodes,
                          std::vector<QVector4D> neurites_skeletons_edges)
{
    qDebug() << "parseSKELETON";
    // eventually all graph nodes are in one map with unique IDs regardless of which skeleton
    // they belong to.
    // iterate over the objets
    for (int i = 0; i < neurites_skeletons_nodes.size(); i++) {
        Node *node = neurites_skeletons_nodes[i];
        int hvgxID = node->getID();
        QVector3D position = node->get3DPosition();
        std::pair<int, int> id_tuple =  std::make_pair(hvgxID,  node->getIdxID());
        // I can access the objects using their IDs then get the type
        Object_t object_type = m_opengl_mngr->getObjectTypeByID(hvgxID);
        Node_t node_type =  Node_t::NONE;
        if (object_type == Object_t::ASTROCYTE)
            node_type = Node_t::ASTROCYTE;

        this->addNode(id_tuple, position.x(), position.y(), position.z(), node_type /* need to assin for each node their type */);
    }

    for (int i = 0; i < neurites_skeletons_edges.size(); i++) {
        int j = neurites_skeletons_edges[i].x();
        int hvgxID = neurites_skeletons_edges[i].y();
        int nID1 = neurites_skeletons_edges[i].z();
        int nID2 = neurites_skeletons_edges[i].w();
        this->addEdge(j, hvgxID, nID1, nID2);
    }

    return true;
}


Node* Graph::addNode(std::pair<int, int> id_tuple, float x, float y, float z, Node_t node_type)
{
    size_t idxID = id_tuple.second;
    int nID = id_tuple.first;
    Node* newNode = new Node(nID, idxID, x, y, z, node_type);
    m_nodes[id_tuple] = newNode; // these IDs should be unique per graph!q
    return newNode;
}


Edge* Graph::addEdge(int eID, int hvgxID, int nID1, int nID2)
{
    std::pair<int, int> id_tuple1, id_tuple2;


    if (m_gType == Graph_t::NODE_NODE) {
        id_tuple1 =  std::make_pair(nID1, -1); // this could be between two different hvgx IDs
        id_tuple2 =  std::make_pair(nID2, -1);
    } else {
        id_tuple1 =  std::make_pair(hvgxID, nID1);
        id_tuple2 =  std::make_pair(hvgxID, nID2);
    }

    if (id_tuple1 == id_tuple2) {
     //   qDebug() << "Duplicates " << nID1 << " " << nID2;
        m_dupEdges++;
        return NULL;
    }

    Node *n1 = getNode(id_tuple1);
    Node *n2 = getNode(id_tuple2);

    if (n1 == NULL || n2 == NULL) {
   //     qDebug() << "cant insert " << nID1 << " and " << nID2;
        return NULL;
    }

    int idxID = m_edges.size();
    Edge* newEdge = new Edge(eID, idxID, n1, n2);
    n1->addEdge(newEdge);
    n2->addEdge(newEdge);

    m_edges[newEdge->getID()] = newEdge; // ID should be unique then!

    return newEdge;
}

Node* Graph::getNode(std::pair<int, int> id_tuple)
{
    if (m_nodes.find(id_tuple) != m_nodes.end()) {
        return m_nodes[id_tuple];
    }

    //qDebug() << id_tuple << " doesnt exist.";
    return NULL;
}

Edge* Graph::getEdge(int eID)
{
    if (m_edges.find(eID) != m_edges.end()) {
        return m_edges[eID];
    }

    //qDebug() << eID << " doesnt exist.";
    return NULL;
}


/************************ Spatial Hashing *********************************/
void Graph::updateNode(Node *node)
{
    hashGrid->updateNode(node);
    return;
}


/************************ Force Directed Layout ****************************/
// when we switch to 2D we use the other vertex with the no rotation matrix
// each time we filter we need to reset this to exclude filtered nodes
void Graph::resetCoordinates()
{
    if (m_opengl_mngr == NULL)
        return;

    m_FDL_terminate = false;
    std::map<int, Object*>  objectMap = m_opengl_mngr->getObjectsMap();

    hashGrid->clear();
    m_FilteredHVGX.clear();

    GEM_initParameters();

    // this take so much time that by the time we reach it still do this?
    // create many threads within one thread?
    for( auto iter = m_nodes.begin(); iter != m_nodes.end(); iter++) {
        if (m_FDL_terminate) goto quit;

        Node *node = (*iter).second;

        // if node filtered then ignore it
        //if node is filtered then continue
        // here I can get the list of filtered objects
        int hvgxID = node->getID() ;
        if ( objectMap.find(hvgxID) == objectMap.end() ||
             objectMap[hvgxID] == NULL )
        {
            qDebug() << hvgxID << " Node not in map, why? quite layouting";
            goto quit;
        }

        if (objectMap[hvgxID]->isFiltered()) {
            m_FilteredHVGX[hvgxID] = 1;
            continue;
        }


        node->resetLayout(m_uniforms.rMatrix);
        node->setLocalTemp(m_Tinit);

        update_node_data(node);

        // add to the spatial hash
        hashGrid->insert((*iter).second);
    }

    if (m_gType == Graph_t::NODE_NODE  || m_gType == Graph_t::NODE_SKELETON)
        runforceDirectedLayout();
   // GEM_run();


quit:
qDebug() << "Exist resetCoordinates" ;
}

void Graph::update_node_data(Node* node)
{
    if (m_opengl_mngr == NULL)
        return;


    switch(m_gType) {
        case Graph_t::NODE_NODE :
        {
            // layout1 using hvgx ID and -1
            m_opengl_mngr->update_ssbo_data_layout1(node->getLayoutedPosition(),
                                                    node->getID() /*hvgx*/);
            break;
        }
        case Graph_t::NODE_SKELETON :
        {
            // if node belong to neurite then
            // layout2 using hvgx ID and -1

            // else, astrocyte skeleton
            if (node->getNodeType() == Node_t::ASTROCYTE) {
                // update astrocyte skeleton
                // layout 2
                m_opengl_mngr->update_skeleton_layout2(node->getLayoutedPosition(),
                                                       node->getIdxID(), node->getID() /*hvgx*/);
            } else {
                m_opengl_mngr->update_ssbo_data_layout2(node->getLayoutedPosition(),
                                                        node->getID() );
            }

            break;
        }
        case Graph_t::NEURITE_SKELETONS :
        {
            m_opengl_mngr->update_skeleton_layout3(node->getLayoutedPosition(),
                                                   node->getIdxID(), node->getID() /*hvgx*/);
            break;
        }
        case Graph_t::ALL_SKELETONS :
        {
            m_opengl_mngr->update_skeleton_layout1(node->getLayoutedPosition(),
                                                   node->getIdxID(), node->getID() /*hvgx*/);
            break;
        }
    }
}

// how to remove filtered objects from graph?????
// for each node check if it is filtered
// if yes ignore it and proceed, but this would wast time
// I need to remove it from graph!!
// dont filter in the connectivity graph??
void Graph::runforceDirectedLayout()
{
    std::map<int, Object*>  objectMap = m_opengl_mngr->getObjectsMap();

    m_FDL_terminate = false;
    float area = 1.0;
    float k = std::sqrt( area / m_nodes.size() );
    std::vector<Node*> nearNodes;
    // reset layouted coordinates to original values
    for ( int i = 0; i < m_fdr_params.iterations; i++ ) {
        if (m_FDL_terminate) goto quit;

        // forces on nodes due to node-node repulsion
        for ( auto iter = m_nodes.begin(); iter != m_nodes.end(); iter++ ) {
            if (m_FDL_terminate) goto quit;

            Node *node1 = (*iter).second;
            // if node is filtered then continue
            // not all nodes belong to an object
            // there is the connectivity node
            int hvgxID1 = node1->getID() ;
            if (!(m_FilteredHVGX.find(hvgxID1) == m_FilteredHVGX.end()) )
            {

                continue;
            }

            nearNodes.clear();
            hashGrid->queryAABB(node1, m_fdr_params.AABBdim, nearNodes);
            for ( auto iter2 = nearNodes.begin(); iter2 != nearNodes.end(); iter2++ ) {
                if (m_FDL_terminate) goto quit;

                Node *node2 = (*iter2);
                int hvgxID2 = node2->getID() ;
                if ( !(m_FilteredHVGX.find(hvgxID2) == m_FilteredHVGX.end()) )
                {
                    continue;
                }

                repulseNodes(node1, node2, m_fdr_params.Cr * k);
            }

           attractToOriginalPosition(node1, m_fdr_params.originalPosAttraction); // the less the more
        }

        // forcs due to edge attraction
        for ( auto iter = m_edges.begin(); iter != m_edges.end(); iter++ ) {
            if (m_FDL_terminate) goto quit;

            Edge *edge = (*iter).second;
            attractConnectedNodes(edge, m_fdr_params.Ca * k);
        }

        float moveAccumlation = 0.0;
        // update nodes position fter force
        for ( auto iter = m_nodes.begin(); iter != m_nodes.end(); iter++) {
            if (m_FDL_terminate) goto quit;

            Node *node = (*iter).second;

            //if node is filtered then continue
            int hvgxID = node->getID() ;
            if ( !(m_FilteredHVGX.find(hvgxID) == m_FilteredHVGX.end()) )
            {
                continue;
            }

            // get amount of force on node
            QVector2D force = m_fdr_params.slow_factor * node->getForceSum();
            if ( force.length() > m_fdr_params.max_force ) {
                force = force.normalized() * m_fdr_params.max_force;
            }
            // calculate how much to move
            float xMove = force.x();
            float yMove = force.y();

            // check if this would take the node outside the bounding area
            // if yes, then compute the torque, and apply that instead

            // limit the movement to the maximum defined
            if ( xMove > m_fdr_params.max_vertex_movement )   {
                xMove = m_fdr_params.max_vertex_movement;
            }
            if ( xMove < -m_fdr_params.max_vertex_movement )   {
                xMove = -m_fdr_params.max_vertex_movement;
            }
            if ( yMove > m_fdr_params.max_vertex_movement )    {
                yMove = m_fdr_params.max_vertex_movement;
            }
            if ( yMove < -m_fdr_params.max_vertex_movement )  {
                yMove = -m_fdr_params.max_vertex_movement;
            }

            moveAccumlation += std::abs(xMove) + std::abs(yMove);
            node->addToLayoutedPosition(QVector2D(xMove, yMove)); // add to 2D position

            // update node value in m_nodes buffer
            update_node_data(node);

            // reset node force
            node->resetForce();

            // update node position in spatial hash
            updateNode(node);
        }
    } // end iterations

quit:
qDebug() << "Exist Thread" ;

}


void Graph::attractToOriginalPosition(Node *node, float k)
{
    QVector2D layoutedXY = node->getLayoutedPosition();
    QVector2D projectdXY = node->getProjectedVertex();
    QVector2D vforce  = attractionForce(layoutedXY.x(), layoutedXY.y(), projectdXY.x(), projectdXY.y(), k);

    // Attract: node ----------> original position
    node->addToForceSum(vforce);
}


void Graph::attractConnectedNodes(Edge *edge, float k)
{
    Node *node1 = edge->getNode1();
    Node *node2 = edge->getNode2();
    if ( !(m_FilteredHVGX.find(node1->getID()) == m_FilteredHVGX.end()) || !(m_FilteredHVGX.find(node2->getID()) == m_FilteredHVGX.end()) )
    {
        return;
    }

    QVector3D n1 = node1->getLayoutedPosition();
    QVector3D n2 = node2->getLayoutedPosition();

    QVector2D force = attractionForce(n1.x(), n1.y(), n2.x(), n2.y(), k);

    // attract: node1 -> ------- <- node2
    node1->addToForceSum( force );
    node2->addToForceSum( -force );
}

// node1: node creating the force
// node2: node the force is acting on
void  Graph::repulseNodes(Node *node1, Node *node2, float k)
{
    QVector3D n1 = node1->getLayoutedPosition();
    QVector3D n2 = node2->getLayoutedPosition();

    QVector2D force = repulsiveForce(n1.x(), n1.y(), n2.x(), n2.y(), k);

    // Repel: <- node1 -------- node2 ->
    node1->addToForceSum( -force );
}

QVector2D  Graph::attractionForce(float x1, float y1, float x2, float y2, float k)
{
    QVector2D force;
    float attraction;
    // node1 -------> node2
    float dx = x2 - x1;
    float dy = y2 - y1;
    QVector2D dxy = QVector2D(dx, dy);
    float distance = dxy.length();
    float distanceSquared = dxy.lengthSquared();
    if (distance < 0.0001) { // too small
        dx = 0.1f * (std::rand() % 100) / 100.0f + 0.1;
        dy = 0.1f * (std::rand() % 100) / 100.0f + 0.1;
        distanceSquared = dx * dx + dy * dy;
    }


    if (distance > m_fdr_params.max_distance) {
         distance = m_fdr_params.max_distance;
         distanceSquared = distance * distance;
     }


    // fa(d) = d^2/k
    // fa(d) = K_s * ( distance - L) (hooke's law)
    // fa(d) = c1 * log(d/c2) -> c1 = 2, c2 = 1
    // fa(d) = -k * d
    attraction = (distanceSquared /*- k*k*/) / k;
    // Hooke's Law: F = -kx

    force =  dxy.normalized() * attraction;

    return force;
}

QVector2D Graph::repulsiveForce(float x1, float y1, float x2, float y2, float k)
{
    // get the difference vector between the positions of the two nodes
    QVector2D force = QVector2D(0.0, 0.0);

    // node 1 -----> node2
    float dx = x2 - x1;
    float dy = y2 - y1;
    float repulsion;
    QVector2D dxy = QVector2D(dx, dy);
    float distance = dxy.length() + 0.0001; // to avoid 0

    // 1) fr(d) = -k^2/d
    // 2) fr(d) = Kr / d^2 (Coulomb's law)
    // 3) fr(d) = c3/d2 -> c3 = 1
    // 4) fr(d) = k/d2
    // Coulomb's Law: F = k(Qq/r^2)

    if (distance <= m_fdr_params.AABBdim) {
        repulsion =    (k * k) / distance;
        force =  dxy.normalized() * repulsion;
    }

    return force;
}



// 1) local temperature
// 2) attracting vertices towards barycenter
// 3) detection of oscillations and rotations
void Graph::GEM_run()
{
    m_FDL_terminate = false;

    qDebug() << "Run GEM Layouting Algorithm";
    int rounds = 0;


    // two stages: initialization and iteration
    // initialization:  initial position (no need), impulse, temperature ----> this is constructed when new node is created

    // iteration loop:

    while(m_Tglobal > m_Tmin && rounds < m_rounds) {
//        if (m_FDL_terminate) goto quit;
        // updates vertex positions until the global temp is lower than a desired minimal temp or the time has expired
        for ( auto iter = m_nodes.begin(); iter != m_nodes.end(); iter++ ) {
            if (m_Tglobal < m_Tmin) {
                break;
            }

//            if (m_FDL_terminate) goto quit;

            // get a vertex v to update
            Node *node = (*iter).second;
            m_Tglobal = m_Tglobal - node->getLocalTemp();

            // compute v's impulse
            QVector2D impulse = GEM_computeImpulse( node );
             // update position and temperature
            GEM_update_node(node, impulse);

            m_Tglobal = m_Tglobal + node->getLocalTemp();

        }

        rounds++;
    }


//quit:
//qDebug() << "Exist Thread" ;

}

QVector2D Graph::GEM_computeImpulse(Node *node)
{
    QVector2D impulse;
    std::map<int, Edge*> edges = node->getAdjEdges();
    QVector2D vNode = node->getLayoutedPosition();

    // 1) attract to original projected location
    QVector2D layoutedXY = node->getLayoutedPosition();
    QVector2D projectdXY = node->getProjectedVertex();
    impulse =  (projectdXY -  layoutedXY) * m_gravity * GEM_function_growing(node);

    // 2) random disturbance
//     QVector2D disturbance; //  small random vector (length of < 0.07)
//     disturbance.setX((std::rand()));
//     disturbance.setY((std::rand()));
//     disturbance /= (RAND_MAX * 10000);
//     impulse = impulse + disturbance;
//     qDebug() << "*** " << disturbance.length();

    // 3) repulsive forces
    // for all neighboring nodes u within a distance d

     // forces on nodes due to node-node repulsion


     std::vector<Node*> nearNodes;
     hashGrid->queryAABB(node, m_fdr_params.AABBdim, nearNodes);
     for ( auto iter2 = nearNodes.begin(); iter2 != nearNodes.end(); iter2++ ) {

         Node *node2 = (*iter2);
         int hvgxID2 = node2->getID() ;
         if ( !(m_FilteredHVGX.find(hvgxID2) == m_FilteredHVGX.end()) )
         {
             continue;
         }

         QVector2D vNode2 = node2->getLayoutedPosition();

         QVector2D delta = vNode - vNode2;
         if (delta != QVector2D(0, 0))
            impulse = impulse + delta * m_edge_sizesquared / (delta.lengthSquared());
     }

    // 4) attractive forces

     // forcs due to edge attraction
     // get all nodes connected to this node
    for ( auto iter = edges.begin(); iter != edges.end(); iter++ ) {

        Edge *edge = (*iter).second;
        Node *node2 = edge->getNode1();;
        if (node2 == node)
            node2 = edge->getNode2();
        attractConnectedNodes(edge, m_fdr_params.Ca );

        QVector2D vNode2 = node2->getLayoutedPosition();
        // for nodes u connected to v
        QVector2D delta = vNode - vNode2;
       // impulse = impulse - delta * delta.lengthSquared()/(m_edge_sizesquared *  GEM_function_growing(node));

    }


    return impulse;
}

double Graph::GEM_function_growing(Node *node)
{
    double result;
    result =  ((double)node->getDeg()  + (double) node->getDeg() /2.0);
    return result;
}

void Graph::GEM_update_node(Node *node, QVector2D impulse)
{

    if( impulse != QVector2D(0, 0) ){
        impulse = node->getLocalTemp() * impulse.normalized() * m_nrm; // scale p by temperature
        node->addToLayoutedPosition(impulse); // move v to new position
        m_barycentric = m_barycentric + impulse; // update sum of points
    }

   if (impulse != QVector2D(0, 0) ) {
        // cehck previous p
        double B = GEM_get_angle(impulse, node->getImpulse());
        if (std::sin(B)  >= std::sin(PI/2 + m_a_r/2) ) {
            double sign = (std::sin(B) > 0) ? 1 : -1;
            node->updateSkew( node->getSkew() + m_s_r  * sign); // rotation
        }

        if ( std::abs( std::cos(B) ) > std::cos(m_a_o/2) )
            node->setLocalTemp( node->getLocalTemp() * m_s_o * std::cos(B));

        double newLocalTemp = node->getLocalTemp() * ( 1 - std::abs(node->getSkew()) );
        newLocalTemp = std::min(newLocalTemp, (double)m_Tmax);
        node->setLocalTemp( newLocalTemp );  // cap to mximum

        node->updateImpulse(impulse); // save impulse
        update_node_data(node);
  }

}

double Graph::GEM_get_angle(QVector2D u, QVector2D v)
{
    double angle;
    angle = std::acos(u.dotProduct(v, u) / v.length() * u.length());
    return angle;
}


