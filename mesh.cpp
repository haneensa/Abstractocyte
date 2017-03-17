#include "mesh.h"

Mesh::Mesh()
{
	verticesList.reserve(9000000);		   //v: 8,123,031
	m_faces.reserve(16500000);			   //f: 16,253,222
	m_typeVertexList[1].reserve(300000);   //mitos.v: 270,303
	m_typeVertexList[2].reserve(2900000);  //axons.v: 2,858,984
	m_typeVertexList[3].reserve(1400000);  //bouts.v: 1,300,846
	m_typeVertexList[4].reserve(14500000); //dends.v: 14,449,558
	m_typeVertexList[5].reserve(750000);   //spine.v: 696,496
	m_typeVertexList[6].reserve(1300000);  //astro.v: 1,246,096
	m_typeVertexList[7].reserve(350000);   //synps.v: 300,748
}


int Mesh::addVertex(struct VertexData vdata, Object_t type)
{
    int idx = verticesList.size();
	vdata.index = idx;
    verticesList.push_back(vdata);
	
	int type_idx = static_cast<int>(type);
	//std::vector<VertexData*>* typeVector = (m_typeVertexList[type_idx]);
	m_typeVertexList[type_idx].push_back(verticesList.data() + idx);
    return idx;
}

bool Mesh::isValidFaces(int f1, int f2, int f3)
{
    if (    f1 > verticesList.size()
            || f2 > verticesList.size()
            || f3 > verticesList.size()  ) {
        qDebug() << "Error in obj file, verticesList size: " << verticesList.size();
        return false;
    }

    return true;
}

void Mesh::allocateVerticesVBO(QOpenGLBuffer vbo_mesh)
{
    vbo_mesh.allocate(verticesList.data(), verticesList.size() * sizeof(VertexData));
}

void Mesh::addFace(int index1, int index2, int index3)
{
    struct face f;
    f.v[0] = index1;
    f.v[1] = index2;
    f.v[2] = index3;

    int face_index = m_faces.size();
    m_faces.push_back(f);

    m_vertexFaces[index1].push_back(face_index);
    m_vertexFaces[index2].push_back(face_index);
    m_vertexFaces[index3].push_back(face_index);
}

void Mesh::getVertexNeighbors(int v_index, std::set< int > &neighs)
{
    // Use 'mesh' to walk the connectivity.
    std::vector<int>  faces_list = m_vertexFaces[v_index];
    for (int i = 0; i < faces_list.size(); ++i) {
        struct face f = m_faces[faces_list[i]];
        for (int j = 0; j < 3; ++j) {
            if (f.v[j] != v_index) {
                neighs.insert(f.v[j]);
            }
        }
    }
}

