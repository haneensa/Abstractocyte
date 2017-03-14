#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QTreeWidgetItem>
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
	//------------- glycogen clustering ----------------------------
	void on_clusterButton_clicked();
	//void on_glycogenClustersTreeWidget_itemClicked(QTreeWidgetItem* item, int column);
	//void on_glycogenClustersTreeWidget_itemActivated(QTreeWidgetItem* item, int column);
	void on_glycogenClustersTreeWidget_itemChanged(QTreeWidgetItem* item, int column);
	void on_glycogenClustersTreeWidget_itemSelectionChanged();

	//------------- glycogen mapping -------------------------------
	void on_mapGlycogenGranulesButton_clicked();
	void on_mapGlycogenClustersButton_clicked();

private:
    Ui::MainWindow *mainwindow_ui;
	std::map<int, Clustering::GlycogenCluster*>* m_clusters;
	QTreeWidgetItem* m_currentSelectedCluster;

};

#endif // MAINWINDOW_H
