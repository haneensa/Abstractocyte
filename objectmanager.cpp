#include <chrono>
#include "objectmanager.h"

// what do I need here?
// vbos and vaos to render the data
// thus their associated buffers
/*
 * m_objects -> object class for each object (astrocyte, dendrite, ..)
 *           -> get from this the indices of the mesh
 *           -> get the skeleton vertices
 *
 * m_buffer_data for ssbo initialization which I can take by looping over all m_objects
 */
ObjectManager::ObjectManager()
    :  m_bindIdx(2),
       m_glFunctionsSet(false),
       m_vbo_mesh( QOpenGLBuffer::VertexBuffer ),
       m_vbo_IndexMesh(QOpenGLBuffer::IndexBuffer),
       m_vbo_skeleton( QOpenGLBuffer::VertexBuffer )
{
    m_indices_size = 0;
    m_skeleton_nodes_size = 0;
    m_limit = 43;
    m_vertex_offset = 0;
    m_ssbo_data.resize(746);
    m_mesh = new Mesh();

    //QString path;
    //loadSkeletonPoints(path); // 11638884, 19131720
    //path = "://data/input_data/mouse03_astro_mesh_center_skeleton.obj.bin";

    //loadDataset(path); //time:  54066.4 ms
    //path = "://data/input_data/mouse03_neurite_center_skeleton_bleeding_conn.obj.bin";
    //loadDataset(path); //time:  53812 ms


    importXML("://scripts/m3_astrocyte.xml"); // astrocyte  time:  79150.9 ms
    importXML("://scripts/m3_neurites.xml"); // neurites time:  28802 ms
}

ObjectManager::~ObjectManager()
{
    qDebug() << "~Mesh()";
    if (m_glFunctionsSet) {
        qDebug() << "~Mesh()";
        for (std::size_t i = 0; i != m_objects.size(); i++) {
            delete m_objects[i];
        }

        glDeleteProgram(m_program_skeleton);
        glDeleteProgram(m_program_mesh);
        glDeleteProgram(m_program_mesh_points);

        m_vao_mesh.destroy();
        m_vbo_mesh.destroy();
    }
}

bool ObjectManager::importXML(QString path)
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

void ObjectManager::parseConnGraph(QXmlStreamReader &xml)
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
void ObjectManager::parseObject(QXmlStreamReader &xml, Object *obj)
{
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() != "o") {
        qDebug() << "Called XML parseObejct without attribs";
        return;
    }

    QString nameline  = xml.attributes().value("name").toString();
    QList<QString> nameList = nameline.split('_');
    QString name;
    for (int i = 0; i < nameList.size() - 1; ++i)
        name += nameList[i];

    int hvgxID = nameList[nameList.size() - 1].toInt();
    obj = new Object(name.toUtf8().constData(), hvgxID);

    QVector4D color = obj->getColor();
    // set ssbo with this ID to this color

    xml.readNext();

    // this object structure is not done
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "o")) {
        // go to the next child of object node
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "mesh") {
                parseMesh(xml, obj);
            } else if (xml.name() == "skeleton") {
                parseSkeleton(xml, obj);
            } else if (xml.name() == "volume") {
                xml.readNext();
                int volume =  xml.text().toInt();
                obj->setVolume(volume);
                //qDebug() << "volume: " << volume;
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
                //qDebug() << "center: " << x << " " << y << " " << z;
                obj->setCenter(QVector4D(x, y, z, 0));
                // set ssbo with this center
            }
        } // if start element
        xml.readNext();
    } // while

    if (obj != NULL) {
         m_objects[hvgxID] =  obj;
         m_ssbo_data[hvgxID] = obj->getSSBOData();
    }
}


