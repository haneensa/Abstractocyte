#include "glycogencluster.h"
#include "glycogenanalysismanager.h"
#include "glwidget.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <map>

//------------------------------------------------------
//
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mainwindow_ui(new Ui::MainWindow)
{
    mainwindow_ui->setupUi(this);
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
void MainWindow::on_glycogenVisibilityCheckBox_toggled(bool checked)
{
	qDebug() << "glycogen visibility toggled";
}

//------------------------------------------------------
//
void MainWindow::on_clusterButton_clicked()
{
	m_clusters = 0;
	mainwindow_ui->glycogenClustersTreeWidget->clear();

	qDebug() << "cluster button clicked";
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

	
}