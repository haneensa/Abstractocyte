// todo:
// filter objects by properties
// color objects by different properties: funtion, type, ?
// label
// check if parent and child has the same center
// if yes then add offset to one of them

#include "object.h"
#include <QDebug>
#include <algorithm>

#include "colors.h" //bouton1insolid.044

Object::Object(std::string name, int ID)
{
    if (name[name.size()-1] == '\n') {
        name.erase(name.size()-1, name.size()-1);
    }

    m_name  = name; // obj name
    m_ID    = ID;   // hvgx ID
    m_object_t = getObjectType(); // obect type
    m_color = QVector4D(1.0, 1.0, 0.0, 1.0);    // default one
    m_volume = 0;
    m_center = QVector4D(0, 0, 0, 0);

    m_closest_astro_vertex.second =  1000000.0;
    m_closest_astro_vertex.first = -1;
    m_skeleton = new Skeleton(m_ID);

    m_parent = NULL;
    m_isFiltered = false;

    m_function = -1;
    qDebug() << "create " << m_name.data() << " hvgxID: " << m_ID;
    m_averageDistance = 0;
}

Object::~Object()
{
}

void Object::updateClosestAstroVertex(float dist, int vertexIdx)
{
    m_averageDistance += dist;
    if (dist < m_closest_astro_vertex.second) {
        m_closest_astro_vertex.first = -1; //   I want from astrocyte skeleton :(
        m_closest_astro_vertex.second = dist;
    }
    if (dist < ASTRO_DIST_THRESH)
        m_VertexidxCloseToAstro.push_back(vertexIdx);
}

Object_t Object::getObjectType()
{
    // make it generic later
    std::string code = m_name.substr(0, 3);
    std::transform(code.begin(), code.end(), code.begin(), ::tolower);

    if (code.compare("ast") == 0) {
       return Object_t::ASTROCYTE;
    } else if (code.compare("axo") == 0) {
        return Object_t::AXON;
    } else if (code.compare("den") == 0) {
        return Object_t::DENDRITE;
    } else if (code.compare("spi") == 0) {
        return Object_t::SPINE;
    } else if (code.compare("bou") == 0 || code.compare("buo") == 0 ) {
        return Object_t::BOUTON;
    } else if (code.compare(0, 1, "m") == 0) {
        return Object_t::MITO;
    } else if ((code.compare(0, 1, "d") == 0 && code.compare(0, 1, "e")) || code.compare("syn") == 0 ) {
        return Object_t::SYNAPSE;
    } else {
        return Object_t::UNKNOWN;
    }
}

void Object::addTriangleIndex(GLuint face)
{
    m_meshIndices.push_back(face);
}

QVector4D Object::getColor()
{
    switch (m_object_t) {
    case Object_t::ASTROCYTE:
        m_color = firebrick;
        break;
    case Object_t::AXON:
        m_color = lightseagreen;
        break;
    case Object_t::DENDRITE:
        m_color = gold;
        break;
    case Object_t::SPINE:
        m_color = orange;
        break;
    case Object_t::BOUTON:
        m_color = green;
        break;
    case Object_t::MITO:
        m_color = mediumslateblue;
        break;
    case Object_t::SYNAPSE:
        m_color = lightgoldenrodyellow;
        break;
    case Object_t::GLYCOGEN:
        m_color = red;
        break;
    default:
        qDebug() << m_name.data();
        m_color = gray;
        break;
    };

    return m_color;
}

void Object::setCenter(QVector4D center)
{
    m_center = center;
}

void Object::setAstPoint(QVector4D ast_point)
{
    m_ast_point = ast_point;
}

struct ssbo_mesh Object::getSSBOData()
{
    struct ssbo_mesh ssbo_data;
    ssbo_data.color = m_color;
    ssbo_data.color.setW(m_closest_astro_vertex.second);

    ssbo_data.center = m_center;
    int type = (int) m_object_t;