void ObjectManager::parseMesh(QXmlStreamReader &xml, Object *obj)
{
    // read vertices and their faces into the mesh
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() != "mesh") {
        qDebug() << "Called XML parseObejct without attribs";
        return;
    }

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

                float x1 = stringlist.at(0).toDouble();
                float y1 = stringlist.at(1).toDouble();
                float z1 = stringlist.at(2).toDouble();

                QVector3D mesh_vertex(x1, y1, z1);
                struct VertexData v;
                v.ID = obj->getHVGXID(); // this used to index ssbo
                v.mesh_vertex = mesh_vertex;
                v.bleed = 0;
                if (stringlist.size() < 5) {
                    // place holder
                    v.skeleton_vertex = mesh_vertex;
                } else {
                    float x2 = stringlist.at(3).toDouble();
                    float y2 = stringlist.at(4).toDouble();
                    float z2 = stringlist.at(5).toDouble();
                    QVector3D skeleton_vertex(x2, y2, z2);
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

void ObjectManager::parseSkeleton(QXmlStreamReader &xml, Object *obj)
{
    // read skeleton vertices and their edges
    // read vertices and their faces into the mesh
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() != "skeleton") {
        qDebug() << "Called XML parseObejct without attribs";
        return;
    }

    xml.readNext();

    int vertices = 0;
    int lines = 0;
    // this object structure is not done
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "skeleton")) {
        // go to the next child of object node
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "sk") { // make it v
                vertices++;
                xml.readNext();
                QString coords = xml.text().toString();
                QStringList stringlist = coords.split(" ");
                if (stringlist.size() < 3) {
                    continue;
                }

                float x = stringlist.at(0).toDouble();
                float y = stringlist.at(1).toDouble();
                float z = stringlist.at(2).toDouble();

                struct SkeletonVertex sk_vertex = {QVector3D(x, y, z), obj->getHVGXID()};
                obj->add_s_vertex(sk_vertex);
                m_skeleton_nodes_size++;

            } else if (xml.name() == "l") {
                lines++;
                xml.readNext();
                QString coords = xml.text().toString();
                QStringList stringlist = coords.split(" ");
                if (stringlist.size() < 2) {
                    continue;
                }

                int nodeID1 = stringlist.at(0).toInt();
                int nodeID2 = stringlist.at(1).toInt();
            }
        } // if start element
        xml.readNext();
    } // while

    qDebug() << "vertices count: " << vertices << " lines: " << lines;
}

void ObjectManager::parseBranch(QXmlStreamReader &xml)
{

}

/*
o astrocyte
p x y z # astrocyte center point
b branch_2_1_3
sv x y z # skeleton vertex
sv x y z
.....
l sv_id1 sv_id2  # line segments between skeleton vertices in branch
l sv_id1 sv_id2
.....
v x y z sv_index # mesh vertices and nearest skeleton vertex to it
v x y z sv_index
v x y z sv_index
v x y z sv_index
...
f v1 v2 v3
f v1 v2 v3
....

# Above Would be Repeated for all objects.

# connectivity edges between neurites
c connecitity_graph1
l p1_idx p2_idx
l p1_idx p2_idx

conn connecitity_graph2
l p1_idx p2_idx
l p1_idx p2_idx

  */


