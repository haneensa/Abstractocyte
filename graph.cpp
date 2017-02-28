#include "graph.h"

Graph::Graph(Graph_t graphType, OpenGLManager *opengl_mnger)
    : m_FDL_terminate(true),
      m_nodesCounter(0),
      m_edgesCounter(0),
      m_dupEdges(0)


{
    m_gType = graphType;
    m_opengl_mngr = opengl_mnger;
    // force directed layout parameters
    m_Cr = 1.5;
    m_Ca = 0.5;
    m_AABBdim = 0.15f; // used for spatial hashing query dim
    m_MAX_DISTANCE = 0.1f;
    m_ITERATIONS = 10000;
    m_MAX_VERTEX_MOVEMENT = 0.01f;
    m_SLOW_FACTOR = 0.01f;
    m_MAX_FORCE = 1.0f;

    // spatial hashing
    int gridCol = 5;
    float gridMin = 0.0f;
    float gridMax = 1.0f;
    hashGrid = new SpatialHash(gridCol, gridMin, gridMax);
}

void Graph::initGemParameters()
{
    m_nrm = 0.0015;  // normalization factor
    m_gravity = 1/16;  // gravitational constant
    m_edge_size = 128 * m_nrm;    // desired edge size
    m_Tmin = 3;         // min temperature
    m_Tmax = 256;
    m_a_r = PI/6;          // PI/6
    m_a_o = PI/2;          // PI/2
    m_s_r = 1/(2*m_nodesCounter);          // 1/2n
    m_s_o = 1/3;          // 1/3
    m_Tinit = std::sqrt(m_nodesCounter);        // initial temperature for a vertex
    m_Tglobal;      // Tinit * n (temperature sum)
    m_rounds;       // max number of rounds

}

// later refactor this
void Graph::updateGraphParam1(double value)
{
    qDebug() << "Updating Cr to " << value;
    m_Cr = value;
}

void Graph::updateGraphParam2(double value)
{
    qDebug() << "Updating Ca to " << value;
    m_Ca = value;
}

void Graph::updateGraphParam3(double value)
{
    qDebug() << "Updating AABBdim to " << value;
    m_AABBdim = value;
}

void Graph::updateGraphParam4(double value)
{
    qDebug() << "Updating m_MAX_DISTANCE to " << value;
    m_MAX_DISTANCE = value;
}

void Graph::updateGraphParam5(double value)
{
    qDebug() << "Updating m_MAX_VERTEX_MOVEMENT to " << value;
    m_MAX_VERTEX_MOVEMENT = value;
}

void Graph::updateGraphParam6(double value)
{
    qDebug() << "Updating m_SLOW_FACTOR to " << value;
    m_SLOW_FACTOR = value;
}

void Graph::updateGraphParam7(double value)
{
    qDebug() << "Updating m_MAX_FORCE to " << value;
    m_MAX_FORCE = value;
}

Graph::~Graph()
{
    qDebug() << "Func: ~Graph";
    for( auto iter = m_nodes.begin(); iter != m_nodes.end(); iter++) {
        delete (*iter).second;
    }

    delete hashGrid;
}

bool Graph::createGraph(DataContainer *objectManager)
{

    return true;
}

// to construct nodes list
// I need: hvgxID, type, center,
// init m_neurites_nodes ssbo here

