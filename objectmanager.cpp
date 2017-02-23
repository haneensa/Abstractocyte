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
    m_limit = 20;
    m_vertex_offset = 0;
    m_ssbo_data.resize(1200);
    m_mesh = new Mesh();

   importXML("://scripts/m3_astrocyte.xml");   // astrocyte  time:  79150.9 ms
//   importXML("://scripts/m3_neurites.xml");    // neurites time:  28802 ms
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
    qDebug() << xml.name();

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
            }
        } // if start element
        xml.readNext();
    } // while

    if (obj != NULL) {
         m_objects[hvgxID] =  obj;
         if (m_ssbo_data.size() < hvgxID) {
             qDebug() << "resizing m_ssbo_data.";
             m_ssbo_data.resize(hvgxID + 100);
         }

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
void ObjectManager::parseSkeleton(QXmlStreamReader &xml, Object *obj)
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
            } else if (xml.name() == "branches") {
              // process branches
                parseBranch(xml, obj);
            }
        } // if start element
        xml.readNext();
    } // while
}

void ObjectManager::parseSkeletonNodes(QXmlStreamReader &xml, Object *obj)
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

void ObjectManager::parseSkeletonPoints(QXmlStreamReader &xml, Object *obj)
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
                m_skeleton_nodes_size++;
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

void ObjectManager::parseBranch(QXmlStreamReader &xml, Object *obj)
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

    SkeletonBranch *branch = NULL;
    // this object structure is not done
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "branches")) {
        // go to the next child of object node
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "b") {
                branch = new SkeletonBranch();
                int ID = xml.attributes().value("name").toInt();
            } else if (xml.name() == "knots") {
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
            } else if (xml.name() == "points_ids") {
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


bool ObjectManager::initVertexAttrib()
{
    if (m_glFunctionsSet == false)
        return false;

    int offset = 0;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,
                          sizeof(VertexData),  0);


    offset +=  sizeof(QVector4D);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE,
                          sizeof(VertexData), (GLvoid*)offset);

    return true;

}


bool ObjectManager::initOpenGLFunctions()
{
    m_glFunctionsSet = true;
    initializeOpenGLFunctions();

    return true;
}

bool ObjectManager::iniShadersVBOs()
{
    // fill all data through iterating over objects(ssbo, vbos)
    fillVBOsData();

    initSSBO();
    initMeshShaders();
    initMeshPointsShaders();
    initSkeletonShaders();

    return true;
}

void ObjectManager::fillVBOsData()
{
    m_vbo_skeleton.create();
    m_vbo_skeleton.setUsagePattern( QOpenGLBuffer::StaticDraw);
    m_vbo_skeleton.bind();
    m_vbo_skeleton.allocate(NULL, m_skeleton_nodes_size * sizeof(SkeletonPoint));


    // initialize index buffers
    m_vbo_IndexMesh.create();
    m_vbo_IndexMesh.bind();
    m_vbo_IndexMesh.allocate( NULL, m_indices_size * sizeof(GLuint) );

    for ( auto iter = m_objects.begin(); iter != m_objects.end(); iter++) { // todo: skip if the object has no skeleton
        Object *object_p = (*iter).second;


   }

    int vbo_IndexMesh_offset = 0;
    int vbo_skeleton_offset = 0;
    for ( auto iter = m_objects.begin(); iter != m_objects.end(); iter++) {
        Object *object_p = (*iter).second;

        // todo: fill ssbo here

        if (object_p->getObjectType() == Object_t::MITO  || object_p->getObjectType()  == Object_t::SYNAPSE   ) {
              continue;
        }

        int ID = object_p->getHVGXID();

        qDebug() << " allocating: " << object_p->getName().data();

        // allocating mesh indices
        int vbo_IndexMesh_count = object_p->get_indices_Size() * sizeof(GLuint);
        m_vbo_IndexMesh.write(vbo_IndexMesh_offset, object_p->get_indices(), vbo_IndexMesh_count);
        vbo_IndexMesh_offset += vbo_IndexMesh_count;

        // allocating skeleton vertices
        int vbo_skeleton_count = object_p->writeSkeletontoVBO(m_vbo_skeleton, vbo_skeleton_offset);
        vbo_skeleton_offset += vbo_skeleton_count;

        // allocate neurites nodes place holders
        object_p->setNodeIdx(m_neurites_nodes.size());
        m_neurites_nodes.push_back(ID);


        // initialize abstract skeleton data
        // find a way to fill the skeleton with data
        // get skeleton of the object
        Skeleton *skeleton = object_p->getSkeleton();
        std::vector<QVector3D> nodes3D = skeleton->getGraphNodes();
        std::vector<QVector2D> edges2D = skeleton->getGraphEdges();
        object_p->setSkeletonOffset(m_abstract_skel_nodes.size());
        // add nodes
        for ( int i = 0; i < nodes3D.size(); i++) {
            QVector4D vertex = nodes3D[i];
            vertex.setW(ID);
            struct AbstractSkelNode skel_node = {vertex, vertex.toVector2D(), vertex.toVector2D(), vertex.toVector2D() };
            m_abstract_skel_nodes.push_back(skel_node);
        }

        int skeleton_offset = object_p->getSkeletonOffset();
        // add edges
        for (int i = 0; i < edges2D.size(); ++i) {
            int nID1 = edges2D[i].x() + skeleton_offset;
            int nID2 = edges2D[i].y() + skeleton_offset;
            m_abstract_skel_edges.push_back(nID1);
            m_abstract_skel_edges.push_back(nID2);
        }
   }

    // allocate neurites nodes edges
    std::vector<QVector2D> edges_info = neurites_neurite_edge;
    for (int i = 0; i < edges_info.size(); ++i) {
        int nID1 = edges_info[i].x();
        int nID2 = edges_info[i].y();
        if (m_objects.find(nID1) == m_objects.end() || m_objects.find(nID2) == m_objects.end()) {
            continue;
        }
        m_neurites_edges.push_back(m_objects[nID1]->getNodeIdx());
        m_neurites_edges.push_back(m_objects[nID2]->getNodeIdx());
    }

    m_vbo_IndexMesh.release();
    m_vbo_skeleton.release();
}

