// skeletons to take care of : 134, 510
// 419, 56
// 422, 59
#include <chrono>
#include "datacontainer.h"
#include "glycogen.h"

// todo: each object has to have all elements (mesh, skeleton, ..)
// if it doesnt then take care of this case (missing data)
/*
 * m_objects -> object class for each object (astrocyte, dendrite, ..)
 *           -> get from this the indices of the mesh
 *           -> get the skeleton vertices
 */
DataContainer::DataContainer()
{
    m_indices_size = 0;
    m_skeleton_points_size = 0;
    m_limit = 10000;
    m_vertex_offset = 0;
    m_mesh = new Mesh();
    importXML("://scripts/m3_astrocyte.xml");   // astrocyte  time:  79150.9 ms
   importXML("://m3_neurites.xml");    // neurites time:  28802 ms

   // loadMetaDataHVGX(":/data/mouse3_metadata_objname_center.hvgx");

}

DataContainer::~DataContainer()
{
    qDebug() << "~Mesh()";
    for (std::size_t i = 0; i != m_objects.size(); i++) {
        delete m_objects[i];
    }
}

// load this after loading obj file
// get center from here, and volume, and connectivity?
void DataContainer::loadMetaDataHVGX(QString path)
{
    qDebug() << "Func: loadMetaDataHVGX";
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "Could not open the file for reading";
        return;
    }

    QTextStream in(&file);
    QList<QByteArray> wordList;
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        wordList = line.split(',');

        if (wordList[0] == "gc") {
            qDebug() << line;
        } else if (wordList[0] == "sg") {
            continue;
        } else if (wordList[0] == "sy") {
            continue;
        } else if (wordList[0] == "mt") {
            continue;
        } else if (wordList[0] == "bo") {
            continue;
        } else if (wordList[0] == "sp") {
            continue;
        } else if (wordList[0] == "dn") {
            continue;
        } else if (wordList[0] == "ax") {
            continue;
        }


    }

    file.close();
}

// need away to optimize this!!!!!
bool DataContainer::importXML(QString path)
{
    qDebug() << "Func: importXML";
    auto t1 = std::chrono::high_resolution_clock::now();

    QFile  *file = new QFile(path);
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Could not open the file for reading";
        return false;
    }

    m_vertex_offset += m_mesh->getVerticesSize();

    QXmlStreamReader xml(file);
    while( !xml.atEnd() && !xml.hasError() ) {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartDocument) {
            continue;
        }

        if (token == QXmlStreamReader::StartElement) {
            if (xml.name() == "o") {
                if (m_objects.size() > m_limit) {
                    qDebug() << "* Reached size limit.";
                    break;
                }
                Object *obj = NULL;

                // make a thread that would read this ?
                // the astrocyte wast most of the time
                // so if we can optimize reading one object would be better
                parseObject(xml, obj); // fills the object with obj info
            } else if (xml.name() == "conn") {
                parseConnGraph(xml);
           }
        }
    }

    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms = t2 - t1;
    qDebug() << "time: " << ms.count() << "ms";
}

void DataContainer::parseConnGraph(QXmlStreamReader &xml)
{
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() != "conn") {
        qDebug() << "Called XML parseObejct without attribs";
        return;
    }

    qDebug()  << "Parsing: " << xml.name();
    xml.readNext();
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "conn")) {
        // go to the next child of object node
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "l") {
                xml.readNext();
                QString coords = xml.text().toString();
                QStringList stringlist = coords.split(" ");
                if (stringlist.size() < 2) {
                    continue;
                }

                int nodeID1 = stringlist.at(0).toInt();
                int nodeID2 = stringlist.at(1).toInt();

                QVector2D edge_info = QVector2D(nodeID1, nodeID2);
                neurites_neurite_edge.push_back(edge_info);
            }
        } // if start element
        xml.readNext();
    } // while
}

