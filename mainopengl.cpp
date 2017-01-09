#include "mainopengl.h"
#include <QResource>

MainOpenGL::MainOpenGL()
    : flag(false)
{
    qDebug() << "MainOpenGL";
}

MainOpenGL::~MainOpenGL()
{
    qDebug() << "Delete MainOpenGL";
    if (flag == true) {
        delete m_program_text;
        // our dtor should clear out the textures and remove the VAO's
        for( auto &m : m_characters)
        {
            glDeleteTextures(1,&m.textureID);
            m.vao->destroy();
        }
    }
}


bool MainOpenGL::initShader(QOpenGLShaderProgram *program, const char *vshader, const char *gshader, const char *fshader)
{
    qDebug() << "Initializing shaders";

    // Compile vertex shader
    if (!program->addShaderFromSourceFile(QOpenGLShader::Vertex, vshader))
    {
        qDebug() << "Error in vertex shader:" << program->log();
        return false;
    }

    // Compile geometry shader
    if (!program->addShaderFromSourceFile(QOpenGLShader::Geometry, gshader))
    {
        qDebug() << "Error in Geometry shader:" << program->log();
        return false;
    }

    // Compile fragment shader
    if (!program->addShaderFromSourceFile(QOpenGLShader::Fragment, fshader))
    {
        qDebug() << "Error in fragment shader:" << program->log();
        return false;
    }


    // Link shader pipeline
    if (!program->link())
    {
        qDebug() << "Error in linking shader program:" << program->log();
        return false;
    }


    return true;
}

bool MainOpenGL::initShader(GLuint program, const char *vshader, const char *gshader, const char *fshader)
{
    qDebug() << "Initializing shaders";
    QResource vs_resource(vshader);
    const char* vs_data = (const char *)vs_resource.data();
    GLint compile_ok = GL_FALSE, link_ok = GL_FALSE;
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vs_data, NULL);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &compile_ok);
    if (!compile_ok) {
        qDebug() << "Error in vertex shader";
        return false;
    }

    QResource gs_resource(gshader);
    const char* gs_data = (const char *)gs_resource.data();
    GLuint gs = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(gs, 1, &gs_data, NULL);
    glCompileShader(gs);
    glGetShaderiv(gs, GL_COMPILE_STATUS, &compile_ok);
    if (!compile_ok) {
        qDebug() << "Error in geometry shader";
        return false;
    }

    QResource fs_resource(fshader);
    const char* fs_data = (const char *)fs_resource.data();
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fs_data, NULL);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &compile_ok);
    if (!compile_ok) {
        qDebug() << "Error in fragment shader";
        return false;
    }

    // link all together
    glAttachShader(program, vs);
    glAttachShader(program, gs);
    glAttachShader(program, fs);

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
        qDebug() << "Error in glLinkProgram";
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
// @brief code taken from here http://jeffreystedfast.blogspot.com/2008/06/calculating-nearest-power-of-2.html
// @param _num the number we wish to get the nearest power from
// OpenGL needs textures to be in powers of two, this function will get the
// nearest power of two to the current value passed in
//---------------------------------------------------------------------------
unsigned int MainOpenGL::nearestPowerOfTwo ( unsigned int _num )
{
    unsigned int j, k;
    (j = _num & 0xFFFF0000) || (j = _num);
    (k = j & 0xFF00FF00) || (k = j);
    (j = k & 0xF0F0F0F0) || (j = k);
    (k = j & 0xCCCCCCCC) || (k = j);
    (j = k & 0xAAAAAAAA) || (j = k);
    return j << 1;
}
// end citation


