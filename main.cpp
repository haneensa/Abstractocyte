#include <QApplication>
#include <QGLFormat>

#include "mainwindow.h"
#include "inputform.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QSurfaceFormat format;
    format.setVersion(4, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);

    InputForm *input_form;
    input_form = new InputForm(0);
    input_form->exec();

    if (input_form->exec() == InputForm::Accepted) {
        MainWindow *window = new MainWindow(0, input_form);

        window->show();
    }

    return app.exec();
}
