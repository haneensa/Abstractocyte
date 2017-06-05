#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QTreeWidgetItem>
#include <QMainWindow>
#include <QStandardItemModel>
#include <QListWidgetItem>
#include "inputform.h"

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
    explicit MainWindow(QWidget *parent = 0, QString input_path = "");
    ~MainWindow();

    void setupSignalsNSlots();

	GLWidget* getGLWidget();
	MousePad* getMousePad();

public slots:
	void on_glycogenVisibilityCheckBox_toggled(bool);
	//------------- glycogen clustering ----------------------------
    void on_clusterButton_clicked();
	void on_glycogenClustersTreeWidget_itemChanged(QTreeWidgetItem* item, int column);
	void on_glycogenClustersTreeWidget_itemSelectionChanged();

	//------------- glycogen mapping -------------------------------
	void on_mapGlycogenGranulesButton_clicked();
	void on_mapGlycogenClustersButton_clicked();
	void on_mitoRadioButton_toggled(bool);
	void on_boutonsSpinesRadioButton_toggled(bool);

    void on_glycogenMappingTreeWidget_itemChanged(QTreeWidgetItem* item, int column);

    //----- selected objects
    void on_object_clicked(QList<QStandardItem*>);
    void clearTable();
    void RemoveRowAt(QModelIndex);
    void getIDAt(QModelIndex);
    void checkAllListWidget();

    void checkByType(std::map<QString, int>);

    // Path Management
    void getPath(QString);
    void getSelectedPathIndex(QModelIndex);
    void deleteSelectedPath(QModelIndex);

	//------ splatting toggle
	void on_splatAstroCheckBox_toggled(bool checked);
	void on_splatGlycoCheckBox_toggled(bool checked);
	void on_splatNMitoCheckBox_toggled(bool checked);
	void on_splatAMitoCheckBox_toggled(bool checked);

	void on_specularLightCheckBox_toggled(bool checked);

    void on_filterByProximityListWidget_itemChanged(QListWidgetItem *);

    void on_listWidget_itemChanged(QListWidgetItem *);

signals:
    void getDeletedData(int);
    void getIDAtSelectedRow(int);

    // path
    void signalPathLabel(QString);
    void signalDeletedPathLabel(QString);

    void glycogenMappedSelectedState(QString, bool);
    void signalProximityTypeState(QString, bool);
    void signalMappingTreeWidget(QTreeWidget *);
    void update_glycogen_clustering_timing(QString);
    void update_glycogen_cluter_mapping_timing(QString);
    void update_glycogen_granules_mapping_timing(QString);

private:
    Ui::MainWindow *mainwindow_ui;
	std::map<int, Clustering::GlycogenCluster*>* m_clusters;
	QTreeWidgetItem* m_currentSelectedCluster;
    QStandardItemModel *tableView;
};

#endif // MAINWINDOW_H
