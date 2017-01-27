#include "graph.h"

Graph::Graph()
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

    QTextStream in (&file);
    QList<QByteArray> wordList;
    while(!file.atEnd()) {
        QByteArray line = file.readLine();
        wordList = line.split(',');
        if (wordList.size() < 4) {
            qDebug() << "something wrong with input file";
            return false;
        }

        int nID = atoi(wordList[0].data());
        float x = atof(wordList[1].data());
        float y = atof(wordList[2].data());
        float z = atof(wordList[3].data());

        // type, size, glycogen around this node

        // add node
        Node* newNode = this->addNode(nID, x, y, z);
        QVector3D coord3D = QVector3D(x, y, z);
        QVector2D coord2D = QVector2D(z, y);

        struct BufferNode bnode = {coord3D, coord2D};
        // ids assigned to nods should match the indices in this vector!
        m_bufferNodes.push_back(bnode);
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
    while(!file.atEnd()) {
        QByteArray line = file.readLine();
        wordList = line.split(',');
        if (wordList.size() < 3) {
            qDebug() << "something wrong with input file";
            return false;
        }

        int eID = atoi(wordList[0].data());
        float nID1 = atof(wordList[1].data());
        float nID2 = atof(wordList[2].data());

        // add edge
        this->addEdge(nID1, nID2);
    }

    qDebug() << "# edges: " << m_edges.size();
    file.close();
    return true;
}

Node* Graph::addNode(int nID, float x, float y, float z)
{
    Node* newNode = new Node(nID, x, y, z);
    m_nodes[newNode->getID()] = newNode;
    return newNode;
}

Edge* Graph::addEdge(int nID1, int nID2)
{
    if (nID1 == nID2) {
        m_dupEdges++;
        return NULL;
    }

    Node *n1 = getNode(nID1);
    Node *n2 = getNode(nID2);

    if (!nID1 || !nID2) {
        qDebug() << "cant insert " << nID1 << " and " << nID2;
        return NULL;
    }

    Edge* newEdge = new Edge(m_edgesCounter++, n1, n2);
    n1->addEdge(newEdge);
    n2->addEdge(newEdge);

    m_edges[newEdge->getID()] = newEdge;

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

