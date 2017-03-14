#include "glycogencluster.h"
#include "glycogenanalysismanager.h"
#include "glwidget.h"
#include "openglmanager.h"

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
	m_currentSelectedCluster = 0;
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