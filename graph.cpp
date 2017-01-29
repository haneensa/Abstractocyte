#include "graph.h"

Graph::Graph()
{
     m_nodesCounter = 0;
     m_edgesCounter = 0;
     m_dupEdges = 0;

}


Graph::~Graph()
{

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
        float x = atof(wordList[1].data())/200.0;
        float y = atof(wordList[2].data())/200.0;
        float z = atof(wordList[3].data())/200.0;
        qDebug() <<nID << " " << x << " " << y << " " << z;
        // type, size, glycogen around this node

        // add node
        this->addNode(nID, x, y, z);

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
        this->addEdge(eID, nID1, nID2);
    }

    qDebug() << "# edges: " << m_edges.size();
    file.close();
    return true;
}

Node* Graph::addNode(int nID, float x, float y, float z)
{
    size_t idxID = m_bufferNodes.size();
    Node* newNode = new Node(nID, idxID, x, y, z);
    m_nodes[newNode->getID()] = newNode;
    m_nodesCounter++;

    QVector3D coord3D = QVector3D(x, y, z);
    QVector2D coord2D = QVector2D(z, y);

    struct BufferNode bnode = {coord3D, coord2D};
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

    int idxID = m_edgesCounter++;
    Edge* newEdge = new Edge(eID, idxID, n1, n2);
    n1->addEdge(newEdge);
    n2->addEdge(newEdge);

    m_edges[newEdge->getIdxID()] = newEdge;

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
                        m_bufferIndices.size() * sizeof(GLushort) );
}

