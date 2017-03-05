// todo: add mitochondira node to neurites skeletons

#ifndef OBJECT_H
#define OBJECT_H

#include <QVector3D>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

#include "skeleton.h"
#include "mesh.h"
#include "ssbo_structs.h"

enum class Object_t { AXON, DENDRITE, BOUTON, SPINE, MITO, SYNAPSE, ASTROCYTE, GLYCOGEN, UNKNOWN };

class Skeleton;
class Object
{
public:
    Object(std::string name, int ID);
    ~Object();

    // temporary skeleton data points handleing function
    void add_s_vertex(struct SkeletonPoint vertex_data);

    int writeSkeletontoVBO(QOpenGLBuffer vbo, int offset);
    // mesh indices functions
    void addTriangleIndex(GLuint faces);
    size_t get_indices_Size()           { return m_meshIndices.size(); }
    void* get_indices()                 { return m_meshIndices.data(); }

    // properties getters
    Object_t getObjectType();
    std::string getName()               {  return m_name; }
    QVector4D getColor();
    QVector4D getCenter()               { return m_center; }
    QVector4D getAstPoint()               { return m_ast_point; }
    int getVolume()                     { return m_volume; }
    int getHVGXID()                     { return m_ID; }
    struct ssbo_mesh getSSBOData();

    // properties setters
    void setColor(QVector4D color)      {  m_color = color; }
    void setCenter(QVector4D center);
    void setAstPoint(QVector4D ast_point);
    void setVolume(int volume)          { m_volume = volume; }
    void setParentID(Object *parent)    { qDebug() << "Adding " << parent->getHVGXID() << " as parent to " <<  m_ID; m_parent == parent; }
    void addChild(Object *child)        { qDebug() << "Adding " << child->getHVGXID() << " as the " << m_children.size() << "th child to " <<  m_ID; m_children.push_back(child); }
    // skeleton management
    void addSkeletonNode(QVector3D coords);
    void addSkeletonPoint(QVector3D coords);
    void addSkeletonBranch(SkeletonBranch *branch);
    Skeleton* getSkeleton()            { return m_skeleton; }

    void setSkeletonOffset(int offset) { m_skeleton->setIndexOffset(offset);}
    int  getSkeletonOffset()           { return m_skeleton->getIndexOffset(); }
    void setNodeIdx(int node_index)    { m_nodeIndx = node_index ; }
    int  getNodeIdx()                  { return m_nodeIndx; }


private:
    std::string                             m_name;
    int                                     m_ID; // hvgx

    int                                     m_nodeIndx;

    // object properties
    Object_t                                m_object_t;     /* object type */
    int                                     m_volume;
    int                                     m_function;     // -1:not applicable, 0:ex, 1:in, 3:unknown

    Object                                  *m_parent;    // NULL if none
    std::vector<Object*>                    m_children;   // axon-> bouton, den->spine

    QVector4D                               m_center;
    // closest point from astrocyte skeleton to this object so we can project the object on skeleton and be part of it
    QVector4D                               m_ast_point;
    QVector4D                               m_color;

    Skeleton                                *m_skeleton;

    // indices to access the global mesh vertices defined in mesh
    std::vector<GLuint>                     m_meshIndices;

};

#endif // OBJECT_H
