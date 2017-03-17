#include "mesh.h"
#include <QVector4D>

#include "glycogen.h"

#define PI 3.14159265358979f

Glycogen::Glycogen(int ID, std::string name, QVector3D center, float radius)
{
    m_ID = ID;
    m_name = name;
    m_center = center/5.0;
	m_radius = radius;
	m_clusterID = 0;

	//use mesh_vertex for center point and ID of glycogen
	m_center2.mesh_vertex.setX( m_center.x());
	m_center2.mesh_vertex.setY( m_center.y());
	m_center2.mesh_vertex.setZ( m_center.z());
	m_center2.mesh_vertex.setW( ID );

	//m_center2.isGlycogen = true;

	//use skeleton_vertex for other info
	m_center2.skeleton_vertex.setX( radius );
	m_center2.skeleton_vertex.setY( (4.0f/3.0f)* std::pow(radius,3)*PI ); //volume of sphere
	m_center2.skeleton_vertex.setZ( m_clusterID );
	m_center2.skeleton_vertex.setW( 1 ); //0 invisible //1 visible //2 selected
}

//----------------------------------------------------------------------------
//
void Glycogen::setClusterID(int clusterID)
{
	m_clusterID = clusterID;
	m_center2.skeleton_vertex.setZ(m_clusterID);
}

//----------------------------------------------------------------------------
//
void Glycogen::setIndex(int index)
{
	m_center2.index = index;
}

//----------------------------------------------------------------------------
//
float Glycogen::getVolume()
{
	return m_center2.skeleton_vertex.y();
}

//----------------------------------------------------------------------------
// 0: invisible 1: visible 2: selected
int Glycogen::getState()
{
	return (int)m_center2.skeleton_vertex.w();
}

//----------------------------------------------------------------------------
//
void Glycogen::setState(int state)
{
	if (state > 2 || state < 0)
		return;

	m_center2.skeleton_vertex.setW(state);
}

//----------------------------------------------------------------------------
//
float Glycogen::x() const { return m_center2.x(); }
float Glycogen::y() const { return m_center2.y(); }
float Glycogen::z() const { return m_center2.z(); }