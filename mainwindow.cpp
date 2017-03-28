#include "glycogencluster.h"
#include "object.h"
#include "glycogenanalysismanager.h"
#include "glwidget.h"
#include "openglmanager.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <map>
#include <QPixmap>

//------------------------------------------------------
//
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mainwindow_ui(new Ui::MainWindow)
{
    mainwindow_ui->setupUi(this);
    m_currentSelectedCluster = 0;
    m_clusters = 0;

    // setup the table view
    tableView = new QStandardItemModel(0, 2, this); //0 Rows and 2 Columns
    tableView->setHorizontalHeaderItem(0, new QStandardItem(QString("ID")));
    tableView->setHorizontalHeaderItem(1, new QStandardItem(QString("Name")));

    mainwindow_ui->tableView->setModel(tableView);

    QObject::connect(getGLWidget(), SIGNAL(object_clicked(QList<QStandardItem*>)),
                     this, SLOT(on_object_clicked(QList<QStandardItem*>)));

    QObject::connect(getGLWidget(), SIGNAL(clearRowsTable()),
                     this, SLOT(clearTable()));

    QObject::connect(getGLWidget(), SIGNAL(RemoveRowAt_GL(QModelIndex)),
                     this, SLOT(RemoveRowAt(QModelIndex)));

    QObject::connect(this, SIGNAL(getDeletedData(int)),
                     getGLWidget(), SLOT(getDeletedHVGXID(int)));

    QObject::connect(getGLWidget(), SIGNAL(checkAllListWidget_GL()),
                     this, SLOT(checkAllListWidget()));


    QObject::connect(getGLWidget(), SIGNAL(GetIDFrom(QModelIndex)),
                     this, SLOT(getIDAt(QModelIndex)));

    QObject::connect(this, SIGNAL(getIDAtSelectedRow(int)),
                     getGLWidget(), SLOT(getHVGXIDAtSelectedRow(int)));

    QObject::connect(getMousePad(), SIGNAL(addPathtoList(QString)),
                     this, SLOT(getPath(QString)));

    QObject::connect(getMousePad(), SIGNAL(signalSelectedPath(QModelIndex)),
                     this, SLOT(getSelectedPathIndex(QModelIndex)));

    QObject::connect(this, SIGNAL(signalPathLabel(QString)),
                     getMousePad(), SLOT(getSelectedPathLabel(QString)));

    QObject::connect(getMousePad(), SIGNAL(signalDeletePath(QModelIndex)),
                     this, SLOT(deleteSelectedPath(QModelIndex)));

    QObject::connect(this, SIGNAL(signalDeletedPathLabel(QString)),
                     getMousePad(), SLOT(getSelectedPathLabelToDelete(QString)));

    QObject::connect(this, SIGNAL(glycogenMappedSelectedState(QString, bool)),
                     getGLWidget(), SLOT(getglycogenMappedSelectedState(QString, bool)));

    QObject::connect(this, SIGNAL(signalProximityTypeState(QString, bool)),
                     getGLWidget(), SLOT(getProximityTypeState(QString, bool)));


    QObject::connect(getGLWidget(), SIGNAL(signalCheckByType(std::map<QString, int>)),
                     this, SLOT(checkByType(std::map<QString, int>)));


    QObject::connect(this, SIGNAL(signalMappingTreeWidget(QTreeWidget *)),
                     getGLWidget(), SLOT(getMappingTreeWidget(QTreeWidget *)));


	//rotate astrocytes label of the abstraction space
	QPixmap myPixmap(QSize(20, 210));
	myPixmap.fill(Qt::transparent);
	QPainter painter(&myPixmap);
	painter.translate(12, 140);
	painter.rotate(-90);
	painter.drawText(0, 0, "Astrocytes");
	mainwindow_ui->label_44->setPixmap(myPixmap);

 }

//------------------------------------------------------
//
MainWindow::~MainWindow()
{
    delete mainwindow_ui;
}

//------------------------------------------------------
//
GLWidget* MainWindow::getGLWidget()
{
    return mainwindow_ui->openGLWidget;
}

//------------------------------------------------------
//
MousePad* MainWindow::getMousePad()
{
    return mainwindow_ui->openGLWidget_2;
}

//------------------------------------------------------
//
void MainWindow::on_object_clicked(QList<QStandardItem*> items)
{
    tableView->appendRow(items);
}

//------------------------------------------------------
//
void MainWindow::clearTable()
{
    tableView->removeRows(0, tableView->rowCount(), QModelIndex());

}