bool ObjectManager::loadDataset(QString path)
{
    // open file
    // if 'o' then create new object and set its name and hvgx ID
    // if 'p' then add to obj its center
    // if vl: then add object volume
    // if 'sv' then add to the skeleton data in obj
    // if 'l' then add edges information which connects skeleton points
    // if v and f construct the mesh vertices

    // if 'c connecitity_graph1' then add these info to the points IDs for neurite neurite connictivity 2D mode which astrocyte disappears
    // if 'c connecitity_graph2' then add skeleton-neurite connecivity points which would be shown only when? in the 2D mode

    qDebug() << "Func: loadDataset";
    auto t1 = std::chrono::high_resolution_clock::now();

    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "Could not open the file for reading";
        return false;
    }

    QTextStream in(&file);
    QList<QByteArray> wordList;

    QList<QByteArray> markersList; // store the list of vertices close to astrocyte

    int flag_prev = 0;

    Object *obj = NULL;
    std::string name;
    QVector4D center = QVector4D(0.0, 0.0, 0.0, 0.0);
    // ssbo buffer info for mesh data
    // [color, center, volume, type, ?]
    int neuritesTOneurites_flag = 0;
    m_vertex_offset += m_mesh->getVerticesSize();
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        wordList = line.split(' ');

        if (wordList[0] == "o") {
            // o objname_hvgxID

            if (flag_prev == 1) {
                int hvgxID = obj->getHVGXID();
                if (hvgxID > m_ssbo_data.size())
                    m_ssbo_data.resize(hvgxID + 1);

                m_ssbo_data[hvgxID] = obj->getSSBOData();
                m_objects[hvgxID] =  obj;
            }

            if (m_objects.size() > m_limit) {
                flag_prev = 0;
                qDebug() << "* Reached size limit.";
                break;
            }

            QByteArray nameline  = wordList[1].data();
            QList<QByteArray> nameList = nameline.split('_');
            name.clear();
            for (int i = 0; i < nameList.size() - 1; ++i)
                name += nameList[i].data();

            int hvgxID = atoi(nameList[nameList.size() - 1].data());
            obj = new Object(name, hvgxID);
            // get objet color based on type
            QVector4D color = obj->getColor();
            flag_prev = 1;
        } else if (wordList[0] == "p") {
            // p centerX centerY centerZ
            if (wordList.size() < 4) {
                qDebug() << "p word list";
                break;
            }
            float x = atof(wordList[1].data());
            float y = atof(wordList[2].data());
            float z = atof(wordList[3].data());
            if (obj->getObjectType() == Object_t::ASTROCYTE ) {
                center = QVector4D(x, y, z, 0);
            } else {
                center = QVector4D(x, y, z, 1);
            }

            obj->setCenter(center);
        } else if (wordList[0] == "vl") { // to do: update the mesh file to remove extra space
            if (wordList.size() < 2) {
                qDebug() << "vl word list";
                break;
            }
            int volume = atoi(wordList[1].data());
            obj->setVolume(volume);
        } else if (wordList[0]  == "v") {
            if (wordList.size() < 4) {
                qDebug() << "v word list";
                break;
            }
            float x1 = atof(wordList[1].data());
            float y1 = atof(wordList[2].data());
            float z1 = atof(wordList[3].data());
            QVector3D mesh_vertex(x1, y1, z1);
            struct VertexData v;
            v.ID = obj->getHVGXID(); // this used to index ssbo
            v.mesh_vertex = mesh_vertex;
            v.bleed = 0;
            if (wordList.size() < 6) {
                // place holder
                v.skeleton_vertex = mesh_vertex;
            } else {
                float x2 = atof(wordList[4].data());
                float y2 = atof(wordList[5].data());
                float z2 = atof(wordList[6].data());
                QVector3D skeleton_vertex(x2, y2, z2);
                v.skeleton_vertex = skeleton_vertex;
            }
            m_mesh->addVertex(v);

        }  else if (wordList[0]  == "f") {
            if (wordList.size() < 4) {
                qDebug() << "f word list";
                break;
            }
            unsigned int f1_index = atoi(wordList[1].data()) + m_vertex_offset;
            unsigned int f2_index = atoi(wordList[2].data()) + m_vertex_offset;
            unsigned int f3_index = atoi(wordList[3].data()) + m_vertex_offset;

            if ( ! m_mesh->isValidFaces(f1_index, f2_index, f3_index) ) {
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

            m_mesh->MarkBleedingVertices(markersList, m_vertex_offset);

        } else if (wordList[0] == "b") { // branch
            // b branch_branchID_parentBranch_knot1_knot2
        } else if (wordList[0] == "sk") { // skeleton point
            if (wordList.size() < 4) {
                qDebug() << "sk word list";
                break;
            }
            // add it to skeleton point cloud
            float x = atof(wordList[1].data());
            float y = atof(wordList[2].data());
            float z = atof(wordList[3].data());
            struct SkeletonVertex sk_vertex = {QVector3D(x, y, z), obj->getHVGXID()};
            obj->add_s_vertex(sk_vertex);
            m_skeleton_nodes_size++;
        } else if (wordList[0] == "conn") { // edge between skeleton points
            qDebug() << line;
            if (wordList[1] == "neuritesToneurites")
                neuritesTOneurites_flag = 1;

        } else if (wordList[0] == "l") { // edge between skeleton points
            if (wordList.size() < 3) {
                qDebug() << "l word list";
                continue;
            }
            int source = atoi(wordList[1].data());
            int target = atoi(wordList[2].data());
            QVector2D edge_info = QVector2D(source, target);
            neurites_neurite_edge.push_back(edge_info);
        } else if (wordList[0] == "rv") {
            if (wordList.size() < 2)
                continue;
            markersList = wordList;
        }
    }

    if (flag_prev == 1) {
        int hvgxID = obj->getHVGXID();
        if (hvgxID > m_ssbo_data.size())
            m_ssbo_data.resize(hvgxID + 1);

        m_ssbo_data[hvgxID] = obj->getSSBOData();
        m_objects[hvgxID] =  obj;
    }

    qDebug() << "neurites_neurite_edge: " << neurites_neurite_edge.size();
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms = t2 - t1;
    qDebug() << "time: " << ms.count() << "ms";

    return true;
}

bool ObjectManager::initVertexAttrib()
{
    if (m_glFunctionsSet == false)
        return false;

    int offset = 0;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexData),  0);


    offset +=  sizeof(QVector3D);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexData), (GLvoid*)offset);


    offset += sizeof(QVector3D);
    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(2, 1, GL_INT, sizeof(VertexData), (GLvoid*)offset);

    offset += sizeof(GL_INT);
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 1, GL_INT, sizeof(VertexData), (GLvoid*)offset);

    return true;

}