void MainOpenGL::initText( const QFont &_f  )
{
    flag = true;
    m_program_text = new QOpenGLShaderProgram();
    bool res = initShader(m_program_text, ":/shaders/text.vert", ":/shaders/text.geom", ":/shaders/text.frag");
    if(res == false)
        return;

    QFontMetrics metric(_f);
    int fontHeight = metric.height();

    // loop over all the basic keyboad chars from space to ~
    const static char startChar = ' ';
    const static char endChar = '~';

    // OpenGL cards need textures to bein powers of 2
    unsigned int heightPow2 = nearestPowerOfTwo(fontHeight);

    // create texture/billboard for each font
    // the same height, could have different widths
    // some same width -> create vao only if we dont have one of the same width
    std::unordered_map<int, QOpenGLVertexArrayObject* > widthVAO;

    for (char c = startChar; c <= endChar; ++c) {
        QChar ch(c);
        FontChar fc;
        // get the width of the font and calculate the ^2 size
        int width = metric.width(c);
        unsigned int widthPow2 = nearestPowerOfTwo(width);

        // set the texture coordinates for our quad
        // triangle billboard with text-cords as:
        //  s0,t0  ---- s1,t0
        //         |\ |
        //         | \|
        //  s0,t1  ---- s1,t1
        // each quad will have the same s0 and the range s0-s1 == 0.0 -> 1.0
        float s0 = 0.0;
        // scale text-cord from 0-1 based on the coverage of the glyph
        float s1 = width * 1.0f / widthPow2;
        float t1 = 0.0f;
        float t0 = metric.height() * -1.0f/heightPow2;

        // store the width for later drawing
        fc.width = width;

        // to store the texture
        // draw into this Qimage, save this in opengl format and load as texture
        QImage finalImage(nearestPowerOfTwo(width), nearestPowerOfTwo(fontHeight), QImage::Format_ARGB32);
        finalImage.fill(Qt::transparent);

        // draw into the image and create our billboards
        QPainter painter;
        painter.begin(&finalImage);
        painter.setRenderHints(QPainter::HighQualityAntialiasing | QPainter::TextAntialiasing);
        painter.setFont(_f);
        painter.setPen(Qt::black);
        painter.drawText(0, metric.ascent(), QString(c));
        painter.end();

        // convert QImage to format suitable for opengl
        finalImage = finalImage.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        // set rgba image data
        int widthTexture = finalImage.width();
        int heightTexture = finalImage.height();

        std::unique_ptr<unsigned char[]> data(new unsigned char[widthTexture * heightTexture * 4]);
        unsigned int index = 0;
        QRgb color;
        for (int y = heightTexture - 1; y > 0; --y) {
            for (int x = 0; x < widthTexture; ++x) {
                color = finalImage.pixel(x, y);
                data[index++] = static_cast<unsigned char>(qRed(color));
                data[index++] = static_cast<unsigned char>(qGreen(color));
                data[index++] = static_cast<unsigned char>(qBlue(color));
                data[index++] = static_cast<unsigned char>(qAlpha(color));
            }
        }

        // create opengl texture ID and bind it to make it active
        glGenTextures(1, &fc.textureID);
        glBindTexture(GL_TEXTURE_2D, fc.textureID);

        // the image in RGBA format and unsigned byte load it ready for later
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthTexture, heightTexture,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, data.get());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);

        // check if billboard exist with this width
        if (widthVAO.find(width) == widthVAO.end()) {
            // this structure is used by the VAO to store the data to be uploaded
            // for drawing the quad
            struct textVertData {
                float x;
                float y;
                float u;
                float v;
            };

            // we are creating a billboad with two triangles so we only need the
            // 6 verts
            textVertData* d = new textVertData[6];

            // load values for triangle 1
            d[0].x = 0.0;
            d[0].y = 0.0;
            d[0].u = s0;
            d[0].v = t0;

            d[1].x = fc.width;
            d[1].y = 0.0;
            d[1].u = s1;
            d[1].v = t0;

            d[2].x = 0.0;
            d[2].y = fontHeight;
            d[2].u = s0;
            d[2].v = t1;

            // load values for triangle two
            d[3].x = 0.0;
            d[3].y = fontHeight;
            d[3].u = s0;
            d[3].v = t1;

            d[4].x = fc.width;
            d[4].y = 0.0;
            d[4].u = s1;
            d[4].v = t0;

            d[5].x = fc.width;
            d[5].y = fontHeight;
            d[5].u = s1;
            d[5].v = t1;

            // create VAO to store the data
            QOpenGLVertexArrayObject   *vao = new QOpenGLVertexArrayObject;
            vao->create();
            vao->bind();
            // set the vertex data (2 for x, y 2 for u, v)
            QOpenGLBuffer               vbo(QOpenGLBuffer::VertexBuffer);
            vbo.create();
            vbo.setUsagePattern( QOpenGLBuffer::StaticDraw);
            if ( !vbo.bind() ) {
                qDebug() << "Could not bind vertex buffer to the context.";
                return;
            }

            vbo.allocate(d, 6 * sizeof(textVertData));

            // set attribute pointer to be 0 (for shader), vertIn
            quintptr offset = 0;
            int vertIn = m_program_text->attributeLocation("vertIn");
            m_program_text->enableAttributeArray(vertIn);
            m_program_text->setAttributeBuffer(vertIn, GL_FLOAT, offset, 2, sizeof(textVertData));

            // set attribute pointer to be 1, for uv_coords
            int uv_coords = m_program_text->attributeLocation("uv_coords");
            m_program_text->enableAttributeArray(uv_coords);
            offset += (2 * sizeof(float));
            m_program_text->setAttributeBuffer(uv_coords, GL_FLOAT, offset, 2, sizeof(textVertData));

            vbo.release();
            vao->release();

            // store the vao pointer for later use in the draw method
            fc.vao = vao;
            widthVAO[width] = fc.vao;
        } else {
            fc.vao = widthVAO[width];
        }

        // add the element to the map
        m_characters[c] = fc;
    }

    qDebug() << "Created " << widthVAO.size() << " unique billboards";
}