//------------------------------------------------------
//
void MainWindow::checkAllListWidget()
{
    qDebug() << "checkAllListWidget";
    for (int row = 0; row < mainwindow_ui->listWidget->count(); row++) {
        QListWidgetItem *item = mainwindow_ui->listWidget->item(row);
        qDebug() << item->text();
        item->setCheckState(Qt::Checked);
    }
}

void MainWindow::checkByType(std::map<QString, int> checkStateByType)
{
    bool oldState =  mainwindow_ui->listWidget->blockSignals(true);

    for (int row = 0; row < mainwindow_ui->listWidget->count(); row++) {
        QListWidgetItem *item = mainwindow_ui->listWidget->item(row);
        int flag = checkStateByType[item->text()];

        if (flag == 0) {
            item->setCheckState(Qt::Unchecked);
        } else if (flag == 1) {
            item->setCheckState(Qt::Checked);
        } else {
            item->setCheckState(Qt::PartiallyChecked);
        }
    }

    mainwindow_ui->listWidget->blockSignals(oldState);

    // SEND THIS LSIT WIDGET TO GLWIDGET
     signalMappingTreeWidget(mainwindow_ui->glycogenMappingTreeWidget);
}

//------------------------------------------------------
//
void MainWindow::RemoveRowAt(QModelIndex selectedIndex)
{
    QModelIndex hvgxID_index = tableView->index(selectedIndex.row() , 0);
    QVariant hvgxID_item =  tableView->data(hvgxID_index);
    int hvgxID = hvgxID_item.toInt();
    getDeletedData(hvgxID);
    tableView->removeRows(selectedIndex.row(), 1, QModelIndex());
}

void MainWindow::getIDAt(QModelIndex selectedIndex)
{
    qDebug() << "getIDAt";
    QModelIndex hvgxID_index = tableView->index(selectedIndex.row() , 0);
    QVariant hvgxID_item =  tableView->data(hvgxID_index);
    int hvgxID = hvgxID_item.toInt();

    getIDAtSelectedRow(hvgxID);
}

//------------------------------------------------------
//
void MainWindow::on_glycogenVisibilityCheckBox_toggled(bool checked)
{
    //qDebug() << "glycogen visibility toggled";

    getGLWidget()->getOpenGLManager()->setRenderGlycogenGranules(checked);
}

//------------------------------------------------------
//
void MainWindow::on_clusterButton_clicked()
{

    mainwindow_ui->glycogenClustersTreeWidget->clear();
    if (m_clusters)
        m_clusters->clear();

    //qDebug() << "cluster button clicked";
    GlycogenAnalysisManager* gam = getGLWidget()->getGlycogenAnalysisManager();
    double eps = mainwindow_ui->epsDoubleSpinBox->value();
    int minPts = mainwindow_ui->minPtsSpinBox->value();
    //run clustering
    m_clusters = gam->runDBScan(eps, minPts);


    mainwindow_ui->glycogenClustersTreeWidget->setColumnCount(3);
    QStringList headerLabels;
    headerLabels.push_back(tr("ClusterID"));
    headerLabels.push_back(tr("Count"));
    headerLabels.push_back(tr("Volume"));
    mainwindow_ui->glycogenClustersTreeWidget->setHeaderLabels(headerLabels);

    mainwindow_ui->glycogenClustersTreeWidget->setSortingEnabled(false);

    //fill clustering tree widget
    for (auto iter = m_clusters->begin(); iter != m_clusters->end(); iter++)
    {
        Clustering::GlycogenCluster* cluster = (*iter).second;
        //create cluster parent
        QTreeWidgetItem* cluster_item = new QTreeWidgetItem(mainwindow_ui->glycogenClustersTreeWidget);
        cluster_item->setText(0, QString::number(cluster->getID()));
        cluster_item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        cluster_item->setCheckState(0, Qt::Checked);

        cluster_item->setText(1, QString::number(cluster->getClusterSize()));
        cluster_item->setText(2, QString::number(cluster->getTotalVolume() , 'f', 4));
    }
    mainwindow_ui->glycogenClustersTreeWidget->setSortingEnabled(true);
    mainwindow_ui->glycogenClustersTreeWidget->sortByColumn(2, Qt::SortOrder::DescendingOrder);
    getGLWidget()->getOpenGLManager()->updateGlycogenPoints();

}

//------------------------------------------------------
//
void MainWindow::on_glycogenClustersTreeWidget_itemChanged(QTreeWidgetItem* item, int column)
{
    //qDebug() << "item changed " << item->text(column);
    if (item)
    {
        int id = item->text(0).toInt();
        if (m_currentSelectedCluster)
        {
            int id2 = m_currentSelectedCluster->text(0).toInt();

            if (id != id2)
                m_clusters->at(id)->setState((item->checkState(0) == Qt::Checked) ? 1 : 0);
        }
        else
        {
            m_clusters->at(id)->setState((item->checkState(0) == Qt::Checked) ? 1 : 0);
        }

        getGLWidget()->getOpenGLManager()->updateGlycogenPoints();
    }
}

