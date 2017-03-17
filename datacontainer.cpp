// skeletons to take care of : 134, 510
// 419, 56
// 422, 59
#include <chrono>
#include "datacontainer.h"
#include <set>

#define MESH_MAX 5.0f

/*
 * m_objects -> object class for each object (astrocyte, dendrite, ..)
 *           -> get from this the indices of the mesh
 *           -> get the skeleton vertices
 */
DataContainer::DataContainer()
{
    m_tempCounter = 0;
    m_indices_size = 0;
    m_skeleton_points_size = 0;

    max_volume = 1;
    max_astro_coverage = 0;

    m_limit = 10;
    m_vertex_offset = 0;
    m_mesh = new Mesh();


    /* 1 */
    loadConnectivityGraph(":/data/connectivityList.csv");// -> neurites_neurite_edge

    /* 2.1 */
    // importXML("://m3_astrocyte.xml");   // astrocyte  time:  79150.9 ms
    /* 2.2 */
    importXML("://m3_neurites.xml");    // neurites time:  28802 ms


    /* 3 */
    loadMetaDataHVGX(":/data/mouse3_metadata_objname_center_astroSyn.hvgx");


    /* 4 */
	qDebug() << "setting up octrees";
	m_boutonOctree.initialize(m_mesh->getVerticesListByType(Object_t::BOUTON));
	m_spineOctree.initialize(m_mesh->getVerticesListByType(Object_t::SPINE));
    m_glycogenOctree.initialize(&m_glycogenList);
	qDebug() << "octrees ready";

	//qDebug() << "testing clustering";
	//testing clustering
	//m_dbscan.initialize(&m_glycogenList, &m_glycogenMap, &m_glycogenOctree);
	//m_dbscan.run();
	//qDebug() << "done clustering";


    /* 5 */
    //  test vertex neighbors
    int v_index = 0;
    std::set< int > neighs2;
    m_mesh->getVertexNeighbors(v_index, neighs2);
    qDebug() << "neighbors of vertex " << v_index << ": ";
    for (auto iter = neighs2.begin(); iter != neighs2.end(); ++iter)
        qDebug() <<" " << *iter;
}

//----------------------------------------------------------------------------
//
DataContainer::~DataContainer()
{
    qDebug() << "~Mesh()";
    for (std::size_t i = 0; i != m_objects.size(); i++) {
        delete m_objects[i];
    }
}

//----------------------------------------------------------------------------
//
void* DataContainer::loadRawFile(QString path, int size)
{
    qDebug() << "Func: loadRawFile";

    QFile  file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open the file for reading";
        return false;
    }

    QDataStream in (&file);
    in.setVersion(5);
    char *buffer = new char[size];
    in.device()->reset();
    int nbytes = in.readRawData(buffer,size);
    qDebug() << nbytes;

    return (void *)buffer;
 }

