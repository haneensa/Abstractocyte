#include "mesh.h"
#include <QVector4D>

#include "glycogen.h"

Glycogen::Glycogen(int ID, std::string name, QVector3D center, float diameter)
{
    m_ID = ID;
    m_name = name;
    m_center = center/5.0;
    m_diameter = diameter;
	m_clusterID = 0;

	//use mesh_vertex for center point and ID of glycogen
	m_center2.mesh_vertex.setX( m_center.x());
	m_center2.mesh_vertex.setY( m_center.y());
	m_center2.mesh_vertex.setZ( m_center.z());
	m_center2.mesh_vertex.setW( ID );

	m_center2.isGlycogen = true;

	//use skeleton_vertex for other info
	m_center2.skeleton_vertex.setX( diameter );

}

//----------------------------------------------------------------------------
//
void Glycogen::setClusterID(int clusterID)
{
	m_clusterID = clusterID;
}

//----------------------------------------------------------------------------
//
void Glycogen::setIndex(int index)
{
	m_center2.index = index;
}

//----------------------------------------------------------------------------
//
float Glycogen::x() const { return m_center2.x(); }
float Glycogen::y() const { return m_center2.y(); }
float Glycogen::z() const { return m_center2.z(); }