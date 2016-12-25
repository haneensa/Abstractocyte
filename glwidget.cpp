// todo:    1- render 3d segmentation
//          2- render the skeleton

#include "glwidget.h"
#include <iostream>
#include <QDebug>
#include <QtGui/QImage>
#include <QPainter>
#include <unordered_map>
#include <array>


//---------------------------------------------------------------------------
// @brief code taken from here http://jeffreystedfast.blogspot.com/2008/06/calculating-nearest-power-of-2.html
// @param _num the number we wish to get the nearest power from
// OpenGL needs textures to be in powers of two, this function will get the
// nearest power of two to the current value passed in
//---------------------------------------------------------------------------
unsigned int nearestPowerOfTwo ( unsigned int _num )
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

void GLWidget::initText( const QFont &_f  )
{
    QFontMetrics metric(_f);
    int fontHeight = metric.height();

    qDebug() << "font height: " << fontHeight;

    // todo: initialize text shader

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
        //  s0/t0  ---- s1,t0
        //         |\ |
        //         | \|
        //  s0,t1  ---- s1,t1
        // each quad will have the same s0 and the range s0-s1 == 0.0 -> 1.0
        float s0 = 0.0;
        // scale text-cord from 0-1 based on the coverage of the glyph
        float s1 = width * 1.0f / widthPow2;
        float t0 = 0.0f;
        float t1 = metric.height() * -1.0f / heightPow2;

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

        QString filename = ".png";
        filename.prepend(c);
        qDebug() << filename;

        /* debug
        if (finalImage.save(filename))
            qDebug() << filename;
        else
            qDebug() <<"ERROR: " << filename;
        */

        // convert QImage to format suitable for opengl
        finalImage = finalImage.convertToFormat(QImage::Format_ARGB32_Premultiplied);

        // create opengl texture ID and bind it to make it active

        glGenTextures(1, &fc.textureID);
        glBindTexture(GL_TEXTURE_2D, fc.textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);

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

        // the image in RGBA format and unsigned byte load it ready for later
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthTexture, heightTexture,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, data.get());
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

            // load values for triable 1
            d[0].x = 0;
            d[0].y = 0;
            d[0].u = s0;
            d[0].v = t0;

            d[1].x = fc.width;
            d[1].y = 0;
            d[1].u = s1;
            d[1].v = t0;

            d[2].x = 0;
            d[2].y = fontHeight;
            d[2].u = s0;
            d[2].v = t1;

            d[3].x = 0;
            d[3].y = fontHeight;
            d[3].u = s0;
            d[3].v = t1;

            d[4].x = fc.width;
            d[4].y = 0;
            d[4].u = s1;
            d[4].v = t0;

            d[5].x = fc.width;
            d[5].y = fontHeight;
            d[5].u = s1;
            d[5].v = t1;

            // create VAO to store the data
            QOpenGLVertexArrayObject   vao;
            vao.create();
            vao.bind();
            // set the vertex data (2 for x, y 2 for u, v)
            QOpenGLBuffer               vbo;
            vbo.create();
            vbo.setUsagePattern( QOpenGLBuffer::StaticDraw);
            if ( !vbo.bind() ) {
                qDebug() << "Could not bind vertex buffer to the context.";
                return;
            }


            vbo.allocate(d, 6 * sizeof(textVertData));

            // set attribute pointer to be 0 (for shader), vertIn
//            quintptr offset = 0;
//            int vertIn = program->attributeLocation("vertIn");
//            program->enableAttributeArray(vertIn);
//            program->setAttributeBuffer(vertIn, GL_FLOAT, offset, 1, sizeof(VertexData));

//            // set attribute pointer to be 1, for uv_coords
//            int uv_coords = program->attributeLocation("uv_coords");
//            program->enableAttributeArray(uv_coords);
//            offset += (2 * sizeof(float));
//            program->setAttributeBuffer(uv_coords, GL_INT, offset, 1, sizeof(VertexData));

            vbo.release();
            vao.release();

            // store the vao pointer for later use in the draw method
            fc.vao = &vao;
            widthVAO[width] = fc.vao;
        } else {
            fc.vao = widthVAO[width];
        }

        // add the element to the map
        qDebug() << "c: " << c << " width: " << fc.width;
        m_characters[c] = fc;
    }

    qDebug() << "Created " << widthVAO.size() << " unique billboards";

}

void GLWidget::renderText( float x, float y, const QString &text )
{
    // todo tomorrow!
}

GLWidget::GLWidget(QWidget *parent)
    :  QOpenGLWidget(parent)
{

}

GLWidget::~GLWidget()
{
    qDebug() << "~GLWidget()";
    // our dtor should clear out the textures and remove the VAO's
    for( auto &m : m_characters)
    {
        qDebug() << m.width;
        glDeleteTextures(1,&m.textureID);
    }

}

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    const QFont f;
    initText(f);

}

void GLWidget::paintGL()
{
    // paint the text here!
}

void GLWidget::resizeGL(int w, int h)
{

}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
}

void GLWidget::getSliderX(int value)
{
    qDebug() << value;
}

void GLWidget::getSliderY(int value)
{
    qDebug() << value;
}
