#ifndef INPUTFORM_H
#define INPUTFORM_H

#include <QWidget>
#include <QDialog>

namespace Ui {
class InputForm;
}

enum class LoadData_t { ALL, ASTRO, NEURITES };
enum class LoadFile_t { DUMP_ASTRO, DUMP_NEURITES, LOAD_MESH_NO_VERTEX, LOAD_MESH_W_VERTEX };
enum class Normals_t { DUMP_NORMAL, LOAD_NORMAL, NO_NORMALS };

struct input_files {
    QString HVGX_metadata;
    QString proximity_astro;
    QString proximity_astro_mito;
    QString proximity_neu_mito;
    QString proximity_glycogen;
    QString xml_astro;
    QString xml_neurites;
    QString extra_files_path;
};

class InputForm : public QDialog
{
    Q_OBJECT

public:
    explicit InputForm(QWidget *parent = 0);
    ~InputForm();
    struct input_files getInputFilesPath()  { return m_input_files; }
    LoadData_t getLoadDataType()            { return m_load_data; }
    LoadFile_t getLoadFileType()            { return m_loadType; }
    int getObjectsCountLimit()              { return m_count_limit; }

protected slots:
    void on_pushButton_extra_files_path_clicked();
    void on_pushButton_metadata_clicked();
    void on_pushButton_proximity_astrocyte_clicked();
    void on_pushButton_proximity_astro_mito_clicked();
    void on_pushButton_proximity_gly_clicked();
    void on_pushButton_proximity_neu_mito_clicked();
    void on_pushButton_xml_astrocyte_clicked();
    void on_pushButton_xml_neurites_clicked();

    void on_pushButton_submit_clicked();

    void on_radioButton_load_neurites_toggled(bool);
    void on_radioButton_load_astrocyte_toggled(bool);
    void on_radioButton_load_all_toggled(bool);

    void on_checkBox_astrocyte_extrafiles_toggled(bool);
    void on_checkBox_neurites_extrafiles_toggled(bool);

    void on_checkBox_lightXML_toggled(bool);

    void on_spinBox_limit_valueChanged(int);

signals:
    void signal_filesPath(struct input_files);

private:
    Ui::InputForm       *ui;
    struct input_files  m_input_files;

    LoadData_t          m_load_data;
    LoadFile_t          m_loadType;
    Normals_t           m_normals_t;

    int                 m_count_limit;
};

#endif // INPUTFORM_H
