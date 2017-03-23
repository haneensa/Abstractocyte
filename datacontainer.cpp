// skeletons to take care of : 134, 510
// 419, 56
// 422, 59
#include <chrono>
#include "datacontainer.h"
#include <set>
#include <QDomDocument>

/*
 * m_objects -> object class for each object (astrocyte, dendrite, ..)
 *           -> get from this the indices of the mesh
 *           -> get the skeleton vertices
 */
DataContainer::DataContainer()
    :   m_indices_size(0),
        m_skeleton_points_size(0),
        max_volume(2),
        max_astro_coverage(0),
        max_synapse_volume(0),
        m_limit(1),
        m_vertex_offset(0),
        m_faces_offset(0),
        m_debug_msg(false)
{

    m_mesh = new Mesh();
	m_glycogen3DGrid.setSize(DIM_G, DIM_G, DIM_G);
	m_boutonHash.setSize(32, 32, 32);
	m_spineHash.setSize(32, 32, 32);
    /* 1: load all data */
    loadData();

    /* 2: build missing skeletons due to order of objects in file */
    buildMissingSkeletons();


    /* 3 */
    qDebug() << "setting up octrees";
    //m_boutonOctree.initialize(m_mesh->getVerticesListByType(Object_t::BOUTON));
    //m_spineOctree.initialize(m_mesh->getVerticesListByType(Object_t::SPINE));
    m_glycogenOctree.initialize(&m_glycogenList);
    qDebug() << "octrees ready";

    //qDebug() << "testing clustering";
    //testing clustering
    //m_dbscan.initialize(&m_glycogenList, &m_glycogenMap, &m_glycogenOctree);
    //m_dbscan.run();
    //qDebug() << "done clustering";

    this->recomputeMaxValues();
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
// *** To write binary files:
// run 1)  m_loadType = LoadFile_t::DUMP_ASTRO;
// run 2)  m_loadType = LoadFile_t::DUMP_NEURITES;
// *** To read from binary files:
// m_loadType = LoadFile_t::LOAD_MESH_NO_VERTEX;
// *** To read from normal xml:
// m_loadType = LoadFile_t::LOAD_MESH_W_VERTEX;
void DataContainer::loadData()
{
    /* 1: no need for this, I can later construct this from hvgx file */
    loadConnectivityGraph(":/data/connectivityList.csv");// -> neurites_neurite_edge
    QString hvgxFile = ":/data/mouse3_metadata_objname_center_astroSyn.hvgx";
    loadParentFromHVGX(hvgxFile);


    m_limit = 10;
    m_loadType = LoadFile_t::LOAD_MESH_NO_VERTEX;
    m_load_data = LoadData_t::NEURITES;
    m_normals_t = Normals_t::LOAD_NORMAL;


    auto t1 = std::chrono::high_resolution_clock::now();

    if (m_loadType == LoadFile_t::DUMP_ASTRO || m_loadType == LoadFile_t::DUMP_NEURITES) {
        if (m_loadType == LoadFile_t::DUMP_ASTRO) {
            importXML("://pipeline_scripts/output/m3_astrocyte.xml");   // 155,266  ms ~ 2.6 min
            const char* filename1 = "astro_data_fv.dat";
            m_mesh->dumpVertexData(filename1);
        } else {
            const char* filename2 = "neurites_data_fv.dat";
            importXML("://pipeline_scripts/output/m3_neurites.xml");    // 910741
            m_mesh->dumpVertexData(filename2);
        }
    } else if (m_loadType == LoadFile_t::LOAD_MESH_NO_VERTEX)  {

        if (m_load_data == LoadData_t::ASTRO) {
            const char* filename1 = "astro_data_fv.dat";
            m_mesh->readVertexData(filename1);
            importXML("://pipeline_scripts/output/m3_astrocyte_noVertex.xml");   //  110,928  ms ~ 2 min -> 17306
        } else if (m_load_data == LoadData_t::NEURITES) {
            const char* filename2 = "neurites_data_fv.dat";
            m_mesh->readVertexData(filename2); //
            importXML("://pipeline_scripts/output/m3_neurites_noVertexNoFace.xml");    // 674046 ~ 12 min -> 118107 ms -> 134884 with vertex type classification
        } else if (m_load_data == LoadData_t::ALL) {
            const char* filename1 = "astro_data_fv.dat";
            m_mesh->readVertexData(filename1);
            importXML("://pipeline_scripts/output/m3_astrocyte_noVertex.xml");   //   110,928  ms ~ 2 min -> 17306 ms

            const char* filename2 = "neurites_data_fv.dat";
            m_mesh->readVertexData(filename2); //
            importXML("://pipeline_scripts/output/m3_neurites_noVertexNoFace.xml");    // 674046 ~ 12 min -> 118107 ms
        }

    } else {
        if (m_load_data == LoadData_t::ASTRO) {
            importXML("://pipeline_scripts/output/m3_astrocyte.xml");   // 155,266  ms ~ 2.6 min
        } else if (m_load_data == LoadData_t::NEURITES) {
            importXML("://pipeline_scripts/output/m3_neurites.xml");    // 910741
        } else if (m_load_data == LoadData_t::ALL) {
            importXML("://pipeline_scripts/output/m3_astrocyte.xml");   // 155,266  ms ~ 2.6 min
            importXML("://pipeline_scripts/output/m3_neurites.xml");    // 910741
        }

    }

    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms = t2 - t1;

    qDebug() << "Objects Loading time: " << ms.count();



     if (m_normals_t == Normals_t::DUMP_NORMAL) {
        t1 = std::chrono::high_resolution_clock::now();
        if (m_load_data == LoadData_t::ASTRO) {
            m_mesh->computeNormalsPerVertex(); // Normals Computing time:  9440.12
            m_mesh->dumpNormalsList("astro_normals.dat");
        } else if (m_load_data == LoadData_t::NEURITES) {
            m_mesh->computeNormalsPerVertex(); // Normals Computing time:  44305.1
            m_mesh->dumpNormalsList("neurites_normals.dat");
        }
        t2 = std::chrono::high_resolution_clock::now();
        ms = t2 - t1;

        qDebug() << "Normals Computing time: " << ms.count();

    } else if (m_normals_t == Normals_t::LOAD_NORMAL) {
        if (m_load_data == LoadData_t::ASTRO) {
            m_mesh->readNormalsBinary("astro_normals.dat");
        } else if (m_load_data == LoadData_t::NEURITES) {
            m_mesh->readNormalsBinary("neurites_normals.dat");
        } else if (m_load_data == LoadData_t::ALL) {
            m_mesh->readNormalsBinary("astro_normals.dat");
            m_mesh->readNormalsBinary("neurites_normals.dat");
        }
    }

     qDebug() << " faces: "  << m_mesh->getFacesListSize() << " " <<
                 " vertices: "  << m_mesh->getVerticesSize() << " " <<
                 " normals: "  << m_mesh->getNormalsListSize();




    /* 3 */
    loadMetaDataHVGX(hvgxFile);
}


//----------------------------------------------------------------------------
//
char* DataContainer::loadRawFile(QString path, int size)
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

    return  buffer;
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

void DataContainer::loadParentFromHVGX(QString path)
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


        if (wordList[0] == "sg") {
            // vast_id, flags, r,g,b, pattern1, r2, g2, b2, pattern2, anchorx, anchory, anchorz, parent_id, child_id, previous_sibiling_id, next_sibiling_id, collapsed, bboxx1, bboxy1, bboxz1, bboxx2, bboxy2, bboxz2, voxels, type, object_id, name

            if (wordList[26] == "MITOCHONDERIA") {
                qDebug() << line;
                continue;
            }

            int hvgxID = wordList[1].toInt();
            int parentID = wordList[14].toInt();

            m_parents[hvgxID] = parentID;

        } else if (wordList[0] == "mt") {
            // update mitochoneria parent here if any exists
            //"mt,1053,307,DENDRITE,144,mito_d048_01_029\n"
            int hvgxID = wordList[1].toInt();
            int parentID = wordList[4].toInt();
            m_parents[hvgxID] = parentID;
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

            m_glycogen3DGrid.addNormalizedPoint(gc->x(), gc->y(), gc->z(), gc->getVolume());


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

            m_objects[hvgxID]->setParentID(parentID);
            m_objects[parentID]->addChild(m_objects[hvgxID]);

        } else if (wordList[0] == "bo") {
            // id, vesicleNo, volume, surfaceArea, axon_id, name, is_terminal_branch, is_mitochondrion
            int hvgxID = wordList[1].toInt();
            if (m_objects.find(hvgxID) == m_objects.end()) {
                continue;
            }

            int parentID = m_objects[hvgxID]->getParentID();
            if (m_objects.find(parentID) == m_objects.end()) {
                continue;
            }

            Object *parent = m_objects[parentID];
            int function = parent->getFunction();
            m_objects[hvgxID]->setFunction(function);

        } else if (wordList[0] == "sp") {
            // id, psd_area, volume, dendrite_id, does_form_synapse, with_apparatus, has_glia_nearby, spine name
            int hvgxID = wordList[1].toInt();
            if (m_objects.find(hvgxID) == m_objects.end()) {
                continue;
            }

            int parentID = m_objects[hvgxID]->getParentID();
            if (m_objects.find(parentID) == m_objects.end()) {
                continue;
            }

            Object *parent = m_objects[parentID];
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
int DataContainer::importXML(QString path)
{
    qDebug() << "Func: importXML";
    auto t1 = std::chrono::high_resolution_clock::now();

    QFile  *file = new QFile(path);
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Could not open the file for reading";
        return 0;
    }


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
                parseObject(xml, obj);
            }
        }
    }

    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms = t2 - t1;

    qDebug() << "importXML time: " << ms.count();

    m_faces_offset += m_mesh->getFacesListSize();
    m_vertex_offset += m_mesh->getVerticesSize();

    return 0;
}