// load the object with all its related informations
void DataContainer::parseObject(QXmlStreamReader &xml, Object *obj)
{
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() != "o") {
        qDebug() << "Called XML parseObejct without attribs";
        return;
    }
    qDebug() << xml.name();

    QString nameline  = xml.attributes().value("name").toString();
    QList<QString> nameList = nameline.split('_');
    QString name;
    for (int i = 0; i < nameList.size() - 1; ++i)
        name += nameList[i];

    int hvgxID = nameList[nameList.size() - 1].toInt();
    obj = new Object(name.toUtf8().constData(), hvgxID);

    // if axon or dendrite then set parent ID
    // Parent -> Child
    // if axon then all boutons afterwards are its children
    // else if dendrite all consecutive spines are its children
    if (obj->getObjectType() == Object_t::AXON || obj->getObjectType() ==  Object_t::DENDRITE) { // assuming: all spines/boutons has parents
        m_curParent = obj;
    } else if (obj->getObjectType() == Object_t::BOUTON ||obj->getObjectType() == Object_t::SPINE) {
        obj->setParentID(m_curParent);
        m_objects[m_curParent->getHVGXID()]->addChild(obj);
    }

    QVector4D color = obj->getColor();
    // set ssbo with this ID to this color

    xml.readNext();

    // this object structure is not done
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "o")) {
        // go to the next child of object node
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "mesh") {
                parseMesh(xml, obj); // takes the most time
            } else if (xml.name() == "skeleton") {
                parseSkeleton(xml, obj);
            } else if (xml.name() == "volume") {
                xml.readNext();
                int volume =  xml.text().toInt();
                obj->setVolume(volume);
                qDebug() << "volume: " << volume;
            } else if (xml.name() == "center") {
                xml.readNext();
                QString coords = xml.text().toString();
                QStringList stringlist = coords.split(" ");
                if (stringlist.size() < 3) {
                    continue;
                }

                float x = stringlist.at(0).toDouble();
                float y = stringlist.at(1).toDouble();
                float z = stringlist.at(2).toDouble();
                qDebug() << "center: " << x << " " << y << " " << z;
                obj->setCenter(QVector4D(x/5.0, y/5.0, z/5.0, 0));
                // set ssbo with this center
            }  else if (xml.name() == "ast_point") {
                // index to astrocyte vertex from the skeleton
                // or the vertex itself
                xml.readNext();
                QString coords = xml.text().toString();
                QStringList stringlist = coords.split(" ");
                if (stringlist.size() < 3) {
                    continue;
                }

                float x = stringlist.at(0).toDouble();
                float y = stringlist.at(1).toDouble();
                float z = stringlist.at(2).toDouble();
                qDebug() << "center: " << x << " " << y << " " << z;
                obj->setAstPoint(QVector4D(x/5.0, y/5.0, z/5.0, 0));
                // set ssbo with this center
            }
        } // if start element
        xml.readNext();
    } // while

    if (obj != NULL) {
        if (hvgxID == 120)
            return;

         m_objects[hvgxID] =  obj;
         std::vector<int> IdsTemp = m_objectsIDsByType[obj->getObjectType()];
         IdsTemp.push_back(obj->getHVGXID());
         m_objectsIDsByType[obj->getObjectType()] = IdsTemp;

    }

}


