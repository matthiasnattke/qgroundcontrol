#include "QGCSwarmControl.h"
#include "ui_QGCSwarmControl.h"

#include <QMessageBox>
#include "UAS.h"
#include "QGCMAVLink.h"
#include "MainWindow.h"
#include <QRadioButton>
#include <QString>
#include <QListWidget>
#include "UASView.h"

const unsigned int QGCSwarmControl::updateInterval = 5000U;

QGCSwarmControl::QGCSwarmControl(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QGCSwarmControl)
{
    ui->setupUi(this);

    connect(ui->continueAllButton,SIGNAL(clicked()),this,SLOT(continueAllButton_clicked()));
	connect(ui->Return2Start,SIGNAL(clicked()),this,SLOT(Return2startButton_clicked()));
	connect(ui->scenarioLaunch,SIGNAL(clicked()),this,SLOT(launchScenario_clicked()));
	connect(ui->autoLanding,SIGNAL(clicked()),this,SLOT(autoLanding_clicked()));
	connect(ui->autoTakeoff,SIGNAL(clicked()),this,SLOT(autoTakeoff_clicked()));

	connect(ui->startLogging,SIGNAL(clicked()),this,SLOT(startLogging_clicked()));
	connect(ui->stopLogging,SIGNAL(clicked()),this,SLOT(stopLogging_clicked()));

	// Get current MAV list => in parameterinterface.cc
    //QList<UASInterface*> systems = UASManager::instance()->getUASList();
	mavlink = MainWindow::instance()->getMAVLink();

	uas =  UASManager::instance()->getActiveUAS();
	uas_previous = UASManager::instance()->getActiveUAS();

	UASlist = UASManager::instance()->getUASList();

	links = LinkManager::instance()->getLinksForProtocol(mavlink);

	connect(UASManager::instance(),SIGNAL(UASCreated(UASInterface*)),this,SLOT(UASCreated(UASInterface*)));
	connect(UASManager::instance(),SIGNAL(UASDeleted(UASInterface*)),this,SLOT(RemoveUAS(UASInterface*)));

	connect(ui->listWidget,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(ListWidgetClicked(QListWidgetItem*)));
	connect(UASManager::instance(),SIGNAL(activeUASSet(UASInterface*)),this,SLOT(newActiveUAS(UASInterface*)));

	
	scenarioSelected = 	ui->scenarioList->item(0);
	ui->scenarioList->setCurrentItem(scenarioSelected);

	connect(ui->scenarioList,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(scenarioListClicked(QListWidgetItem*)));

	connect(this,SIGNAL(uasTextReceived(UASInterface*, QString)),this,SLOT(textMessageReceived(UASInterface*, QString)));

	connect(&updateTimer, SIGNAL(timeout()), this, SLOT(refreshView()));
    updateTimer.start(updateInterval);

    connect(ui->setComfortSlider,SIGNAL(clicked()),this,SLOT(setComfort_clicked()));
}

QGCSwarmControl::~QGCSwarmControl()
{
    delete ui;
}

void QGCSwarmControl::continueAllButton_clicked()
{
	qDebug() << "continueAllButton clicked";

	mavlink_message_t msg;
	mavlink_msg_command_long_pack(mavlink->getSystemId(), mavlink->getComponentId(), &msg, 0, MAV_COMP_ID_MISSIONPLANNER, MAV_CMD_MISSION_START, 1, 1, 1, 0, 0, 0, 0, 0);
	mavlink->sendMessage(msg);
}

void QGCSwarmControl::Return2startButton_clicked()
{
	qDebug() << "Return2startButton clicked";

	mavlink_message_t msg;
	mavlink_msg_command_long_pack(mavlink->getSystemId(), mavlink->getComponentId(), &msg, 0, MAV_COMP_ID_MISSIONPLANNER, MAV_CMD_NAV_RETURN_TO_LAUNCH, 1, 0, 0, 0, 0, 0, 0, 0);
    mavlink->sendMessage(msg);
}

void QGCSwarmControl::launchScenario_clicked()
{
	qDebug() << "launchScenario clicked";

	QListWidgetItem* currentItem = ui->scenarioList->currentItem();

	int scenarioNum;
	if(currentItem->text() == "Circle")
	{
		scenarioNum = 1;
	}
	else if (currentItem->text() == "Circle uniform")
	{
		scenarioNum = 2;
	}
	else
	{
		scenarioNum = 3;
	}

	int autoContinue = 0;
	if(ui->autoContinueCheckBox->checkState() == Qt::Checked)
	{
		autoContinue = 1;
	}

	mavlink_message_t msg;
	mavlink_msg_command_long_pack(mavlink->getSystemId(), mavlink->getComponentId(), &msg, 0, MAV_COMP_ID_MISSIONPLANNER, MAV_CMD_CONDITION_LAST, 1, scenarioNum, ui->radiusSpinBox->value(), ui->numVhcSpinBox->value(),  ui->altitudeSpinBox->value(), autoContinue, 0, 0);
    mavlink->sendMessage(msg);
}