    ssbo_data.center.setW(type);
    ssbo_data.info.setX(1); // node size

    qDebug() << "m_VertexidxCloseToAstro: " << m_VertexidxCloseToAstro.size() ;
    ssbo_data.info.setY(m_VertexidxCloseToAstro.size()); // how many vertices are covered by astrocyte

    if (m_parent != NULL)
        ssbo_data.info.setZ(m_parent->getHVGXID());
    else
        ssbo_data.info.setZ(-1);

    ssbo_data.info.setW(0); // filtered? 0: no, 1: yes
    ssbo_data.layout1 = m_center.toVector2D();
    ssbo_data.layout2 = m_center.toVector2D();
    return ssbo_data;
}

// skeleton management
void Object::addSkeletonNode(QVector3D coords)
{


    // also ID to be used for the vertex (parent or child)
    // how would I know this node belongs to a child
    // and which child it would be?
    // for children, I have the list of IDs of points that belong to the child
    // then get the skeleton of its parent
    // and mark these with child IDs
    m_skeleton->addNode(coords);
}

void Object::addSkeletonPoint(QVector3D coords)
{
    m_skeleton->addPoint(coords);
}

int Object::writeSkeletontoVBO(QOpenGLBuffer vbo, int offset)
{
    int size = m_skeleton->getSkeletonPointsSize();
    if (size == 0) {
        qDebug() << "no skeleton";
        return 0;
    }

    int count = size * sizeof(SkeletonPoint);
    vbo.write(offset, m_skeleton->getSkeletonPoints(), count);
    qDebug() <<  " allocating: " <<  getName().data() << " count: " << count;

    return count;
}
void Object::addSkeletonBranch(SkeletonBranch *branch)
{
    if ( m_parent != NULL &&
         (m_object_t == Object_t::SPINE || m_object_t == Object_t::BOUTON || m_object_t == Object_t::MITO)) {
        // I have the parent
        // access the parent
        // mark the points and nodes in this branch at the parent
        // with this child ID
        m_parent->markChildSubSkeleton(branch, m_ID);
        Skeleton *parentSkeleton = m_parent->getSkeleton();
        m_skeleton->addBranch(branch, parentSkeleton);
    } else {
        m_skeleton->addBranch(branch, NULL);
    }

}

void Object::markChildSubSkeleton(SkeletonBranch *childBranch, int childID)
{
    if (childBranch == NULL)
        return;

    // no need to mark the nodes since they are already part of the points
    std::vector<int> childPoints = childBranch->getPointsIndxs();
    for (int i = 0; i < childPoints.size(); ++i) {
        m_skeleton->markPoint(childPoints[i], childID);

    }

}

void Object::setParentID(Object *parent)
{
    qDebug() << "Adding " << parent->getHVGXID() << " as parent to " <<  m_ID;
    m_parent = parent;
}

void Object::addChild(Object *child)
{
    qDebug() << "Adding " << child->getHVGXID() << " as the " << m_children.size() << "th child to " <<  m_ID;
    m_children.push_back(child);
}


// if object type == synapse
void Object::UpdateSynapseData(int axon_id, int dendrite_id, int spine_id, int bouton_id)
{
    if (m_object_t != Object_t::SYNAPSE)
        return;

    m_synapse_data.axon = axon_id;
    m_synapse_data.dendrite = dendrite_id;
    m_synapse_data.spine = spine_id;
    m_synapse_data.bouton = bouton_id;

}

void Object::addSynapse(Object *synapse_object)
{
    if (synapse_object == NULL)
        return;

    m_synapses.push_back(synapse_object);
}

float Object::getAstroCoverage()
{
    // return m_VertexidxCloseToAstro.size();
    qDebug() << m_averageDistance << " " <<  m_averageDistance /(float) m_meshIndices.size();
    return 1.0/(m_averageDistance /(float) m_meshIndices.size()); // the smaller the better value
}
