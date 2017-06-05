#ifndef INPUTFORM_H
#define INPUTFORM_H

#include <QWidget>
#include <QDialog>

namespace Ui {
class InputForm;
}

class InputForm : public QDialog
{
    Q_OBJECT

public:
    explicit InputForm(QWidget *parent = 0);
    ~InputForm();
    QString getInputFilesPath() { return m_filesPath; }

protected slots:
    void on_pushButton_path_clicked();
    void on_pushButton_submit_clicked();

signals:
    void signal_filesPath(QString);

private:
    Ui::InputForm *ui;
    QString m_filesPath;
};

#endif // INPUTFORM_H
