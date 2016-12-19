#include "mainwindow.h"
#include <QApplication>
#include <QGLFormat>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);

    MainWindow window;
    window.show();

    return app.exec();
}
