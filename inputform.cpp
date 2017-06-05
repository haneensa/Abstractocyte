#include "inputform.h"
#include "ui_inputform.h"
#include <QFileDialog>
#include <qDebug>

InputForm::InputForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InputForm)
{
    ui->setupUi(this);
}

InputForm::~InputForm()
{
    delete ui;
}

void InputForm::on_pushButton_path_clicked()
{
    // display file open dialog
    QString s = QFileDialog::getExistingDirectory(
                this, "Open Directory", "/home", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );

    if ( s=="" ) {
        return;
    }

    QByteArray ba = s.toLatin1();
    ui->lineEdit->setText(ba.data());
    m_filesPath = ba;
    // "Todo: list the files in the folder and display if they are good fits or not, then pass dir to mainwindow"
}

void InputForm::on_pushButton_submit_clicked()
{
    signal_filesPath(m_filesPath);
    this->accept();
}