//----------------------------------------------------------------------------
// Experimental
/*bool DataContainer::importXML_DOM(QString path)
{
    qDebug() << "Func: importXML";
    auto t1 = std::chrono::high_resolution_clock::now();

    QFile  *file = new QFile(path);
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Could not open the file for reading";
        return false;
    }

    QDomDocument doc;
    if (!doc.setContent(file)) {
        file->close();
        return 0;
    }
    file->close();

    m_vertex_offset += m_mesh->getVerticesSize();
    QDomNode n = doc.firstChild();
    while (!n.isNull()) {
        //QXmlStreamReader::TokenType token = xml.readNext();
        //if (token == QXmlStreamReader::StartDocument) {
        //	continue;
        //}

        /*if (token == QXmlStreamReader::StartElement) {
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
        n = n.firstChild();
    }

    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms = t2 - t1;
    qDebug() << "time: " << ms.count() << "ms, m_tempCounter: " << m_tempCounter;
}*/



//----------------------------------------------------------------------------
//
// load the object with all its related informations
void DataContainer::parseObject(QXmlStreamReader &xml, Object *obj)
{
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() != "o") {
        qDebug() << "Called XML parseObejct without attribs";
        return;
    }

    if (m_debug_msg)
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
    // make parent IDs not pointers to objects
    if (m_parents.find(hvgxID) != m_parents.end()) {
        int parentID = m_parents[hvgxID];
        obj->setParentID(parentID);

        if (m_objects.find(parentID) != m_objects.end()) {
            m_objects[parentID]->addChild(obj);
        } else
            qDebug() << "Object has no parents in m_objects yet " << parentID;
    }

    QVector4D color = obj->getColor();
    // set ssbo with this ID to this color

    xml.readNext();


    // this object structure is not done
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "o")) {
        // go to the next child of object node
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "mesh") {
                auto t1 = std::chrono::high_resolution_clock::now();

                if (m_loadType == LoadFile_t::LOAD_MESH_NO_VERTEX) { //130823
                    parseMeshNoVertexnoFace(xml, obj); // takes the most time
                } else {
                    parseMesh(xml, obj); // 175008
                }
                auto t2 = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> ms = t2 - t1;
                if (m_debug_msg)
                    qDebug() << "time: " << ms.count() << "ms ";
            } else if (xml.name() == "skeleton") {
                parseSkeleton(xml, obj);
            } else if (xml.name() == "volume") {
                xml.readNext();
                int volume =  xml.text().toInt();
                obj->setVolume(volume);
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
                obj->setAstPoint(QVector4D(x/MESH_MAX, y/MESH_MAX, z/MESH_MAX, 0));
                // set ssbo with this center
            }
        } // if start element
        xml.readNext();
    } // while

    if (obj != NULL) {
        if (hvgxID == 120) // cant skip because it has mito 916 as well (skip them both if we didnt fix 120
            return;

         m_objects[hvgxID] =  obj;
         std::vector<Object*> objects_list = m_objectsByType[obj->getObjectType()];
         objects_list.push_back(obj);
         m_objectsByType[obj->getObjectType()] = objects_list;

         if (max_volume < obj->getVolume())
             max_volume = obj->getVolume();

         max_synapse_volume = max_volume;

         // need to update these info whenever we filter or change the threshold
         if (obj->getObjectType() == Object_t::ASTROCYTE)
             return;

         if (obj->getObjectType() == Object_t::MITO) {
             // check its parent, if astrocyte it wont be fair to include it
             int parentID =obj->getParentID();
             if (m_objects.find(parentID) != m_objects.end()) {
                 Object *parent = m_objects[parentID];
                 if (parent->getObjectType()  == Object_t::ASTROCYTE  )
                     return;
             }
         }


         if (max_astro_coverage < obj->getAstroCoverage())
            max_astro_coverage = obj->getAstroCoverage();


    }

}