//------------------------------------------------------
//
void MainWindow::on_glycogenClustersTreeWidget_itemSelectionChanged()
{
    //qDebug() << "item selection changed";
    QList<QTreeWidgetItem*> selected_Items = mainwindow_ui->glycogenClustersTreeWidget->selectedItems();

    if (m_currentSelectedCluster)
    {

        int id = m_currentSelectedCluster->text(0).toInt();
        //unselect
        m_clusters->at(id)->setState((m_currentSelectedCluster->checkState(0) == Qt::Checked) ? 1 : 0);
        m_currentSelectedCluster = 0;
    }

    if (selected_Items.size())
    {
        //set selected
        m_currentSelectedCluster = selected_Items[0];
        int id = m_currentSelectedCluster->text(0).toInt();
        m_clusters->at(id)->setState(2);
    }
    getGLWidget()->getOpenGLManager()->updateGlycogenPoints();
}

//------------------------------------------------------
//
void MainWindow::on_mapGlycogenGranulesButton_clicked()
{
	mainwindow_ui->glycogenMappingTreeWidget->clear();

    GlycogenAnalysisManager* gam = getGLWidget()->getGlycogenAnalysisManager();

    bool boutons = mainwindow_ui->glycogenMapToBoutonsCheckBox->isChecked();
    bool spines = mainwindow_ui->glycogenMapToSpinesCheckBox->isChecked();
	bool mito = mainwindow_ui->mitoRadioButton->isChecked();
	if (mito)
	{
		boutons = false;
		spines = false;
	}
    bool clusters = false;

	std::map<int, std::map<int, int>>* results = gam->computeGlycogenMapping(boutons, spines, mito, clusters);
	std::map<int, float>* result_volumes = gam->getCurrentMappingVolumes();
	float result_max_volume = gam->getCurrentMappedMaxVolume();

    DataContainer* dc = getGLWidget()->getDataContainer();
	dc->resetMappingValues();
	std::map<int, Object*>* objects = dc->getObjectsMapPtr();

    mainwindow_ui->glycogenMappingTreeWidget->setColumnCount(4);
    QStringList headerLabels;
    headerLabels.push_back(tr("Object ID"));
    headerLabels.push_back(tr("Name"));
    headerLabels.push_back(tr("Granules"));
	headerLabels.push_back(tr("Total Volume"));
    mainwindow_ui->glycogenMappingTreeWidget->setHeaderLabels(headerLabels);

    //fill mapping tree widget
    for (auto iter = results->begin(); iter != results->end(); iter++)
    {
        int object_id = iter->first;
        Object* current_object = objects->at(object_id);

        //create map item
        QTreeWidgetItem* map_item = new QTreeWidgetItem(mainwindow_ui->glycogenMappingTreeWidget);
        map_item->setText(0, QString::number(current_object->getHVGXID()));
        map_item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        map_item->setCheckState(0, Qt::Checked);

        map_item->setText(1, QString(current_object->getName().c_str()));
        map_item->setText(2, QString::number(iter->second.size()));
		float volume = result_volumes->at(object_id);
		map_item->setText(3, QString::number(volume, 'f', 4));

		//set total volume value to object to color code
		current_object->setMappedValue(volume / result_max_volume);

    }
}

