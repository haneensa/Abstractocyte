#ifndef GLYCOGEN_H
#define GLYCOGEN_H

#include <QVector3D>

struct VertexData;

class Glycogen
{
public:
    Glycogen(int ID, std::string name, QVector3D center, float radius);
    int			getID()         { return m_ID; }
    QVector3D	getCenter() { return m_center; }
	VertexData*  getVertexData() { return &m_center2; }
	float		getRadius() { return m_radius; }
	int			getClusterID() { return m_clusterID; }
	float		getVolume();
	float		x() const;
	float		y() const;
	float		z() const;

	void setClusterID(int clusterID);
	void setIndex(int index);

	int		  getState();
	void      setState(int state);

protected:
    int             m_ID;
    std::string     m_name;
    QVector3D       m_center;
	VertexData		m_center2; //hack to be used with octree
	float           m_radius;
	int				m_clusterID;

};

#endif // GLYCOGEN_H