//----------------------------------------------------------------------------
//
void DataContainer::parseMeshNoVertexnoFace(QXmlStreamReader &xml, Object *obj)
{
    // read vertices and their faces into the mesh
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() != "mesh") {
        qDebug() << "Called XML parseObejct without attribs";
        return;
    }

    if (m_debug_msg)
        qDebug() << xml.name();

    xml.readNext();

    // this object structure is not done
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "mesh")) {
        // go to the next child of object node
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "vc") {
                xml.readNext();
                QString coords = xml.text().toString();
                QStringList stringlist = coords.split(" ");
                if (stringlist.size() < 2) {
                    continue;
                }

                int vertexIdx = stringlist.at(0).toInt() + m_vertex_offset;
                int vertexCount = stringlist.at(1).toInt();
                // mark vertices in vertex list with this range as they belong to the object type

                std::vector< struct VertexData >* meshVertexList = m_mesh->getVerticesList();

                for (int i = vertexIdx; i < (vertexIdx + vertexCount); ++i ) {
                    struct VertexData* mesh_vertex = &meshVertexList->at(i);
                    mesh_vertex->index = i;
                    float VertexToAstroDist = mesh_vertex->skeleton_vertex.w();
                    obj->updateClosestAstroVertex(VertexToAstroDist, i);
                    m_mesh->addVertex(mesh_vertex, obj->getObjectType());
					if (obj->getObjectType() == Object_t::BOUTON)
						m_boutonHash.addNormalizedPoint(mesh_vertex->x(), mesh_vertex->y(), mesh_vertex->z(), mesh_vertex);
					if (obj->getObjectType() == Object_t::SPINE)
						m_spineHash.addNormalizedPoint(mesh_vertex->x(), mesh_vertex->y(), mesh_vertex->z(), mesh_vertex);
                }

            } else if (xml.name() == "fc") {
                xml.readNext();
                QString coords = xml.text().toString();
                QStringList stringlist = coords.split(" ");
                if (stringlist.size() < 2) {
                    continue;
                }

                int faceIdx = stringlist.at(0).toInt() + m_faces_offset;
                int faceCount = stringlist.at(1).toInt();

                std::vector< struct face > * facesList = m_mesh->getFacesList();
                // process faces that start from index "faceIdx" with count as faceCount here

                for (int i = faceIdx; i < (faceCount+faceIdx); ++i) {
                     struct face f = facesList->at(i);
                     obj->addTriangleIndex(f.v[0] + m_vertex_offset ); // do I have to add the offset?
                     obj->addTriangleIndex(f.v[1] + m_vertex_offset  );
                     obj->addTriangleIndex(f.v[2] + m_vertex_offset  );

                     m_mesh->addVertexNeighbor(f.v[0] + m_vertex_offset, i);
                     m_mesh->addVertexNeighbor(f.v[1] + m_vertex_offset, i);
                     m_mesh->addVertexNeighbor(f.v[2] + m_vertex_offset, i);
                }

                m_indices_size += (faceCount * 3);

                if (m_indices_size_byType.find(obj->getObjectType()) == m_indices_size_byType.end() ) {
                    m_indices_size_byType[obj->getObjectType()] = 0;
                }

                m_indices_size_byType[obj->getObjectType()] += (faceCount * 3);

                // update synapse center
                if (obj->getObjectType() == Object_t::SYNAPSE) {
                    // get any vertex that belong to the synapse
                    std::vector< struct VertexData >* vertixList = m_mesh->getVerticesList();
                    struct face f = facesList->at(faceIdx);
                    struct VertexData vertex = vertixList->at( f.v[0] + m_vertex_offset );
                    obj->setCenter(QVector4D(vertex.mesh_vertex.x(),
                                                           vertex.mesh_vertex.y(),
                                                           vertex.mesh_vertex.z(), 0));

                }

            }
        } // if start element
        xml.readNext();
    } // while

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

    if (m_debug_msg)
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
                std::vector< struct VertexData >* meshVertexList = m_mesh->getVerticesList();

                meshVertexList->emplace_back();
                int vertexIdx = meshVertexList->size() - 1;
                struct VertexData* v = &meshVertexList->at(vertexIdx);
                v->index = vertexIdx;
                //struct VertexData v;
                //v.isGlycogen = false;
                v->mesh_vertex = mesh_vertex;

                bool mito_no_parent = false;
                if (obj->getObjectType() == Object_t::MITO) {
                    // check if it doesnt have any skeletons
                    if (obj->getParentID() == 0) {
                       qDebug() << obj->getName().data(); // mitoa1532b101113, mitoaxxxmyel126, mitoaxxx01154, mitoxxx01001
                        mito_no_parent = true;
                    }
                }

                // todo: get the skeleton vertex from the skeleton itself using an index
                // get the point branch knots as well
                if (stringlist.size() < 5 || obj->getObjectType() == Object_t::SYNAPSE || mito_no_parent) {
                    // place holder
                    v->skeleton_vertex = mesh_vertex;
                } else {
                    // I could use index to be able to connect vertices logically
                    float x2 = stringlist.at(3).toFloat()/MESH_MAX;
                    float y2 = stringlist.at(4).toFloat()/MESH_MAX;
                    float z2 = stringlist.at(5).toFloat()/MESH_MAX;
                    QVector4D skeleton_vertex(x2, y2, z2, 0);
                    v->skeleton_vertex = skeleton_vertex;
                }

                float VertexToAstroDist = 100;
                if (stringlist.size() > 6) {
                    VertexToAstroDist = stringlist.at(6).toFloat();
                  //  () << "### VertexToAstroDist: " << VertexToAstroDist;
                }

                v->skeleton_vertex.setW(VertexToAstroDist); // distance from neurite to astrocyte
                // find the minimum distance and store it in the object so we can easily decide if
                // it touches the astrocyte or not

                vertexIdx = m_mesh->addVertex(v, obj->getObjectType());
                obj->updateClosestAstroVertex(VertexToAstroDist, vertexIdx); // make local function that goes through all vertices of this object (store unique indices) and compute this
             } else if (xml.name() == "vn") {
                // add normal to mesh
                xml.readNext();
                QString coords = xml.text().toString();
                QStringList stringlist = coords.split(" ");
                if (stringlist.size() < 3) {
                    continue;
                }

                float x = stringlist.at(0).toDouble();
                float y = stringlist.at(1).toDouble();
                float z = stringlist.at(2).toDouble();
                m_mesh->addVertexNormal(QVector4D(x, y, z, 0));

            } else if (xml.name() == "f") { // f v1/vt1/vn1 v2/vt2/vn2/ v3/vt3/vn3
                ++faces;
                xml.readNext();
                int vertexIdx[3];

                QString coords = xml.text().toString();
                QStringList stringlist = coords.split(" ");
                if (stringlist.size() < 3) {
                    continue;
                }

                for (int i = 0; i < 3; ++i) {
                    // I dont need normal index because its the same as vertex index
                    QStringList face = stringlist.at(i).split("/");
                    vertexIdx[i] = face.at(0).toInt()  + m_vertex_offset;

                }

                if (! m_mesh->isValidFaces(vertexIdx[0], vertexIdx[1], vertexIdx[2]) ) {
                    // error, break
                    qDebug() << "Error in obj file! " << obj->getName().data() << " " << obj->getHVGXID()
                             << " " << vertexIdx[0] << " " << vertexIdx[1] << " " << vertexIdx[2] ;
                    delete obj;
                    break;
                }

                int t_index[3];
                for (int i = 0; i < 3; ++i) {
                    t_index[i] =  vertexIdx[i] - 1;
                    // add faces indices to object itself
                    obj->addTriangleIndex(t_index[i]);
                }

                m_mesh->addFace(t_index[0], t_index[1], t_index[2]);

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

    if (m_debug_msg)
        qDebug() << xml.name();

    xml.readNext();

    if ( obj->getObjectType() == Object_t::SYNAPSE )
        return;

    // this object structure is not done
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "skeleton")) {
        // go to the next child of object node
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "nodes") { // make it v
                parseSkeletonNodes(xml, obj);
            } else if (xml.name() == "points") {
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

    if (m_debug_msg)
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


    if (m_debug_msg)
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

    if (m_debug_msg)
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

    if (m_debug_msg)
        qDebug() << "points count: " << nodes;
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

    if (m_debug_msg)
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
                int parentID = obj->getParentID();
                Object *parent = NULL;
                if (m_objects.find(parentID) != m_objects.end()) {
                    parent = m_objects[parentID];
                }

                if (parent != NULL && parent->hasParent()) {
                    int parentParentID = parent->getParentID();
                    if (m_objects.find(parentParentID) == m_objects.end()) {
                        qDebug() << "No parentParentID Again!!! " << parentParentID << " " << obj->getHVGXID();
                    } else {
                        parent = m_objects[parentParentID];
                    }
                }

                // pass parent here if exists else null
                bool branch_added = obj->addSkeletonBranch(branch, parent);
                if (branch_added == false) {
                    // add this to the list that would be processed later again
                    m_missingParentSkeleton.insert(obj);
                }
            }
        } // if start element
        xml.readNext();
    } // while
}