void MainOpenGL::renderText( float x, float y, float scaleX, float scaleY, const QString &text  )
{
    glActiveTexture(GL_TEXTURE0);

    m_program_text->bind();
    m_program_text->setUniformValue("ypos",  y);
    m_program_text->setUniformValue("scaleX",  scaleX);
    m_program_text->setUniformValue("scaleY",  scaleY);

    m_projection.setToIdentity();
    m_projection.ortho( -1.0f,  1.0f, -1.0f, 1.0f, -10.0, 10.0 );
    m_program_text->setUniformValue("pMatrix", m_projection);

    // for fonts to renders correctly
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // loop for each of the car and draw our billboard
    int textLength = text.length();
    for (int i = 0; i < textLength; ++i) {
        // set the shader x position
        // we render a glyph by the width of the char
        FontChar f = m_characters[text[i].toLatin1()];
        f.vao->bind();
        m_program_text->setUniformValue("xpos",  x);
        // bind the pre generated texture
        glBindTexture(GL_TEXTURE_2D, f.textureID);
        m_program_text->setUniformValue("tex",  0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        f.vao->release();
        x += f.width;
    }

    m_program_text->release();
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

// change this to pass the objects vector (done)
// todo: make this efficient by writing the input as binary file and loading items at once
// todo: get the ID from hvgx and add it to the obj objects names -> used later to map skeleton to objects
// todo: combine skeleton and mesh information in one obj file
unsigned int MainOpenGL::loadOBJ(QString path, std::vector<Object*> & objects)
{
    qDebug() << "Func: loadVertices";
    unsigned int vertices_count = 0;

    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "Could not open the file for reading";
        return vertices_count;
    }

    QTextStream in(&file);
    QList<QByteArray> wordList;


    // temp containters
    std::vector< unsigned int > vertexIndices;
    std::vector< QVector3D > temp_vertices;
    bool flag_prev = false;
    float max_x, max_y, max_z;
    max_x = INT_MIN;
    max_y = INT_MIN;
    max_z = INT_MIN;

    float min_x, min_y, min_z;
    min_x = INT_MAX;
    min_y = INT_MAX;
    min_z = INT_MAX;
    std::string name;

    // load all vertices once -> should be fast
    // for each object "o", go through its faces, and substitute using vertices loaded at the start
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        wordList = line.split(' ');
        if (wordList[0] == "o") {
            if (flag_prev) {
                Object *obj = new Object(name);
                // indexing
                // for each vertex of each triangle
                qDebug() << "vertexIndices: " << vertexIndices.size();
                qDebug() << "temp_vertices: " << temp_vertices.size();
                vertices_count += vertexIndices.size();

                for ( unsigned int i = 0; i < vertexIndices.size(); ++i ) {
                    unsigned int vertexIndex = vertexIndices[i];
                    QVector3D vertex = temp_vertices[vertexIndex - 1];
                    obj->add_vertex(vertex);
                }
                qDebug() << "done vertexIndices: " << vertexIndices.size();
                QVector4D color = QVector4D(1.0, 0.0, 1.0, 1.0) ;
                obj->setColor(color);
                objects.push_back(obj);
                if (objects.size() > 2) {
                    break;
                }
            }
            name  = wordList[1].data();
            vertexIndices.clear();
            temp_vertices.clear();
            flag_prev = true;
        }
        else if (wordList[0]  == "v") {
            float x = atof(wordList[1].data());
            float y = atof(wordList[2].data());
            float z = atof(wordList[3].data());
            QVector3D vertex(x, y, z);

            if (x < min_x)
                min_x = x;
            if (y < min_y)
                min_y = y;
            if (z < min_z)
                min_z = z;

            if (x > max_x)
                max_x = x;
            if (y > max_y)
                max_y = y;
            if (z > max_z)
                max_z = z;

            temp_vertices.push_back(vertex);
        } else if (wordList[0]  == "f") {
            unsigned int f1_index = atoi(wordList[1].data());
            unsigned int f2_index = atoi(wordList[2].data());
            unsigned int f3_index = atoi(wordList[3].data());
            vertexIndices.push_back(f1_index);
            vertexIndices.push_back(f2_index);
            vertexIndices.push_back(f3_index);
        } else if (wordList[0] == "vn") {
            qDebug() << "To do compute the normals and read them from here";
        }
    }

    file.close();

    qDebug() << "Done Func: loadVertices";
    qDebug() << "MIN: " << min_x << " " << min_y << " " << min_z;
    qDebug() << "MAX: " << max_x << " " << max_y << " " << max_z;

    return vertices_count;
}