void ObjectManager::allocate_neurites_nodes(QOpenGLBuffer vbo)
{
    vbo.allocate( m_neurites_nodes.data(),
                            m_neurites_nodes.size() * sizeof(GLuint) );
}

void ObjectManager::allocate_neurites_edges(QOpenGLBuffer vbo)
{
    vbo.allocate( m_neurites_edges.data(),
                                 m_neurites_edges.size() * sizeof(GLuint) );
}

int ObjectManager::get_neurites_nodes_size()
{
    return m_neurites_nodes.size();
}

int ObjectManager::get_neurites_edges_size()
{
    return m_neurites_edges.size();
}


void ObjectManager::allocate_abs_skel_nodes(QOpenGLBuffer vbo)
{
    vbo.allocate( m_abstract_skel_nodes.data(),
                                  m_abstract_skel_nodes.size() * sizeof(struct AbstractSkelNode) );
}

void ObjectManager::allocate_abs_skel_edges(QOpenGLBuffer vbo)
{
    vbo.allocate( m_abstract_skel_edges.data(),
                                  m_abstract_skel_edges.size() * sizeof(GLuint) );
}

int ObjectManager::get_abs_skel_nodes_size()
{
    return m_abstract_skel_nodes.size();
}

int ObjectManager::get_abs_skel_edges_size()
{
    return m_abstract_skel_edges.size();
}

bool ObjectManager::initSSBO()
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

    write_ssbo_data();

    return true;
}

void ObjectManager::write_ssbo_data()
{
    int bufferSize =  m_ssbo_data.size() * sizeof(struct ssbo_mesh);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
    GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
    memcpy(p,   m_ssbo_data.data(),  bufferSize);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

bool ObjectManager::initMeshShaders()
{
    /* start initializing mesh */
    qDebug() << "Initializing MESH";
    m_program_mesh = glCreateProgram();
    bool res = initShader(m_program_mesh, ":/shaders/mesh_vert.glsl", ":/shaders/mesh_geom.glsl", ":/shaders/mesh_frag.glsl");
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
    m_vbo_IndexMesh.bind();
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
    bool res = initShader(m_program_mesh_points, ":/shaders/mesh_vert.glsl", ":/shaders/mesh_points_geom.glsl", ":/shaders/mesh_points_frag.glsl");
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
    bool res = initShader(m_program_skeleton, ":/shaders/skeleton_point_vert.glsl", ":/shaders/skeleton_point_geom.glsl", ":/shaders/skeleton_point_frag.glsl");
    if (res == false)
        return res;

    qDebug() << "Initializing SKELETON 1";
    m_vao_skeleton.create();
    m_vao_skeleton.bind();

    glUseProgram(m_program_skeleton);

    updateUniformsLocation(m_program_skeleton);


    m_vbo_skeleton.bind();


    qDebug() << "writeSkeletontoVBO done ";

    GL_Error();

    int offset = 0;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(SkeletonPoint),  0);

    offset += sizeof(QVector4D);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(SkeletonPoint),  (GLvoid*)offset);

    offset += sizeof(QVector4D);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(SkeletonPoint),  (GLvoid*)offset);

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

void ObjectManager::updateUniforms(struct GlobalUniforms uniforms)
{
    m_uniforms = uniforms;
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

std::map<int, Object*>  ObjectManager::getObjectsMap()
{
    return m_objects;
}

std::vector<QVector2D> ObjectManager::getNeuritesEdges()
{
    return neurites_neurite_edge;
}

void ObjectManager::update_ssbo_data_layout1(QVector2D layout1, int hvgxID)
{
    if (m_ssbo_data.size() < hvgxID)
        return;

    m_ssbo_data[hvgxID].layout1 = layout1;
}

void ObjectManager::update_ssbo_data_layout2(QVector2D layout2, int hvgxID)
{
    if (m_ssbo_data.size() < hvgxID)
        return;

    m_ssbo_data[hvgxID].layout2 = layout2;
}

void ObjectManager::update_skeleton_layout1(QVector2D layout2, int hvgxID)
{
    // get the object -> get its skeleton -> update the layout
}

void ObjectManager::update_skeleton_layout2(QVector2D layout2, int hvgxID)
{
    // get the object -> get its skeleton -> update the layout
}

void ObjectManager::update_skeleton_layout3(QVector2D layout2, int hvgxID)
{
    // get the object -> get its skeleton -> update the layout
}
