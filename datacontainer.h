#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

// file manipulations
#include <QString>
#include <QFile>
#include <QXmlStreamReader>

#include "object.h"
#include "mesh.h"

class DataContainer
{
public:
    DataContainer();
    ~DataContainer();

    bool importXML(QString path);
    void parseObject(QXmlStreamReader &xml, Object *obj);
    void parseMesh(QXmlStreamReader &xml, Object *obj);
    void parseSkeleton(QXmlStreamReader &xml, Object *obj);
    void parseSkeletonNodes(QXmlStreamReader &xml, Object *obj);
    void parseBranch(QXmlStreamReader &xml, Object *obj);
    void parseConnGraph(QXmlStreamReader &xml);
    void parseSkeletonPoints(QXmlStreamReader &xml, Object *obj);

    // graph related function
    std::map<int, Object*>  getObjectsMap();
    std::vector<QVector2D> getNeuritesEdges();

    int getSkeletonPointsSize();
    int getMeshIndicesSize();
    Mesh* getMeshPointer();

    Object_t getObjectTypeByID(int hvgxID);

    std::vector<int> getObjectsIDsByType(Object_t type);

protected:
    // store all vertices of the mesh.
    // unique vertices, faces to index them.
    Mesh                                    *m_mesh;

    Object                                  *m_curParent;
    int                                     m_skeleton_points_size;
    int                                     m_indices_size;
    int                                     m_vertex_offset;
    int                                     m_limit;

    std::map<int, Object*>                  m_objects;

    // for each type, make a list of their IDs
    std::map< Object_t, std::vector<int> >  m_objectsIDsByType;

    // graph related data
    std::vector<QVector2D>                  neurites_neurite_edge;
};

#endif // OBJECTMANAGER_H