void DataContainer::parseMesh(QXmlStreamReader &xml, Object *obj)
{
    // read vertices and their faces into the mesh
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() != "mesh") {
        qDebug() << "Called XML parseObejct without attribs";
        return;
    }

    qDebug() << xml.name();
    xml.readNext();

    int vertices = 0;
    int faces = 0;

    // this object structure is not done
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "mesh")) {
        // go to the next child of object node
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "v") {
                ++vertices;
                xml.readNext();
                QString coords = xml.text().toString();
                QStringList stringlist = coords.split(" ");
                if (stringlist.size() < 3) {
                    continue;
                }

                float x1 = stringlist.at(0).toDouble()/5.0;
                float y1 = stringlist.at(1).toDouble()/5.0;
                float z1 = stringlist.at(2).toDouble()/5.0;

                QVector4D mesh_vertex(x1, y1, z1,  obj->getHVGXID());
                struct VertexData v;
                v.mesh_vertex = mesh_vertex;

                // todo: get the skeleton vertex from the skeleton itself using an index
                // get the point branch knots as well
                if (stringlist.size() < 5) {
                    // place holder
                    v.skeleton_vertex = mesh_vertex;
                } else {
                    // I could use index to be able to connect vertices logically
                    float x2 = stringlist.at(3).toDouble()/5.0;
                    float y2 = stringlist.at(4).toDouble()/5.0;
                    float z2 = stringlist.at(5).toDouble()/5.0;
                    QVector4D skeleton_vertex(x2, y2, z2, 0);
                    v.skeleton_vertex = skeleton_vertex;
                }
                m_mesh->addVertex(v);
            } else if (xml.name() == "f") {
                ++faces;
                xml.readNext();
                QString coords = xml.text().toString();
                QStringList stringlist = coords.split(" ");
                if (stringlist.size() < 3) {
                    continue;
                }

                int f1_index = stringlist.at(0).toInt()  + m_vertex_offset;
                int f2_index = stringlist.at(1).toInt()  + m_vertex_offset;
                int f3_index = stringlist.at(2).toInt()  + m_vertex_offset;

                if (! m_mesh->isValidFaces(f1_index, f2_index, f3_index) ) {
                    // error, break
                    qDebug() << "Error in obj file! " << obj->getName().data() << " " << obj->getHVGXID()
                             << " " << f1_index << " " << f2_index << " " << f3_index ;
                    delete obj;
                    break;
                }

                // add faces indices to object itself
                /* vertex 1 */
                obj->addTriangleIndex(f1_index - 1);
                /* vertex 2 */
                obj->addTriangleIndex(f2_index - 1);
                /* vertex 3 */
                obj->addTriangleIndex(f3_index - 1);
                m_indices_size += 3;

                // parse normals

            } else if (xml.name() == "markers") {
                qDebug() << xml.name();
                xml.readNext();
                QStringList markersList = xml.text().toString().split(" ");
                m_mesh->MarkBleedingVertices(markersList, m_vertex_offset);
            }
        } // if start element
        xml.readNext();
    } // while


    qDebug() << "vertices count: " << vertices << " faces: " << faces;
}

//
void DataContainer::parseSkeleton(QXmlStreamReader &xml, Object *obj)
{
    // read skeleton vertices and their edges
    // read vertices and their faces into the mesh
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() != "skeleton") {
        qDebug() << "Called XML parseObejct without attribs";
        return;
    }
    qDebug() << xml.name();

    xml.readNext();

    // this object structure is not done
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "skeleton")) {
        // go to the next child of object node
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "nodes") { // make it v
                qDebug() << xml.name();
                parseSkeletonNodes(xml, obj);
            } else if (xml.name() == "points") {
                qDebug() << xml.name();
                parseSkeletonPoints(xml, obj);
            } else if (xml.name() == "branches") { // children has only branches which uses their parents points
              // process branches
                parseBranch(xml, obj);
            }
        } // if start element
        xml.readNext();
    } // while
}

void DataContainer::parseSkeletonNodes(QXmlStreamReader &xml, Object *obj)
{
    // read vertices and their faces into the mesh
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() != "nodes") {
        qDebug() << "Called XML parseSkeletonNodes without attribs";
        return;
    }
    qDebug() << xml.name();

    xml.readNext();

    int nodes = 0;

    // this object structure is not done
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "nodes")) {
        // go to the next child of object node
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "v") {
                ++nodes;
                xml.readNext();
                QString coords = xml.text().toString();
                QStringList stringlist = coords.split(" ");
                if (stringlist.size() < 3) {
                    continue;
                }

                float x = stringlist.at(0).toDouble()/5.0;
                float y = stringlist.at(1).toDouble()/5.0;
                float z = stringlist.at(2).toDouble()/5.0;
                QVector3D node(x, y, z);
                obj->addSkeletonNode(node);
            }
        } // if start element
        xml.readNext();
    } // while


    qDebug() << "nodes count: " << nodes;
}