// temp functions until we think of way to represent the data
int ObjectManager::getNodesCount()
{
    return m_skeleton_nodes_size;
}

bool ObjectManager::initOpenGLFunctions()
{
    m_glFunctionsSet = true;
    initializeOpenGLFunctions();

    return true;
}

bool ObjectManager::iniShadersVBOs()
{
    initBuffer();

    initMeshShaders();
    initMeshPointsShaders();
    initSkeletonShaders();

    return true;
}

bool ObjectManager::initBuffer()
{
    if (m_glFunctionsSet == false)
        return false;

    qDebug() <<  " m_buffer_data.size() : " << m_ssbo_data.size() ;
    int bufferSize =  m_ssbo_data.size() * sizeof(struct ssbo_mesh);

    glGenBuffers(1, &m_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize , NULL, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bindIdx, m_ssbo);
    qDebug() << "mesh buffer size: " << bufferSize;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
    GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
    memcpy(p,   m_ssbo_data.data(),  bufferSize);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    return true;
}

bool ObjectManager::initMeshShaders()
{
    /* start initializing mesh */
    qDebug() << "Initializing MESH";
    m_program_mesh = glCreateProgram();
    bool res = initShader(m_program_mesh, ":/shaders/mesh.vert", ":/shaders/mesh.geom", ":/shaders/mesh.frag");
    if (res == false)
        return res;

    // create vbos and vaos
    m_vao_mesh.create();
    m_vao_mesh.bind();

    glUseProgram(m_program_mesh); // m_program_mesh->bind();

    updateUniformsLocation(m_program_mesh);


    QVector3D lightDir = QVector3D(-2.5f, -2.5f, -0.9f);
    GLuint lightDir_loc = glGetUniformLocation(m_program_mesh, "diffuseLightDirection");
    glUniform3fv(lightDir_loc, 1, &lightDir[0]);

    m_vbo_mesh.create();
    m_vbo_mesh.setUsagePattern( QOpenGLBuffer::StaticDraw );
    if ( !m_vbo_mesh.bind() ) {
        qDebug() << "Could not bind vertex buffer to the context.";
    }

    m_mesh->allocateVerticesVBO(m_vbo_mesh);

    // initialize index buffers
    m_vbo_IndexMesh.create();
    m_vbo_IndexMesh.bind();
    m_vbo_IndexMesh.allocate( NULL, m_indices_size * sizeof(GLuint) );
    int offset = 0;
    for ( auto iter = m_objects.begin(); iter != m_objects.end(); iter++) {
      //  if (m_objects[i]->getObjectType() == Object_t::SYNAPSE  ||m_objects[i]->getObjectType() == Object_t::MITO   ) {
         //   continue;
      //  }
        Object *object_p = (*iter).second;
        int count = object_p->get_indices_Size() * sizeof(GLuint);
        qDebug() << " allocating: " << object_p->getName().data();
        m_vbo_IndexMesh.write(offset, object_p->get_indices(), count);
        offset += count;
   }

    m_vbo_IndexMesh.release();

    initVertexAttrib();

    m_vbo_mesh.release();
    m_vao_mesh.release();

}

bool ObjectManager::initMeshPointsShaders()
{
    /***************************************/
    qDebug() << "Initializing MESH POINTS";
    m_program_mesh_points = glCreateProgram();
    bool res = initShader(m_program_mesh_points, ":/shaders/mesh.vert", ":/shaders/mesh_points.geom", ":/shaders/mesh_points.frag");
    if (res == false)
        return res;

    // create vbos and vaos
    m_vao_mesh_points.create();
    m_vao_mesh_points.bind();

    glUseProgram(m_program_mesh_points);

    QVector3D lightDir = QVector3D(-2.5f, -2.5f, -0.9f);
    GLuint lightDir_loc = glGetUniformLocation(m_program_mesh_points, "diffuseLightDirection");
    glUniform3fv(lightDir_loc, 1, &lightDir[0]);


    if ( !m_vbo_mesh.bind() ) {
        qDebug() << "Could not bind vertex buffer to the context.";
    }

    updateUniformsLocation(m_program_mesh_points);

    initVertexAttrib();

    m_vbo_mesh.release();
    m_vao_mesh_points.release();
}

