#include "QGCSwarmRemote.h"
#include "ui_QGCSwarmRemote.h"

#include <QMessageBox>
#include "UAS.h"
#include "QGCMAVLink.h"
#include "MainWindow.h"
#include <QRadioButton>
#include <QString>
#include <QListWidget>

QGCSwarmRemote::QGCSwarmRemote(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QGCSwarmRemote)
{
    ui->setupUi(this);

	connect(ui->CheckAll, SIGNAL(clicked()), this, SLOT(CheckAllButton_clicked()));
	connect(ui->ClearAll, SIGNAL(clicked()), this, SLOT(ClearAllButton_clicked()));

	connect(ui->listWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(ListWidgetChanged(QListWidgetItem*)));
	connect(ui->listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(ListWidgetClicked(QListWidgetItem*)));

	// Creates Test item in WidgetList
	QListWidgetItem* item = new QListWidgetItem("test");

	item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
	item->setCheckState(Qt::Unchecked);

	//Creates list of connected UAS

	mavlink = MainWindow::instance()->getMAVLink();

	uas =  UASManager::instance()->getActiveUAS();
	uas_previous = UASManager::instance()->getActiveUAS();

	UASlist = UASManager::instance()->getUASList();

	links = LinkManager::instance()->getLinksForProtocol(mavlink);

	connect(UASManager::instance(),SIGNAL(UASCreated(UASInterface*)),this,SLOT(UASCreated(UASInterface*)));
	connect(UASManager::instance(),SIGNAL(UASDeleted(UASInterface*)),this,SLOT(RemoveUAS(UASInterface*)));


	ui->listWidget->addItem(item);
}

QGCSwarmRemote::~QGCSwarmRemote()
{
    delete ui;
}

void QGCSwarmRemote::UASCreated(UASInterface* uas)
{
	QString idstring;
	if (uas->getUASName() == "")
    {
        idstring = tr("UAS ") + QString::number(uas->getUASID());
    }
    else
    {
        idstring = uas->getUASName();
    }

	//QListWidgetItem* item = new QListWidgetItem(ui->listWidget);
	//ui->listWidget->setItemWidget(item,new QRadioButton(idstring));

	QListWidgetItem* item = new QListWidgetItem(idstring);

	item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
	item->setCheckState(Qt::Unchecked);

	ui->listWidget->addItem(item);

	uasToItemMapping[uas] = item;
	itemToUasMapping[item] = uas;
}


void QGCSwarmRemote::RemoveUAS(UASInterface* uas)
{
	QListWidgetItem* item = uasToItemMapping[uas];
	uasToItemMapping.remove(uas);
	
	itemToUasMapping.remove(item);
	
	//ui->listWidget->removeItemWidget(item);
	ui->listWidget->takeItem(ui->listWidget->row(item));
	delete item;
}

void QGCSwarmRemote::CheckAllButton_clicked()
{
	qDebug() << "CheckAllButton clicked";
}

void QGCSwarmRemote::ClearAllButton_clicked()
{
	qDebug() << "ClearAllButton clicked";
}

void QGCSwarmRemote::ListWidgetChanged(QListWidgetItem* item)
{
	qDebug() << "ListWidgetItem changed";
}

void QGCSwarmRemote::ListWidgetClicked(QListWidgetItem* item)
{
	qDebug() <<"ListWidgetItem clicked";
}
