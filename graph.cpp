#include "graph.h"

Graph::Graph()
    : m_FDL_terminate(true)
{
     m_nodesCounter = 0;
     m_edgesCounter = 0;
     m_dupEdges = 0;


     // force directed layout parameters
     m_Cr = 1.5;
     m_Ca = 0.5;
     m_AABBdim = 0.15f; // used for spatial hashing query dim
     m_MAX_DISTANCE = 0.1f;
     m_ITERATIONS = 10000;
     m_MAX_VERTEX_MOVEMENT = 0.05f;
     m_SLOW_FACTOR = 0.01f;
     m_MAX_FORCE = 1.0f;

     // spatial hashing
     int gridCol = 1;
     float gridMin = 0.0f;
     float gridMax = 1.0f;
     hashGrid = new SpatialHash(gridCol, gridMin, gridMax);
}


Graph::~Graph()
{
    qDebug() << "Func: ~Graph";
    for( auto iter = m_nodes.begin(); iter != m_nodes.end(); iter++) {
        delete (*iter).second;
    }

    delete hashGrid;
}

bool Graph::createGraph(std::vector<Object*> nodes_info, std::vector<QVector2D> edges_info)
{
    // add nodes
    for (int i = 0; i < nodes_info.size(); ++i) {
        int nID = nodes_info[i]->getHVGXID();
        int ssbo_ID = nodes_info[i]->getIdxID();
        QVector4D center = nodes_info[i]->getCenter();
        this->addNode(nID, ssbo_ID, center.x(), center.y(), center.z());
    }

    // add edges
    for (int i = 0; i < edges_info.size(); ++i) {
        int eID = i;
        int nID1 = edges_info[i].x();
        int nID2 = edges_info[i].y();
        this->addEdge(eID, nID1, nID2);
    }

    return true;
}

// nID, x, y, z
bool Graph::loadNodes(QString filename)
{
    qDebug() << "Func: loadNodes ( "<< filename << " )";
    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "could not open the file for reading.";
        return false;
    }
    int flag = 0;
    QTextStream in (&file);
    QList<QByteArray> wordList;
    while(!file.atEnd()) {
        QByteArray line = file.readLine();
        wordList = line.split(',');
        if (wordList.size() < 4) {
            qDebug() << "something wrong with input file";
            return false;
        }
        if (flag == 0) {
            flag = 1;
            continue;
        }
        int nID = atoi(wordList[0].data());
        float x = atof(wordList[2].data())/*200.0*/;
        float y = atof(wordList[3].data())/*200.0*/;
        float z = atof(wordList[4].data())/*200.0*/;
        // type, size, glycogen around this node

        // add node
        this->addNode(nID, 0, x, y, z);

    }
    qDebug() << "# nodes: " << m_nodes.size();

    file.close();
    return true;
}

// eID, nID1, nID2
bool Graph::loadEdges(QString filename)
{
    qDebug() << "Func: loadEdges ( "<< filename << " )";
    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "could not open the file for reading.";
        return false;
    }

    QTextStream in (&file);
    QList<QByteArray> wordList;
    int flag = 0;
    while(!file.atEnd()) {
        QByteArray line = file.readLine();
        wordList = line.split(',');
        if (wordList.size() < 3 ) {
            qDebug() << "something wrong with input file";
            return false;
        }

        if (flag == 0) {
            flag = 1;
            continue;
        }

        int eID = atoi(wordList[0].data());
        float nID1 = atof(wordList[1].data());
        float nID2 = atof(wordList[2].data());
        // add edge
       // qDebug() << eID << " " << nID1 << " " << nID2;
        this->addEdge(eID, nID1, nID2);
    }

    qDebug() << "# edges: " << m_edges.size();
    file.close();
    return true;
}

Node* Graph::addNode(int nID, int ssboID, float x, float y, float z)
{
    size_t idxID = m_bufferNodes.size();
    Node* newNode = new Node(nID, idxID, x, y, z);
    m_nodes[newNode->getID()] = newNode; // these IDs should be unique per graph!q
    m_nodesCounter++;

    QVector3D coord3D = QVector3D(x, y, z);
    QVector2D coord2D = QVector2D(x, y);

    struct BufferNode bnode = {coord3D, coord2D, ssboID};
    // ids assigned to nods should match the indices in this vector!
    m_bufferNodes.push_back(bnode);

    return newNode;
}

Edge* Graph::addEdge(int eID, int nID1, int nID2)
{
    if (nID1 == nID2) {
        m_dupEdges++;
        return NULL;
    }

    Node *n1 = getNode(nID1);
    Node *n2 = getNode(nID2);

    if (n1 == NULL || n2 == NULL) {
        qDebug() << "cant insert " << nID1 << " and " << nID2;
        return NULL;
    }

    qDebug() << "create edges: " << nID1 << " " << nID2;
    int idxID = m_edgesCounter++;
    Edge* newEdge = new Edge(eID, idxID, n1, n2);
    n1->addEdge(newEdge);
    n2->addEdge(newEdge);

    m_edges[newEdge->getID()] = newEdge; // ID should be unique then!

    m_bufferIndices.push_back(n1->getIdxID());
    m_bufferIndices.push_back(n2->getIdxID());

    return newEdge;
}

Node* Graph::getNode(int nID)
{
    if (m_nodes.find(nID) != m_nodes.end()) {
        return m_nodes[nID];
    }

    qDebug() << nID << " doesnt exist.";
    return NULL;
}

Edge* Graph::getEdge(int eID)
{
    if (m_edges.find(eID) != m_edges.end()) {
        return m_edges[eID];
    }

    qDebug() << eID << " doesnt exist.";
    return NULL;
}

void Graph::allocateBVertices(QOpenGLBuffer vertexVbo)
{
    vertexVbo.allocate( m_bufferNodes.data(),
                        m_bufferNodes.size() * sizeof(struct BufferNode) );
}


void Graph::allocateBIndices(QOpenGLBuffer indexVbo)
{

    indexVbo.allocate( m_bufferIndices.data(),
                        m_bufferIndices.size() * sizeof(GLuint) );
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

void Graph::drawGrid(struct GridUniforms grid_uniforms)
{
    hashGrid->drawGrid(grid_uniforms);
}

/************************ Force Directed Layout ****************************/


void Graph::resetCoordinates(QMatrix4x4 rotationMatrix)
{
    hashGrid->clear();
    for( auto iter = m_nodes.begin(); iter != m_nodes.end(); iter++) {
        Node *node = (*iter).second;
        node->resetLayout(rotationMatrix);
        m_bufferNodes[node->getIdxID()].coord3D = node->getLayoutedPosition();
        // add to the spatial hash
        hashGrid->insert((*iter).second);
    }

}

void Graph::runforceDirectedLayout()
{
    qDebug() << "run force directed layout";
    m_FDL_terminate = false;
    float area = 25.0;
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
            for ( auto iter2 = m_nodes.begin(); iter2 != m_nodes.end(); iter2++ ) {
                if (m_FDL_terminate) goto quit;

                Node *node2 = (*iter2).second;
                if ( node1->getIdxID() == node2->getIdxID() )
                    continue;
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
            m_bufferNodes[node->getIdxID()].coord3D = node->getLayoutedPosition();
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
       // qDebug() << "distance <= m_MAX_DISTANCE: " << distance;
        repulsion =    (k * k) / distance;
       // repulsion =    (k * k) * (distance - m_MAX_DISTANCE);

        force =  dxy.normalized() * repulsion;
    }

    return force;
}