bool ObjectManager::initSkeletonShaders()
{
    /********** START SKELETON **************/
    qDebug() << "point";

    m_program_skeleton = glCreateProgram();
    bool res = initShader(m_program_skeleton, ":/shaders/skeleton_point.vert", ":/shaders/skeleton_point.geom", ":/shaders/skeleton_point.frag");
    if (res == false)
        return res;

    qDebug() << "Initializing SKELETON 1";
    m_vao_skeleton.create();
    m_vao_skeleton.bind();

    glUseProgram(m_program_skeleton);

    updateUniformsLocation(m_program_skeleton);

    m_vbo_skeleton.create();
    m_vbo_skeleton.setUsagePattern( QOpenGLBuffer::StaticDraw);
    if ( !m_vbo_skeleton.bind() ) {
        qDebug() << "Could not bind vertex buffer to the context.";
    }

    m_vbo_skeleton.allocate(NULL, m_skeleton_nodes_size * sizeof(SkeletonVertex));

    int offset = 0;
    for ( auto iter = m_objects.begin(); iter != m_objects.end(); iter++) {
        Object *object_p = (*iter).second;
        int count = object_p->get_s_Size() * sizeof(SkeletonVertex);
        m_vbo_skeleton.write(offset, &object_p->get_s_Vertices()[0], count);
        qDebug() <<  " allocating: " << object_p->getName().data() << " count: " << count;
        offset += count;
   }

    qDebug() << " m_mesh.getNodesCount(): " << m_skeleton_nodes_size;
    qDebug() << " m_mesh.getNodesCount()* sizeof(QVector3D): " << m_skeleton_nodes_size* sizeof(SkeletonVertex);

    GL_Error();

    offset = 0;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkeletonVertex),  0);

    offset += sizeof(QVector3D);
    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(1, 1, GL_INT, sizeof(SkeletonVertex), (GLvoid*)offset);

    GL_Error();

    m_vbo_skeleton.release();
    m_vao_skeleton.release();
}

void ObjectManager::draw()
{
    /************************/
    // I need this because vertex <-> skeleton mapping is not complete
    m_vao_skeleton.bind();
    glUseProgram(m_program_skeleton);

    updateUniformsLocation(m_program_skeleton);

    glDrawArrays(GL_POINTS, 0,  m_skeleton_nodes_size );
    m_vao_skeleton.release();

    /************************/

    glDisable (GL_BLEND);
    glBlendFunc (GL_ONE, GL_ONE);

    // I need this because transitioning from mesh to skeleton is not smooth
    m_vao_mesh_points.bind();
    glUseProgram(m_program_mesh_points);

    updateUniformsLocation(m_program_mesh_points);

    m_vbo_IndexMesh.bind();
    glDrawElements(GL_POINTS, m_indices_size,  GL_UNSIGNED_INT, 0 );
    m_vbo_IndexMesh.release();

    m_vao_mesh_points.release();
    /************************/
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_vao_mesh.bind();
    glUseProgram(m_program_mesh);

    updateUniformsLocation(m_program_mesh);

    m_vbo_IndexMesh.bind();
    glDrawElements(GL_TRIANGLES, m_indices_size,  GL_UNSIGNED_INT, 0 );
    m_vbo_IndexMesh.release();
    m_vao_mesh.release();
}

void ObjectManager::updateUniforms(struct MeshUniforms mesh_uniforms)
{
    m_uniforms = mesh_uniforms;
}

void ObjectManager::updateUniformsLocation(GLuint program)
{
    if (m_glFunctionsSet == false)
        return;

    // initialize uniforms
    GLuint mMatrix = glGetUniformLocation(program, "mMatrix");
    glUniformMatrix4fv(mMatrix, 1, GL_FALSE, m_uniforms.mMatrix);

    GLuint vMatrix = glGetUniformLocation(program, "vMatrix");
    glUniformMatrix4fv(vMatrix, 1, GL_FALSE, m_uniforms.vMatrix);

    GLuint pMatrix = glGetUniformLocation(program, "pMatrix");
    glUniformMatrix4fv(pMatrix, 1, GL_FALSE, m_uniforms.pMatrix);

    GLint y_axis = glGetUniformLocation(program, "y_axis");
    glUniform1iv(y_axis, 1, &m_uniforms.y_axis);

    GLint x_axis = glGetUniformLocation(program, "x_axis");
    glUniform1iv(x_axis, 1, &m_uniforms.x_axis);
}

std::map<int, Object*>  ObjectManager::getNeuriteNodes()
{
    return m_objects;
}

std::vector<QVector2D> ObjectManager::getNeuritesEdges()
{
    return neurites_neurite_edge;
}