void DataContainer::parseSkeletonPoints(QXmlStreamReader &xml, Object *obj)
{
    // read vertices and their faces into the mesh
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() != "points") {
        qDebug() << "Called XML parseSkeletonNodes without attribs";
        return;
    }
    qDebug() << xml.name();


    xml.readNext();

    int nodes = 0;

    // this object structure is not done
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "points")) {
        // go to the next child of object node
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "v") {
                ++nodes;
                m_skeleton_points_size++;
                xml.readNext();
                QString coords = xml.text().toString();
                QStringList stringlist = coords.split(" ");
                if (stringlist.size() < 3) {
                    continue;
                }

                float x = stringlist.at(0).toDouble()/5.0;
                float y = stringlist.at(1).toDouble()/5.0;
                float z = stringlist.at(2).toDouble()/5.0;
                QVector3D node(x, y, z);
                obj->addSkeletonPoint(node);
            }
        } // if start element
        xml.readNext();
    } // while

    qDebug() << "nodes count: " << nodes;
}

void DataContainer::parseBranch(QXmlStreamReader &xml, Object *obj)
{
    // b -> one branch
    // knots
    // points
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() != "branches") {
        qDebug() << "Called XML parseBranch without attribs";
        return;
    }

    qDebug() << xml.name();
    xml.readNext();

    if (obj == NULL) {
        qDebug() << "Problem Obj is Null parseBranch.";
        return;
    }

    SkeletonBranch *branch = NULL;
    // this object structure is not done
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "branches")) {
        // go to the next child of object node
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "b") {
                branch = new SkeletonBranch();
                int ID = xml.attributes().value("name").toInt();
            } else if (xml.name() == "knots") { // for children knots uses parents points not nodes
                if (branch == NULL) {
                    qDebug() << "branch null";
                    continue;
                }
                xml.readNext();
                // two integers for the skeleton end nodes IDs
                QString coords = xml.text().toString();
                QStringList stringlist = coords.split(" ");
                if (stringlist.size() < 2) {
                    continue;
                }

                int knot1 = stringlist.at(0).toInt();
                int knot2 = stringlist.at(1).toInt();
                branch->addKnots(knot1, knot2);
            } else if (xml.name() == "points_ids") { // uses parent points, I need to mark these points with the child ID that uses them
                if (branch == NULL || obj == NULL) {
                    qDebug() << "branch null or obj is NULL";
                    continue;
                }
                xml.readNext();
                // list of integers for points IDs
                QString coords = xml.text().toString();
                QStringList stringlist = coords.split(" ");
                branch->addPointsIndxs(stringlist);
                obj->addSkeletonBranch(branch);
            }
        } // if start element
        xml.readNext();
    } // while
}

int DataContainer::getSkeletonPointsSize()
{
    return m_skeleton_points_size;
}

Mesh* DataContainer::getMeshPointer()
{
    return m_mesh;
}

int DataContainer::getMeshIndicesSize()
{
    return m_indices_size;
}

std::map<int, Object*>  DataContainer::getObjectsMap()
{
    return m_objects;
}

std::vector<QVector2D> DataContainer::getNeuritesEdges()
{
    return neurites_neurite_edge;
}

Object_t DataContainer::getObjectTypeByID(int hvgxID)
{
    Object* obj = m_objects[hvgxID]; // check if the ID is valid
    if (obj == NULL)
        return Object_t::UNKNOWN;

    return obj->getObjectType();
}

std::vector<int> DataContainer::getObjectsIDsByType(Object_t type)
{
  return m_objectsIDsByType[type];
}
