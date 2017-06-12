#include "inputform.h"
#include "ui_inputform.h"
#include <QFileDialog>
#include <QDialog>
#include <qDebug>
#include <QTextBrowser>

InputForm::InputForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InputForm)
{
    ui->setupUi(this);

    m_load_data = LoadData_t::ALL;

    m_count_limit = 1000000;
    ui->spinBox_limit->setValue(m_count_limit);
    /* 0: extract input files names */
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
}

InputForm::~InputForm()
{
    delete ui;
}

void InputForm::on_pushButton_extra_files_path_clicked()
{
    // display file open dialog
    QString s = QFileDialog::getExistingDirectory(
                this, "Open Directory", "/home", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );

    if ( s=="" ) {
        return;
    }

    QByteArray folder_name = s.toLatin1();
    ui->lineEdit_extra_files_path->setText(folder_name.data());

    m_input_files.extra_files_path = folder_name;
}

void InputForm::on_pushButton_metadata_clicked()
{
    QString types("Metadata HVGX file (*.hvgx);;");

    // display file open dialog
    QString s = QFileDialog::getOpenFileName(
        this, "Choose an hvgx file to open", 0, types);

    if ( s=="" ) {
        return;
    }

    QByteArray file_name = s.toLatin1();
    ui->lineEdit_metadata->setText(file_name.data());

    m_input_files.HVGX_metadata = file_name;
}

void InputForm::on_pushButton_proximity_astrocyte_clicked()
{
    QString types("Raw file (*.raw);;");

    // display file open dialog
    QString s = QFileDialog::getOpenFileName(
        this, "Choose an raw file to open", 0, types);

    if ( s=="" ) {
        return;
    }

    QByteArray file_name = s.toLatin1();
    ui->lineEdit_proximity_astrocyte->setText(file_name.data());

    m_input_files.proximity_astro = file_name;
}

void InputForm::on_pushButton_proximity_astro_mito_clicked()
{
    QString types("Raw file (*.raw);;");

    // display file open dialog
    QString s = QFileDialog::getOpenFileName(
        this, "Choose an raw file to open", 0, types);

    if ( s=="" ) {
        return;
    }

    QByteArray file_name = s.toLatin1();
    ui->lineEdit_proximity_astro_mito->setText(file_name.data());

    m_input_files.proximity_astro_mito = file_name;
}

void InputForm::on_pushButton_proximity_gly_clicked()
{
    QString types("Raw file (*.raw);;");

    // display file open dialog
    QString s = QFileDialog::getOpenFileName(
        this, "Choose an raw file to open", 0, types);

    if ( s=="" ) {
        return;
    }

    QByteArray file_name = s.toLatin1();
    ui->lineEdit_proximity_gly->setText(file_name.data());

    m_input_files.proximity_glycogen = file_name;
}

void InputForm::on_pushButton_proximity_neu_mito_clicked()
{
    QString types("Raw file (*.raw);;");

    // display file open dialog
    QString s = QFileDialog::getOpenFileName(
        this, "Choose an raw file to open", 0, types);

    if ( s=="" ) {
        return;
    }

    QByteArray file_name = s.toLatin1();
    ui->lineEdit_proximity_neu_mito->setText(file_name.data());

    m_input_files.proximity_neu_mito = file_name;
}

void InputForm::on_pushButton_xml_astrocyte_clicked()
{
    QString types("XML file (*.xml);;");

    // display file open dialog
    QString s = QFileDialog::getOpenFileName(
        this, "Choose an xml file to open", 0, types);

    if ( s=="" ) {
        return;
    }

    QByteArray file_name = s.toLatin1();
    ui->lineEdit_xml_astrocyte->setText(file_name.data());

    m_input_files.xml_astro = file_name;
}

void InputForm::on_pushButton_xml_neurites_clicked()
{
    QString types("XML file (*.xml);;");

    // display file open dialog
    QString s = QFileDialog::getOpenFileName(
        this, "Choose an xml file to open", 0, types);

    if ( s=="" ) {
        return;
    }

    QByteArray file_name = s.toLatin1();
    ui->lineEdit_xml_neurites->setText(file_name.data());

    m_input_files.xml_neurites = file_name;
}

void InputForm::on_pushButton_submit_clicked()
{
    signal_filesPath(m_input_files);
    this->accept();
}

void InputForm::on_radioButton_load_neurites_toggled(bool val)
{
    if (!val)
        return;

    if (ui->checkBox_astrocyte_extrafiles->isChecked()) {
        bool oldState = ui->radioButton_load_astrocyte->blockSignals(true);
        ui->radioButton_load_astrocyte->setChecked(true);
        ui->radioButton_load_astrocyte->blockSignals(oldState);
        return;
    }

    m_load_data = LoadData_t::NEURITES;
}

void InputForm::on_radioButton_load_astrocyte_toggled(bool val)
{
    if (!val)
        return;

    if (ui->checkBox_neurites_extrafiles->isChecked()) {
        bool oldState = ui->radioButton_load_neurites->blockSignals(true);
        ui->radioButton_load_neurites->setChecked(true);
        ui->radioButton_load_neurites->blockSignals(oldState);
        return;
    }

    m_load_data = LoadData_t::ASTRO;
}