unsigned int MainOpenGL::loadSkeletonPoints(QString path,  std::vector<Object*> & objects, int flag)
{
    qDebug() << "Func: loadSkeletonPoints";
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "Could not open the file for reading";
        return 0;
    }

    QTextStream in(&file);
    QList<QByteArray> wordList;

    float max_x, max_y, max_z;
    max_x = INT_MIN;
    max_y = INT_MIN;
    max_z = INT_MIN;

    float min_x, min_y, min_z;
    min_x = INT_MAX;
    min_y = INT_MAX;
    min_z = INT_MAX;

    Object *obj = new Object("Skeleton");
    // Point ID, thickness, X Coord, Y Coord, Z Coord, Object ID
    unsigned int vertices_count = 0;
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        wordList = line.split(',');
        if (flag == 0) {
            float x = atof(wordList[2].data());
            float y = atof(wordList[3].data());
            float z = atof(wordList[4].data());
            obj->add_vertex(QVector3D(x, y, z));
            vertices_count++;
        } else {
            int pID = atoi(wordList[5].data());
            if (pID == 746 || pID == 745 || pID == 743 || pID == 742) {
//                qDebug() << pID;
            } else {
                continue;
            }

            float x = atof(wordList[2].data())/200.0;
            float y = atof(wordList[3].data())/200.0;
            float z = atof(wordList[4].data())/200.0;
            obj->add_vertex(QVector3D(x, y, z));
            vertices_count++;
        }
        // to color points, each point has to have color!
    }

    objects.push_back(obj);

    return vertices_count;
}