//----------------------------------------------------------------------------
//
void DataContainer::loadConnectivityGraph(QString path)
{
    qDebug() << "Func: loadConnectivityGraph";
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

        if (wordList[0] == "Id") {
            continue;
        } else {
            if (wordList.size() < 3) {
                qDebug() << "wordList.size() < 3";
                return;
            }
            int ID = wordList[0].toInt();
            int nodeID1 = wordList[1].toInt();
            int nodeID2 = wordList[2].toInt();

            QVector2D edge_info = QVector2D(nodeID1, nodeID2);
            neurites_neurite_edge.push_back(edge_info);
        }
    }

    file.close();
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
	int glycogenCount = 0;
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        wordList = line.split(',');

		
        if (wordList[0] == "gc") {
            if (wordList.size() < 7) {
                qDebug() << "wordList.size() < 7";
                return;
            }
            int ID = wordList[1].toInt();
            float x = wordList[2].toFloat();
            float y = wordList[3].toFloat();
            float z = wordList[4].toFloat();
            float diameter = wordList[5].toFloat();
            std::string name = wordList[6];

            Glycogen *gc = new Glycogen(ID, name, QVector3D(x, y, z), diameter);
			gc->setIndex(glycogenCount);
			glycogenCount++;
            m_glycogenMap[ID] = gc;
			m_glycogenList.push_back(gc->getVertexData());


        } else if (wordList[0] == "sg") {
            // update the nodes center here?
            // get the point from neurite to astrocyte skeleton
            // update: m_closest_astro_vertex.first -> astro skeleton point ID
            // or get theat point from astrocyte and mark it with the object ID

            // just to debug the value and see if it is on astrocyte skeleton
            // use center as this value

            int hvgxID = wordList[1].toInt();
            if (m_objects.find(hvgxID) == m_objects.end()) {
                continue;
            }

            int pID = wordList[18].toInt();
            float x = wordList[19].toFloat();
            float y = wordList[20].toFloat();
            float z = wordList[21].toFloat();

            m_objects[hvgxID]->setCenter(QVector4D(x/MESH_MAX, y/MESH_MAX, z/MESH_MAX, 0));

            // volume
            int volume = wordList[25].toInt();

            // skeleton center
            float center_x = wordList[7].toFloat();
            float center_y = wordList[8].toFloat();
            float center_z = wordList[9].toFloat();

        } else if (wordList[0] == "sy") {
            // add this info to the related objects
            // id, vol_node_id, abs_edge_id, axon_id, dendrite_id, spine_id, bouton_id, name
            int hvgxID = wordList[1].toInt();
            if (m_objects.find(hvgxID) == m_objects.end()) {
                continue;
            }

            int axon_id = wordList[4].toInt();
            int dendrite_id = wordList[5].toInt();
            int spine_id = wordList[6].toInt();
            int bouton_id = wordList[7].toInt();

            Object *synapse = m_objects[hvgxID];
            synapse->UpdateSynapseData(axon_id, dendrite_id, spine_id, bouton_id);

            if (axon_id && m_objects.find(axon_id) != m_objects.end()) {
                m_objects[axon_id]->addSynapse(synapse);
            }

            if (dendrite_id && m_objects.find(dendrite_id) != m_objects.end()) {
                m_objects[dendrite_id]->addSynapse(synapse);
            }

            if (spine_id && m_objects.find(spine_id) != m_objects.end()) {
                m_objects[spine_id]->addSynapse(synapse);
            }

            if (bouton_id && m_objects.find(bouton_id) != m_objects.end()) {
                m_objects[bouton_id]->addSynapse(synapse);
            }

        } else if (wordList[0] == "mt") {
            // update mitochoneria parent here if any exists
            //"mt,1053,307,DENDRITE,144,mito_d048_01_029\n"
            int hvgxID = wordList[1].toInt();
            int parentID = wordList[4].toInt();
            if (m_objects.find(hvgxID) == m_objects.end()) {
                continue;
            }

            if (m_objects.find(parentID) == m_objects.end()) {
                continue;
            }

            m_objects[hvgxID]->setParentID(m_objects[parentID]);
            m_objects[parentID]->addChild(m_objects[hvgxID]);

        } else if (wordList[0] == "bo") {
            // id, vesicleNo, volume, surfaceArea, axon_id, name, is_terminal_branch, is_mitochondrion
            int hvgxID = wordList[1].toInt();
            if (m_objects.find(hvgxID) == m_objects.end()) {
                continue;
            }

            Object *parent = m_objects[hvgxID]->getParent();
            if (parent == NULL) {
                continue;
            }

            int function = parent->getFunction();
            m_objects[hvgxID]->setFunction(function);

        } else if (wordList[0] == "sp") {
            // id, psd_area, volume, dendrite_id, does_form_synapse, with_apparatus, has_glia_nearby, spine name
            int hvgxID = wordList[1].toInt();
            if (m_objects.find(hvgxID) == m_objects.end()) {
                continue;
            }

            Object *parent = m_objects[hvgxID]->getParent();
            if (parent == NULL) {
                continue;
            }

            int function = parent->getFunction();
            m_objects[hvgxID]->setFunction(function);

        } else if (wordList[0] == "dn") {
            // id, function (0:ex,1:in), abs_node_id, name
            int hvgxID = wordList[1].toInt();
            if (m_objects.find(hvgxID) == m_objects.end()) {
                continue;
            }

            int function = wordList[2].toInt();
            m_objects[hvgxID]->setFunction(function);

        } else if (wordList[0] == "ax") {
            // id, function (0:ex,1:in), is_mylenated, abs_node_id, name
            int hvgxID = wordList[1].toInt();
            if (m_objects.find(hvgxID) == m_objects.end()) {
                continue;
            }

            int function = wordList[2].toInt();
            m_objects[hvgxID]->setFunction(function);
        }


    }

    file.close();
}

