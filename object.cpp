// todo:
// filter objects by properties
// color objects by different properties: funtion, type, ?
// label
// check if parent and child has the same center
// if yes then add offset to one of them

// bouton 510!!!

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
	m_mappedValue = 0;
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

    ssbo_data.info.setY(getAstroCoverage()); // how many vertices are covered by astrocyte

    // make sure it is valid ID?
    ssbo_data.info.setZ(m_parentID);

    ssbo_data.info.setW(0); // 1 bit: visibility, 2 bit: marked
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

void Object::fixSkeleton(Object *parent)
{
    // check if the parent is not null again
    if (parent == NULL)
        return;

    std::vector<SkeletonBranch *> * branches = m_skeleton->getBufferedBranches();
    // iterate over this object branches
    for ( int i = 0; i < branches->size(); ++i ) {
        this->addSkeletonBranch(branches->at(i), parent);
    }

    m_skeleton->clearBufferedBranches();
}

bool Object::addSkeletonBranch(SkeletonBranch *branch, Object *parent)
{
    if ( hasParent() ) { // use parent skeleton and mark child there
        if ( parent == NULL ) {
            qDebug() << "child has no parent";
            // add it to list of empty branches to be processed later
            m_skeleton->addBufferedBranches(branch);
            return false;
        }

        // I have the parent, access the parent
        // mark the points and nodes in this branch at the parent with this child ID

        if (parent->hasParent()) {
            qDebug() << "parent has parent! ";
            // use parent parent skeleton, but the parent itself id
            return false;
        }

        parent->markChildSubSkeleton(branch, m_ID);
        Skeleton *parentSkeleton = parent->getSkeleton();
        m_skeleton->addBranch(branch, parentSkeleton);
    } else {
        m_skeleton->addBranch(branch, NULL);
    }

    return true;
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

void Object::addChild(Object *child)
{
    // qDebug() << "Adding " << child->getHVGXID() << " as the " << m_children.size() << "th child to " <<  m_ID;
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
    // qDebug() << m_averageDistance << " " <<  m_averageDistance /(float) m_meshIndices.size();
    return (1.0 / (m_averageDistance /(float) m_meshIndices.size()) ); // the smaller the better value
}

bool Object::hasParent()
{
   if (m_object_t == Object_t::BOUTON || m_object_t == Object_t::SPINE ||  m_object_t == Object_t::MITO  )
       return true;

   return false;
}

int Object::getSynapseSize()
{
    // loop over list of synapses this object has (spine , bouton)
    // loop over children and get their size if (dendrite, axon )
    int synapse_size = 0;
    if ( m_synapses.size() > 0 && (m_object_t == Object_t::SPINE || m_object_t == Object_t::BOUTON) ) {
        // or get the maximum?
         for (int i = 0; i < m_synapses.size(); ++i) {
             Object *synapse = m_synapses[i];
             if (synapse_size <  synapse->getVolume())
                synapse_size  = synapse->getVolume();
         }

    } else if ( m_children.size() > 0 && (m_object_t == Object_t::DENDRITE || m_object_t == Object_t::AXON)) {
        for (int i = 0; i < m_children.size(); ++i) {
            Object *child = m_children[i];
            synapse_size +=child->getSynapseSize();
        }

        synapse_size /= m_children.size();
    } else {
        synapse_size = 5;
    }

    return synapse_size;
}
