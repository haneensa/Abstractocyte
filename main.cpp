#include "mainwindow.h"
#include <QApplication>
#include <QGLFormat>
#include <QResource>
#include <QDebug>

int main(int argc, char *argv[])
{
    if (!QResource::registerResource("external_resources.rcc")) {
          qDebug() << "Could not register resources.";
          return 0;
    }

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
