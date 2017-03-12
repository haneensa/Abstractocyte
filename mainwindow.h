#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class GLWidget;
class MousePad;
namespace Clustering {
	class GlycogenCluster;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT // for signals and slots

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

	GLWidget* getGLWidget();
	MousePad* getMousePad();

public slots:

	void on_glycogenVisibilityCheckBox_toggled(bool);
	void on_clusterButton_clicked();


private:
    Ui::MainWindow *mainwindow_ui;
	std::map<int, Clustering::GlycogenCluster*>* m_clusters;


};

#endif // MAINWINDOW_H
