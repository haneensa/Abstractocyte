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

    // OpenGL initialization
    int getSkeletonPointsSize();
    int getMeshIndicesSize();
    Mesh* getMeshPointer();

    bool filterByType(Object *object_p );


protected:
    // store all vertices of the mesh.
    // unique vertices, faces to index them.
    Mesh                                *m_mesh;

    int                                 m_skeleton_points_size;
    int                                 m_indices_size;
    int                                 m_vertex_offset;
    int                                 m_limit;

    std::map<int, Object*>              m_objects;

    // graph related data
    std::vector<QVector2D>              neurites_neurite_edge;
};

#endif // OBJECTMANAGER_H