// edges simple just end points
bool Graph::parseNODE_NODE(std::vector<Node*> neurites_nodes, std::vector<QVector2D> neurites_edges)
{
    for (int i = 0; i < neurites_nodes.size(); i++) {
        Node *node = neurites_nodes[i];
        QVector3D position = node->get3DPosition();
        std::pair<int, int> id_tuple =  std::make_pair(node->getID(), -1);
        this->addNode(id_tuple, position.x(), position.y(), position.z());
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
        QVector3D position = node->get3DPosition();
        std::pair<int, int> id_tuple =  std::make_pair(node->getID(),  node->getIdxID());
        this->addNode(id_tuple, position.x(), position.y(), position.z());
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


Node* Graph::addNode(std::pair<int, int> id_tuple, float x, float y, float z)
{
    size_t idxID = id_tuple.second;
    int nID = id_tuple.first;
    Node* newNode = new Node(nID, idxID, x, y, z);
    m_nodes[id_tuple] = newNode; // these IDs should be unique per graph!q
    m_nodesCounter++;
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

    int idxID = m_edgesCounter++;
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

void Graph::initGridBuffers()
{
    qDebug() << "initGridBuffers";
    hashGrid->initOpenGLFunctions();
    if (hashGrid->init_Shaders_Buffers() == false) {
        qDebug() << "error!";
        return;
    }
}

void Graph::drawGrid(struct GlobalUniforms grid_uniforms)
{
    hashGrid->drawGrid(grid_uniforms);
}

/************************ Force Directed Layout ****************************/
// when we switch to 2D we use the other vertex with the no rotation matrix
void Graph::resetCoordinates(QMatrix4x4 rotationMatrix)
{
    if (m_opengl_mngr == NULL)
        return;

    qDebug() << "2D";
    hashGrid->clear();
    for( auto iter = m_nodes.begin(); iter != m_nodes.end(); iter++) {
        Node *node = (*iter).second;

        node->resetLayout(rotationMatrix);
        update_node_data(node);

        // add to the spatial hash
        hashGrid->insert((*iter).second);
    }

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
                qDebug() << "Update skeleton layout 2 vertex";
                m_opengl_mngr->update_skeleton_layout2(node->getLayoutedPosition(),
                                                       node->getIdxID(), node->getID() /*hvgx*/);
            } else {
                m_opengl_mngr->update_ssbo_data_layout2(node->getLayoutedPosition(),
                                                        node->getID() /*hvgx*/);
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

void Graph::runforceDirectedLayout()
{
    qDebug() << "run force directed layout";
    m_FDL_terminate = false;
    float area = 1.0;
    float k = std::sqrt( area / m_nodesCounter );
    std::vector<Node*> nearNodes;
    // reset layouted coordinates to original values
    for ( int i = 0; i < m_ITERATIONS; i++ ) {
        if (m_FDL_terminate) goto quit;

        qDebug() << "Iteration # " << i;

        // forces on nodes due to node-node repulsion
        for ( auto iter = m_nodes.begin(); iter != m_nodes.end(); iter++ ) {
            if (m_FDL_terminate) goto quit;

            Node *node1 = (*iter).second;
            nearNodes.clear();
            hashGrid->queryAABB(node1, m_AABBdim, nearNodes);
            for ( auto iter2 = nearNodes.begin(); iter2 != nearNodes.end(); iter2++ ) {
                if (m_FDL_terminate) goto quit;

               // Node *node2 = (*iter2).second;
                Node *node2 = (*iter2);

                // be careful with this condition, because a skeleton has the same ID for all its nodes
                //if ( node1->getID() == node2->getID() ) {
                //    continue;
                //}
                // this one,
                repulseNodes(node1, node2, m_Cr * k);
            }
        }

        // forcs due to edge attraction
        for ( auto iter = m_edges.begin(); iter != m_edges.end(); iter++ ) {
            if (m_FDL_terminate) goto quit;

            Edge *edge = (*iter).second;
            attractConnectedNodes(edge, m_Ca * k);
        }

        float moveAccumlation = 0.0;
        // update nodes position fter force
        for ( auto iter = m_nodes.begin(); iter != m_nodes.end(); iter++) {
            if (m_FDL_terminate) goto quit;

            Node *node = (*iter).second;

            // get amount of force on node
            QVector2D force = m_SLOW_FACTOR * node->getForceSum();
            if ( force.length() > m_MAX_FORCE ) {
                qDebug() << "MAX_FORCE: " << m_MAX_FORCE;
                force = force.normalized() * m_MAX_FORCE;
            }
            // calculate how much to move
            float xMove = force.x();
            float yMove = force.y();

            // check if this would take the node outside the bounding area
            // if yes, then compute the torque, and apply that instead

            // limit the movement to the maximum defined
            if ( xMove > m_MAX_VERTEX_MOVEMENT )   {
                qDebug() << "xMove: " << xMove;
                xMove = m_MAX_VERTEX_MOVEMENT;
            }
            if ( xMove < -m_MAX_VERTEX_MOVEMENT )   {
                qDebug() << "xMove: " << xMove;
                xMove = -m_MAX_VERTEX_MOVEMENT;
            }
            if ( yMove > m_MAX_VERTEX_MOVEMENT )    {
                qDebug() << "xMove: " << yMove;
                yMove = m_MAX_VERTEX_MOVEMENT;
            }
            if ( yMove < -m_MAX_VERTEX_MOVEMENT )  {
                qDebug() << "xMove: " << yMove;
                yMove = -m_MAX_VERTEX_MOVEMENT;
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

void Graph::attractConnectedNodes(Edge *edge, float k)
{
    Node *node1 = edge->getNode1();
    Node *node2 = edge->getNode2();

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
        qDebug() << "Too small distance " << distance;
        dx = 0.1f * (std::rand() % 100) / 100.0f + 0.1;
        dy = 0.1f * (std::rand() % 100) / 100.0f + 0.1;
        distanceSquared = dx * dx + dy * dy;
    }


    if (distance > m_MAX_DISTANCE) {
         qDebug() << "MAX_DISTANCE: " << distance;
         distance = m_MAX_DISTANCE;
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

    if (distance <= m_AABBdim) {
        repulsion =    (k * k) / distance;
       // repulsion =    (k * k) * (distance - m_MAX_DISTANCE);

        force =  dxy.normalized() * repulsion;
    }

    return force;
}

void Graph::runGEM()
{
    qDebug() << "Run GEM Layouting Algorithm";
    int rounds = 0;

    while(m_Tglobal/m_nodesCounter < m_Tmin && rounds < m_rounds) {
        for (int i = 0; i < m_nodesCounter; i++) {
            if (m_Tglobal/m_nodesCounter < m_Tmin)
                break;

            // get a vertex v
            // Tglobal = Tglobal - v.t
            // p = compute_impulse(v)
            // update(v, p) // update position and temperature
            // Tglobal = Tglobal + v.t

        }

        rounds++;
    }
}

double Graph::computeImpulse(/*vertex*/)
{
    // attract to original projected location
    // p = (v.opos - v.pos) * g * function_growing(v)

    // random disturbance
    // b = small random vector (length of < 0.07)
    // p = p + b

    // repulsive forces
    // for all neighboring nodes u within a distance d
        // delta = v.pos - u.pos
        // if (delta != 0)
            // p = p + delta * E * E/(delta_length^2)

    // attractive forces
        // for nodes u connected to v
            // delta = v.pos - u.pos
            // p = p - delta * (delta_length^2)/(E*E * function_growing(v))

    // return p
    return 0.1;
}

double Graph::function_growing(/*vertex*/)
{
    double result;
    // (deg(v) + deg(v)/2) * nrm
    return result;
}

void Graph::update_node(/*vertex and impulse*/)
{
    /*
     * if p != 0
     *      p = v.t * normalize(p) * nrm // scale p by temperature
     *      v.pos = v.pos + p // move v to new position
     *      c = c + p // update sum of points
     *
     * if v.p != 0:
     *      // cehck previous p
     *      B = get_angle(p, v.p)
     *      if (sin(B) >= sim(PI/2 + ar)
     *          v.d = v.d + s_r  sign(sin B) // rotation
     *
     *      if abs( cos(B) ) > cos(a_o)
     *          v.t = v.t * s_o * cos(B) // oscillation
     *
     *      v.t = v.t * ( 1 - abs(v.d) )
     *      v.t = min(v.t, Tmax)    // cap to mximum
     *
     *      v.p = p // save impulse
     */
}

double Graph::get_angle()
{
    double angle;
    // angle = arc_cosine(dot(u, v) / length(v) * length(u) )
    return angle;
}