//----------------------------------------------------------------------------
//
// need a way to optimize this!!!!!
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
            }
        }
    }

    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms = t2 - t1;
    qDebug() << "time: " << ms.count() << "ms, m_tempCounter: " << m_tempCounter;
}

//----------------------------------------------------------------------------
//
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
                obj->setCenter(QVector4D(x/MESH_MAX, y/MESH_MAX, z/MESH_MAX, 0));
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
                obj->setAstPoint(QVector4D(x/MESH_MAX, y/MESH_MAX, z/MESH_MAX, 0));
                // set ssbo with this center
            }
        } // if start element
        xml.readNext();
    } // while

    if (obj != NULL) {
        if (hvgxID == 120)
            return;

         m_objects[hvgxID] =  obj;
         std::vector<Object*> objects_list = m_objectsByType[obj->getObjectType()];
         objects_list.push_back(obj);
         m_objectsByType[obj->getObjectType()] = objects_list;

         // need to update these info whenever we filter or change the threshold
         if (obj->getObjectType() == Object_t::ASTROCYTE)
             return;

         if (max_astro_coverage < obj->getAstroCoverage())
            max_astro_coverage = obj->getAstroCoverage();

         if (max_volume < obj->getVolume())
             max_volume = obj->getVolume();
    }

}

//----------------------------------------------------------------------------
//
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

                float x1 = stringlist.at(0).toDouble()/MESH_MAX;
                float y1 = stringlist.at(1).toDouble()/MESH_MAX;
                float z1 = stringlist.at(2).toDouble()/MESH_MAX;

                QVector4D mesh_vertex(x1, y1, z1,  obj->getHVGXID());
                struct VertexData v;
				v.isGlycogen = false;
                v.mesh_vertex = mesh_vertex;

                // todo: get the skeleton vertex from the skeleton itself using an index
                // get the point branch knots as well
                if (stringlist.size() < 5) {
                    // place holder
                    qDebug() << "Problem!";
                    v.skeleton_vertex = mesh_vertex;
                } else if ( obj->getObjectType() == Object_t::MITO || obj->getObjectType() == Object_t::SYNAPSE ) {
                    QVector4D center = obj->getCenter();
                    center.setW(0);
                    v.skeleton_vertex = center;
                } else {
                    // I could use index to be able to connect vertices logically
                    float x2 = stringlist.at(3).toFloat()/MESH_MAX;
                    float y2 = stringlist.at(4).toFloat()/MESH_MAX;
                    float z2 = stringlist.at(5).toFloat()/MESH_MAX;
                    QVector4D skeleton_vertex(x2, y2, z2, 0);
                    v.skeleton_vertex = skeleton_vertex;
                }

                float VertexToAstroDist = 100;
                if (stringlist.size() > 6) {
                    VertexToAstroDist = stringlist.at(6).toFloat();
                  //  qDebug() << "### VertexToAstroDist: " << VertexToAstroDist;
                }

                v.skeleton_vertex.setW(VertexToAstroDist); // distance from neurite to astrocyte
                // find the minimum distance and store it in the object so we can easily decide if
                // it touches the astrocyte or not

                int vertexIdx = m_mesh->addVertex(v, obj->getObjectType());
                obj->updateClosestAstroVertex(VertexToAstroDist, vertexIdx);
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

                int t_index1 = f1_index - 1;
                int t_index2 = f2_index - 1;
                int t_index3 = f3_index - 1;

                // add faces indices to object itself
                /* vertex 1 */
                obj->addTriangleIndex(t_index1);
                /* vertex 2 */
                obj->addTriangleIndex(t_index2);
                /* vertex 3 */
                obj->addTriangleIndex(t_index3);

                m_mesh->addFace(t_index1, t_index2, t_index3);

                m_indices_size += 3;

                if (m_indices_size_byType.find(obj->getObjectType()) == m_indices_size_byType.end() ) {
                    m_indices_size_byType[obj->getObjectType()] = 0;
                }

                m_indices_size_byType[obj->getObjectType()] += 3;
                // parse normals

            }
        } // if start element
        xml.readNext();
    } // while


    qDebug() << "vertices count: " << vertices << " faces: " << faces;
}

