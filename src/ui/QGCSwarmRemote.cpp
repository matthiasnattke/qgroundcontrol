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

	// Creates Test item in WidgetList
	/*QListWidgetItem* item = new QListWidgetItem("test1");

	item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
	item->setCheckState(Qt::Unchecked);

	ui->listWidget->addItem(item);

	item = new QListWidgetItem("test2");

	item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
	item->setCheckState(Qt::Unchecked);

	ui->listWidget->addItem(item);
	*/
	//Creates list of connected UAS

	mavlink = MainWindow::instance()->getMAVLink();

	uas =  UASManager::instance()->getActiveUAS();
	uas_previous = UASManager::instance()->getActiveUAS();

	UASlist = UASManager::instance()->getUASList();

	links = LinkManager::instance()->getLinksForProtocol(mavlink);

	connect(UASManager::instance(),SIGNAL(UASCreated(UASInterface*)),this,SLOT(UASCreated(UASInterface*)));
	connect(UASManager::instance(),SIGNAL(UASDeleted(UASInterface*)),this,SLOT(RemoveUAS(UASInterface*)));


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
	int row;
	for(row = 0; row < ui->listWidget->count(); row++)
	{
		QListWidgetItem* item = ui->listWidget->item(row);
		item->setCheckState(Qt::Checked);
	}
}

void QGCSwarmRemote::ClearAllButton_clicked()
{
	int row;
	for(row = 0; row < ui->listWidget->count(); row++)
	{
		QListWidgetItem* item = ui->listWidget->item(row);
		item->setCheckState(Qt::Unchecked);
	}
}

void QGCSwarmRemote::ListWidgetChanged(QListWidgetItem* item)
{
	
	UASInterface* uas = itemToUasMapping[item];

	//sets the value of bool depending on the item state
	int arm = (item->checkState() == Qt::Checked) ? 1 : 0;

	//mavlink msg TODO change comp id to uas->getComponentId()
	mavlink_message_t msg;
	mavlink_msg_command_long_pack(mavlink->getSystemId(), mavlink->getComponentId(), &msg, uas->getUASID(), MAV_COMP_ID_SYSTEM_CONTROL, MAV_CMD_COMPONENT_ARM_DISARM, arm, 0, 0, 0, 0, 0, 0, 0);
	mavlink->sendMessage(msg);

	printf("arm=%d",arm);
}