void QGCSwarmControl::autoLanding_clicked()
{
	qDebug() << "autoLanding clicked";

	mavlink_message_t msg;
	mavlink_msg_command_long_pack(mavlink->getSystemId(), mavlink->getComponentId(), &msg, 0, 0, MAV_CMD_NAV_LAND, 1, 0, 0, 0, 0, 0, 0, 0);
	mavlink->sendMessage(msg);
}

void QGCSwarmControl::autoTakeoff_clicked()
{
	qDebug() << "autoTakeoff clicked";

	mavlink_message_t msg;
	mavlink_msg_command_long_pack(mavlink->getSystemId(), mavlink->getComponentId(), &msg, 0, 0, MAV_CMD_NAV_TAKEOFF, 1, 0, 0, 0, 0, 0, 0, 0);
	mavlink->sendMessage(msg);
}

void QGCSwarmControl::startLogging_clicked()
{
	qDebug() << "startLogging clicked";

	mavlink_message_t msg;
	mavlink_msg_command_long_pack(mavlink->getSystemId(), mavlink->getComponentId(), &msg, 0, 0, MAV_CMD_DO_SET_PARAMETER, 1, 1, 0, 0, 0, 0, 0, 0);
	mavlink->sendMessage(msg);
}

void QGCSwarmControl::stopLogging_clicked()
{
	qDebug() << "stopLogging clicked";

	mavlink_message_t msg;
	mavlink_msg_command_long_pack(mavlink->getSystemId(), mavlink->getComponentId(), &msg, 0, 0, MAV_CMD_DO_SET_PARAMETER, 1, 0, 0, 0, 0, 0, 0, 0);
	mavlink->sendMessage(msg);
}

void QGCSwarmControl::UASCreated(UASInterface* uas)
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

	connect(uas,SIGNAL(textMessageReceived(int,int,int,QString)),this,SLOT(textEmit(int,int,int,QString)));
}


void QGCSwarmControl::RemoveUAS(UASInterface* uas)
{
	QListWidgetItem* item = uasToItemMapping[uas];
	uasToItemMapping.remove(uas);
	
	itemToUasMapping.remove(item);
	
	//ui->listWidget->removeItemWidget(item);
	ui->listWidget->takeItem(ui->listWidget->row(item));
	delete item;
}

void QGCSwarmControl::ListWidgetClicked(QListWidgetItem* item)
{

	QMap<QListWidgetItem*,UASInterface*>::const_iterator it = itemToUasMapping.constFind(item);

	UASInterface* uas = it.value();

	if (uas != UASManager::instance()->getActiveUAS())
	{
		UASManager::instance()->setActiveUAS(uas);
	}
}

void QGCSwarmControl::newActiveUAS(UASInterface* uas)
{
	QListWidgetItem* item;

	if (uas_previous)
	{
		item = uasToItemMapping[uas_previous];
		item->setCheckState(Qt::Unchecked);
	}
	uas_previous = uas;


	item = uasToItemMapping[uas];
	ui->listWidget->setItemSelected(item,true);

	item->setCheckState(Qt::Checked);

}

void QGCSwarmControl::scenarioListClicked(QListWidgetItem* item)
{
	qDebug() << "scenario change";

	for(int i=0; i<ui->scenarioList->count(); i++)
	{
		ui->scenarioList->item(i)->setCheckState(Qt::Unchecked);
	}

	ui->scenarioList->setCurrentItem(item);

	qDebug() << "new current item: " << item->text();

	item->setCheckState(Qt::Checked);
}

void QGCSwarmControl::textEmit(int uasid, int component, int severity, QString message)
{
	Q_UNUSED(uasid);
	Q_UNUSED(component);
    Q_UNUSED(severity);

	UASInterface* uas = (UASInterface*)sender();

	emit uasTextReceived(uas,message);
}

void QGCSwarmControl::textMessageReceived(UASInterface* uas, QString message)
{
    QListWidgetItem* item;

    if (uas)
    {
    	qDebug() << "UAS id:" << QString::number(uas->getUASID());
    	item = uasToItemMapping[uas];

    	if (message.contains("SUCCESS"))
    	{
    		item->setBackground(Qt::green);
    	}
    	else
    	{
    		item->setBackground(Qt::red);
    	}
    }
}

void QGCSwarmControl::refreshView()
{
	QListWidgetItem* item;

	foreach(UASInterface* uas, UASManager::instance()->getUASList())
	{
		item = uasToItemMapping[uas];
		item->setBackground(Qt::transparent);
	}
}
void QGCSwarmControl::setComfort_clicked()
{
	float comfortValue = ui->comfortSpinBox->value();

	mavlink_message_t msg;
	mavlink_msg_command_long_pack(mavlink->getSystemId(), mavlink->getComponentId(), &msg, 0, 0, MAV_CMD_NAV_PATHPLANNING, 1, comfortValue, 0, 0, 0, 0, 0, 0);
	mavlink->sendMessage(msg);
}