//----------------------------------------------------------------------------
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
    if ( obj->getObjectType() == Object_t::MITO || obj->getObjectType() == Object_t::SYNAPSE )
        return;

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

//----------------------------------------------------------------------------
//
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

                float x = stringlist.at(0).toDouble()/MESH_MAX;
                float y = stringlist.at(1).toDouble()/MESH_MAX;
                float z = stringlist.at(2).toDouble()/MESH_MAX;
                QVector3D node(x, y, z);
                obj->addSkeletonNode(node);
            }
        } // if start element
        xml.readNext();
    } // while


    qDebug() << "nodes count: " << nodes;
}

//----------------------------------------------------------------------------
//
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

                // children? they should not have points
                float x = stringlist.at(0).toDouble()/MESH_MAX;
                float y = stringlist.at(1).toDouble()/MESH_MAX;
                float z = stringlist.at(2).toDouble()/MESH_MAX;
                QVector3D node(x, y, z);
                obj->addSkeletonPoint(node);
            }
        } // if start element
        xml.readNext();
    } // while

    qDebug() << "nodes count: " << nodes;
}

//----------------------------------------------------------------------------
//
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

//----------------------------------------------------------------------------
//
int DataContainer::getSkeletonPointsSize()
{
    return m_skeleton_points_size;
}

//----------------------------------------------------------------------------
//
Mesh* DataContainer::getMeshPointer()
{
    return m_mesh;
}

//----------------------------------------------------------------------------
//
int DataContainer::getMeshIndicesSize()
{
    return m_indices_size;
}

//----------------------------------------------------------------------------
//
std::map<int, Object*>  DataContainer::getObjectsMap()
{
    return m_objects;
}

//----------------------------------------------------------------------------
//
std::vector<QVector2D> DataContainer::getNeuritesEdges()
{
    return neurites_neurite_edge;
}

//----------------------------------------------------------------------------
//
Object_t DataContainer::getObjectTypeByID(int hvgxID)
{
    Object* obj = m_objects[hvgxID]; // check if the ID is valid
    if (obj == NULL)
        return Object_t::UNKNOWN;

    return obj->getObjectType();
}

//----------------------------------------------------------------------------
//
std::vector<Object*> DataContainer::getObjectsByType(Object_t type)
{
  return m_objectsByType[type];
}

 std::string DataContainer::getObjectName(int hvgxID)
 {
     if (m_objects.find(hvgxID) == m_objects.end())
         return "Unknown";

     return m_objects[hvgxID]->getName();
 }

void DataContainer::recomputeMaxVolAstro()
{
    float temp_max_astro_coverage = 0;
    int temp_max_volume = 1;
    for ( auto iter = m_objects.begin(); iter != m_objects.end(); iter++ ) {
        Object *obj = (*iter).second;
        if (obj->isFiltered() || obj->getObjectType() == Object_t::ASTROCYTE)
            continue;
        // need to update these info whenever we filter or change the threshold
        if (temp_max_astro_coverage < obj->getAstroCoverage())
           temp_max_astro_coverage = obj->getAstroCoverage();

        if (temp_max_volume < obj->getVolume())
            temp_max_volume = obj->getVolume();

    }

    max_volume = temp_max_volume;
    max_astro_coverage = temp_max_astro_coverage;
}