//----------------------------------------------------------------------------
//
void DataContainer::buildMissingSkeletons()
{
    int k = 0;

    // check the missing skeletons due to missing parents
    for (auto iter = m_missingParentSkeleton.begin(); iter != m_missingParentSkeleton.end(); ++iter) { // 135
        Object * obj = *iter;
        // get object parent
        int parentID = obj->getParentID();
        if (m_objects.find(parentID) == m_objects.end()) {
            qDebug() << "No Parent Again!!! " << parentID << " " << obj->getHVGXID();
            continue;
        }

        m_objects[parentID]->addChild(obj);

        Object *parent = m_objects[parentID];

        if (parent->hasParent()) {
            int parentParentID = parent->getParentID();
            if (m_objects.find(parentParentID) == m_objects.end()) {
                qDebug() << "No parentParentID Again!!! " << parentParentID << " " << obj->getHVGXID();
                continue;
            }

            parent = m_objects[parentParentID];
            m_objects[parentParentID]->addChild(obj);// and if parent has parent add it to the parent as well
        }
        if (m_debug_msg)
            qDebug() <<  k++ << " " << obj->getHVGXID() << " " << obj->getName().data();
        obj->fixSkeleton(parent);
    }

    m_missingParentSkeleton.clear();
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
unsigned char* DataContainer::getGlycogen3DGridData()
{
        return m_glycogen3DGrid.getData8Bit();
}

void DataContainer::resetMappingValues()
{
	for (auto iter = m_objects.begin(); iter != m_objects.end(); iter++)
	{
		iter->second->setMappedValue(0);
	}
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

//----------------------------------------------------------------------------
//
 std::string DataContainer::getObjectName(int hvgxID)
 {
     if (m_objects.find(hvgxID) == m_objects.end())
         return "Unknown";

     return m_objects[hvgxID]->getName();
 }

 //----------------------------------------------------------------------------
 //
 Object* DataContainer::getObject(int hvgxID)
 {
	 if (m_objects.find(hvgxID) == m_objects.end())
		 return 0;
	 else return m_objects.at(hvgxID);
 }

 //----------------------------------------------------------------------------
 //
void DataContainer::recomputeMaxValues()
{
    float temp_max_astro_coverage = 0;
    float temp_max_synapse_volume = 0;
    int temp_max_volume = 1;
    for ( auto iter = m_objects.begin(); iter != m_objects.end(); iter++ ) {
        Object *obj = (*iter).second;
        if (obj->isFiltered() || obj->getObjectType() == Object_t::ASTROCYTE)
            continue;

        if (obj->getObjectType() == Object_t::MITO) {
            // check its parent, if astrocyte it wont be fair to include it
            int parentID =obj->getParentID();
            if (m_objects.find(parentID) != m_objects.end()) {
                Object *parent = m_objects[parentID];
                if (parent->getObjectType()  == Object_t::ASTROCYTE  )
                    continue;
            }
        }

        if (temp_max_synapse_volume < obj->getSynapseSize())
            temp_max_synapse_volume = obj->getSynapseSize();

        // need to update these info whenever we filter or change the threshold
        if (temp_max_astro_coverage < obj->getAstroCoverage())
           temp_max_astro_coverage = obj->getAstroCoverage();

        if (temp_max_volume < obj->getVolume())
            temp_max_volume = obj->getVolume();

    }

    max_synapse_volume = temp_max_synapse_volume;
    max_volume = temp_max_volume;
    max_astro_coverage = temp_max_astro_coverage;
}
