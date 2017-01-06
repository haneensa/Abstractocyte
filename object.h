#ifndef OBJECT_H
#define OBJECT_H

#include <QVector3D>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

enum class Object_t { AXON, DENDRITE, BOUTON, SPINE, MITO, SYNAPSE, ASTROCYTE, GLYCOGEN };

class Object
{
public:
    Object(std::string name);
    ~Object();
    void add_vertex(QVector3D vertex);
    std::vector<QVector3D> getVertices();
    Object_t getObjectType(std::string name);
    bool update_vbo();
    bool allocate_vbo(QOpenGLShaderProgram  *program_mesh);
    void draw();
    std::string getName();
    int getSize();
    QVector4D getColor();
    void setColor(QVector4D color);

private:
    Object_t                m_object_t;     /* object type */
    std::string             m_name;
    int                     m_ID;
    std::vector<QVector3D>  m_vertices;
    QVector4D               m_color;
    // either one per mesh or one for the whole object

    // store skeleton nodes and edges here. graph? skeleton?
    // one graph for all skeletons?
};

#endif // OBJECT_H