//------------------------------------------------------
//
void MainWindow::on_mapGlycogenClustersButton_clicked()
{
    if (m_clusters && m_clusters->size() > 0)
    {
		mainwindow_ui->glycogenMappingTreeWidget->clear();

        GlycogenAnalysisManager* gam = getGLWidget()->getGlycogenAnalysisManager();

        bool boutons = mainwindow_ui->glycogenMapToBoutonsCheckBox->isChecked();
        bool spines = mainwindow_ui->glycogenMapToSpinesCheckBox->isChecked();
		bool mito = mainwindow_ui->mitoRadioButton->isChecked(); 
		if (mito)
		{
			boutons = false;
			spines = false;
		}
        bool clusters = true;

        std::map<int, std::map<int, int>>* results = gam->computeGlycogenMapping(boutons, spines, mito, clusters);
		std::map<int, float>* result_volumes = gam->getCurrentMappingVolumes();
		float result_max_volume = gam->getCurrentMappedMaxVolume();

        DataContainer* dc = getGLWidget()->getDataContainer();
		dc->resetMappingValues();
        std::map<int, Object*>* objects = dc->getObjectsMapPtr();

        mainwindow_ui->glycogenMappingTreeWidget->setColumnCount(4);
        QStringList headerLabels;
        headerLabels.push_back(tr("Object ID"));
        headerLabels.push_back(tr("Name"));
        headerLabels.push_back(tr("Clusters"));
		headerLabels.push_back(tr("Total Volume"));
        mainwindow_ui->glycogenMappingTreeWidget->setHeaderLabels(headerLabels);

        //fill mapping tree widget
        for (auto iter = results->begin(); iter != results->end(); iter++)
        {
            int object_id = iter->first;
            Object* current_object = objects->at(object_id);

            //create map item
            QTreeWidgetItem* map_item = new QTreeWidgetItem(mainwindow_ui->glycogenMappingTreeWidget);
            map_item->setText(0, QString::number(current_object->getHVGXID()));
            map_item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            map_item->setCheckState(0, Qt::Checked);

            map_item->setText(1, QString(current_object->getName().c_str()));
            map_item->setText(2, QString::number(iter->second.size()));
			float volume = result_volumes->at(object_id);
			map_item->setText(3, QString::number(volume, 'f', 4));

			//set total volume value to object to color code
			current_object->setMappedValue(volume / result_max_volume);
        }
    }
    else
    {
        //show message saying: No clusters available, calculate clusters first
    }
}

//------------------------------------------------------
//
void MainWindow::on_mitoRadioButton_toggled(bool checked)
{
		mainwindow_ui->glycogenMapToBoutonsCheckBox->setEnabled(!checked);
		mainwindow_ui->glycogenMapToSpinesCheckBox->setEnabled(!checked);
}

//------------------------------------------------------
//
void MainWindow::on_boutonsSpinesRadioButton_toggled(bool checked)
{
		mainwindow_ui->glycogenMapToBoutonsCheckBox->setEnabled(checked);
		mainwindow_ui->glycogenMapToSpinesCheckBox->setEnabled(checked);
}

//------------------------------------------------------
//
void MainWindow::getPath(QString pathLabel)
{
    mainwindow_ui->listPaths->addItem(pathLabel);
}

//------------------------------------------------------
//
void MainWindow::getSelectedPathIndex(QModelIndex index)
{
    QString pathLabel = mainwindow_ui->listPaths->item(index.row())->text();
    signalPathLabel(pathLabel);
}

//------------------------------------------------------
//
void MainWindow::deleteSelectedPath(QModelIndex index)
{
    // return label to mousepad to remove it from paths list
    QListWidgetItem *item = mainwindow_ui->listPaths->item(index.row());
    QString pathLabel = item->text();
    delete mainwindow_ui->listPaths->takeItem(index.row());
    signalDeletedPathLabel(pathLabel);
}

//------------------------------------------------------
//
void MainWindow::on_splatAstroCheckBox_toggled(bool checked)
{
	//qDebug() << "glycogen visibility toggled";

	getGLWidget()->getOpenGLManager()->setAstroSplat(checked);
}

//------------------------------------------------------
//
void MainWindow::on_splatGlycoCheckBox_toggled(bool checked)
{
	//qDebug() << "glycogen visibility toggled";

	getGLWidget()->getOpenGLManager()->setGlycoSplat(checked);
}

//------------------------------------------------------
//
void MainWindow::on_splatNMitoCheckBox_toggled(bool checked)
{
	//qDebug() << "glycogen visibility toggled";

	getGLWidget()->getOpenGLManager()->setNMitoSplat(checked);
}

//------------------------------------------------------
//
void MainWindow::on_splatAMitoCheckBox_toggled(bool checked)
{
	//qDebug() << "glycogen visibility toggled";

	getGLWidget()->getOpenGLManager()->setAMitoSplat(checked);
}

//------------------------------------------------------
//
void MainWindow::on_specularLightCheckBox_toggled(bool checked)
{
	getGLWidget()->getOpenGLManager()->setSpecularLight(checked);
}

void  MainWindow::on_glycogenMappingTreeWidget_itemChanged(QTreeWidgetItem* item, int column)
{
   bool state = true;
   if ( item->checkState(0) == Qt::Unchecked)
       state = false;

   QString ID_str = item->text(0); // highlighted immediately
   glycogenMappedSelectedState(ID_str, state);
}

void MainWindow::on_filterByProximityListWidget_itemChanged(QListWidgetItem *item)
{
    bool flag = true;
    if ( item->checkState() == Qt::Unchecked )
        flag = false;

    signalProximityTypeState( item->text() ,flag );
}

void MainWindow::on_listWidget_itemChanged(QListWidgetItem *)
{
   signalMappingTreeWidget(mainwindow_ui->glycogenMappingTreeWidget);
}
