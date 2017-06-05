#include "mainwindow.h"
#include <QApplication>
#include <QGLFormat>
#include <QResource>
#include <QDebug>

#include "inputform.h"

int main(int argc, char *argv[])
{
    if (!QResource::registerResource("input/external_resources.rcc")) {
          qDebug() << "Could not register resources.";
          return 0;
    }

    QApplication app(argc, argv);

    QSurfaceFormat format;
    format.setVersion(4, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);

    InputForm input_form;
    input_form.exec();


    MainWindow *window = new MainWindow(0, input_form.getInputFilesPath());

    // 1) load the original xml file -> sae the compressed dat files
    // 2) load the light weight xml file + the dat files

    // compressed faces and vertices
    // const char* filename1 = "input/astro_data_fv.dat";
    // const char* filename2 = "input/neurites_data_fv.dat";

    // this is a must
    // importXML("://pipeline_scripts/output/m3_astrocyte.xml");
    // importXML("://pipeline_scripts/output/m3_neurites.xml");

    // for the normals:
    // 1) compute the normals
    // 2) save the normals as dat file
    // 3) use the saved normals

    // for splatting:
    // 1) "input/mask_745_sigma3.raw",
    // 2) "input/astrocytic_mitochondria_s5.raw",
    // 3) "input/Neuronic_mitochondria_binary_s5.raw"
    // 4) "input/mask_glycogen_sig3_blured20.raw"

    window->show();

    return app.exec();
}