void InputForm::on_radioButton_load_all_toggled(bool val)
{
    if (!val)
        return;

    if (ui->checkBox_astrocyte_extrafiles->isChecked()) {
        bool oldState = ui->radioButton_load_astrocyte->blockSignals(true);
        ui->radioButton_load_astrocyte->setChecked(true);
        ui->radioButton_load_astrocyte->blockSignals(oldState);
        return;
    } else if (ui->checkBox_neurites_extrafiles->isChecked()) {
        bool oldState = ui->radioButton_load_neurites->blockSignals(true);
        ui->radioButton_load_neurites->setChecked(true);
        ui->radioButton_load_neurites->blockSignals(oldState);
        return;
    }

    m_load_data = LoadData_t::ALL;
}

void InputForm::on_checkBox_astrocyte_extrafiles_toggled(bool val)
{
    if (!val)
        return;

    bool oldState = ui->checkBox_neurites_extrafiles->blockSignals(true);
    ui->checkBox_neurites_extrafiles->setChecked(false);
    ui->checkBox_neurites_extrafiles->blockSignals(oldState);

    ui->radioButton_load_astrocyte->setChecked(true);
    ui->radioButton_load_all->setChecked(false);
    ui->radioButton_load_neurites->setChecked(false);

    m_loadType = LoadFile_t::DUMP_ASTRO;
}

void InputForm::on_checkBox_neurites_extrafiles_toggled(bool val)
{
    if (!val)
        return;
    bool oldState = ui->checkBox_astrocyte_extrafiles->blockSignals(true);
    ui->checkBox_astrocyte_extrafiles->setChecked(false);
    ui->checkBox_astrocyte_extrafiles->blockSignals(oldState);

    ui->radioButton_load_astrocyte->setChecked(false);
    ui->radioButton_load_all->setChecked(false);
    ui->radioButton_load_neurites->setChecked(true);


    m_loadType = LoadFile_t::DUMP_NEURITES;
}

void InputForm::on_checkBox_lightXML_toggled(bool val)
{
    if (val)
        m_loadType = LoadFile_t::LOAD_MESH_NO_VERTEX;
    else
        m_loadType = LoadFile_t::LOAD_MESH_W_VERTEX;
}

void InputForm::on_spinBox_limit_valueChanged(int limit)
{
    m_count_limit = limit;
}

void InputForm::on_pushButton_options_help_clicked()
{
    QDialog *dialog = new QDialog();
    QTextBrowser *textBrowser = new QTextBrowser;
    textBrowser->setSource(QUrl("qrc:/doc/options_help.html"));
    QVBoxLayout* mainLayout = new QVBoxLayout(dialog);
    mainLayout->addWidget(textBrowser);
    dialog->exec();
}

void InputForm::on_pushButton_extra_files_path_help_clicked()
{
    QDialog *dialog = new QDialog();
    QTextBrowser *textBrowser = new QTextBrowser;
    textBrowser->setSource(QUrl("qrc:/doc/extrafiles_path_help.html"));
    QVBoxLayout* mainLayout = new QVBoxLayout(dialog);
    mainLayout->addWidget(textBrowser);
    dialog->exec();
}

void InputForm::on_pushButton_metadata_help_clicked()
{
    QDialog *dialog = new QDialog();
    QTextBrowser *textBrowser = new QTextBrowser;
    textBrowser->setSource(QUrl("qrc:/doc/metadata_hvgx_help.html"));
    QVBoxLayout* mainLayout = new QVBoxLayout(dialog);
    mainLayout->addWidget(textBrowser);
    dialog->exec();
}

void InputForm::on_pushButton_xml_help_clicked()
{
    QDialog *dialog = new QDialog();
    QTextBrowser *textBrowser = new QTextBrowser;
    textBrowser->setSource(QUrl("qrc:/doc/xml_help.html"));
    QVBoxLayout* mainLayout = new QVBoxLayout(dialog);
    mainLayout->addWidget(textBrowser);
    dialog->exec();
}

void InputForm::on_pushButton_proximity_astrocyte_help_clicked()
{
    QDialog *dialog = new QDialog();
    QTextBrowser *textBrowser = new QTextBrowser;
    textBrowser->setSource(QUrl("qrc:/doc/proximity_astrocyte_help.html"));
    QVBoxLayout* mainLayout = new QVBoxLayout(dialog);
    mainLayout->addWidget(textBrowser);
    dialog->exec();
}

void InputForm::on_pushButton_proximity_astro_mito_help_clicked()
{
    QDialog *dialog = new QDialog();
    QTextBrowser *textBrowser = new QTextBrowser;
    textBrowser->setSource(QUrl("qrc:/doc/proximity_astro_mito_help.html"));
    QVBoxLayout* mainLayout = new QVBoxLayout(dialog);
    mainLayout->addWidget(textBrowser);
    dialog->exec();
}

void InputForm::on_pushButton_proximity_neu_mito_help_clicked()
{
    QDialog *dialog = new QDialog();
    QTextBrowser *textBrowser = new QTextBrowser;
    textBrowser->setSource(QUrl("qrc:/doc/proximity_neu_mito_help.html"));
    QVBoxLayout* mainLayout = new QVBoxLayout(dialog);
    mainLayout->addWidget(textBrowser);
    dialog->exec();
}

void InputForm::on_pushButton_proximity_gly_help_clicked()
{
    QDialog *dialog = new QDialog();
    QTextBrowser *textBrowser = new QTextBrowser;
    textBrowser->setSource(QUrl("qrc:/doc/proximity_gly_help.html"));
    QVBoxLayout* mainLayout = new QVBoxLayout(dialog);
    mainLayout->addWidget(textBrowser);
    dialog->exec();
}
