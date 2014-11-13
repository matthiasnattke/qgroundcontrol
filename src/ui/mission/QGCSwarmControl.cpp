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
#include "UASManager.h"

const unsigned int QGCSwarmControl::updateInterval = 5000U;

static struct full_mode_s modes_list_common[] = {
    { MAV_MODE_FLAG_MANUAL_INPUT_ENABLED,
            0 },
    { (MAV_MODE_FLAG_MANUAL_INPUT_ENABLED | MAV_MODE_FLAG_STABILIZE_ENABLED),
            0 },
    { (MAV_MODE_FLAG_MANUAL_INPUT_ENABLED | MAV_MODE_FLAG_STABILIZE_ENABLED | MAV_MODE_FLAG_GUIDED_ENABLED),
            0 },
    { (MAV_MODE_FLAG_AUTO_ENABLED | MAV_MODE_FLAG_STABILIZE_ENABLED | MAV_MODE_FLAG_GUIDED_ENABLED),
            0 },
};

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

	connect(ui->startButton,SIGNAL(clicked()),this,SLOT(startButton_clicked()));
	connect(ui->stopButton,SIGNAL(clicked()),this,SLOT(stopButton_clicked()));

	connect(ui->armButton,SIGNAL(clicked()),this,SLOT(armButton_clicked()));
	connect(ui->disarmButton,SIGNAL(clicked()),this,SLOT(disarmButton_clicked()));

	connect(ui->selectButton,SIGNAL(clicked()),this,SLOT(selectButton_clicked()));

	connect(ui->setHomeButton,SIGNAL(clicked()),this,SLOT(sendNewHomePosition()));

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

	connect(ui->remoteList,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(remoteItem_clicked(QListWidgetItem*)));
	
	scenarioSelected = 	ui->scenarioList->item(0);
	ui->scenarioList->setCurrentItem(scenarioSelected);

	connect(ui->scenarioList,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(scenarioListClicked(QListWidgetItem*)));

	connect(this,SIGNAL(uasTextReceived(UASInterface*, QString)),this,SLOT(textMessageReceived(UASInterface*, QString)));

	connect(&updateTimer, SIGNAL(timeout()), this, SLOT(refreshView()));
    updateTimer.start(updateInterval);

    connect(ui->setComfortSlider,SIGNAL(clicked()),this,SLOT(setComfort_clicked()));

    connect(ui->remoteButton,SIGNAL(clicked()),this,SLOT(remoteButton_clicked()));

    connect(ui->modeComboBox, SIGNAL(activated(int)), this, SLOT(setMode(int)));

    modesList = modes_list_common;
    modesNum = sizeof(modes_list_common) / sizeof(struct full_mode_s);

    // Set combobox items
    ui->modeComboBox->clear();
    for (int i = 0; i < modesNum; i++) {
        struct full_mode_s mode = modesList[i];
        ui->modeComboBox->insertItem(i, UAS::getShortModeTextFor(mode.baseMode, mode.customMode, MAV_AUTOPILOT_GENERIC).remove(0, 2), i);
    }

    // Select first mode in list
    modeIdx = 0;
    ui->modeComboBox->setCurrentIndex(modeIdx);
    ui->modeComboBox->update();

    all_selected = false;
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

	QListWidgetItem* itemRemote = new QListWidgetItem(idstring);
	itemRemote->setFlags(itemRemote->flags() | Qt::ItemIsUserCheckable);
	itemRemote->setCheckState(Qt::Checked);

	uasToItemRemote[uas] = itemRemote;
	itemToUasRemote[itemRemote] = uas;

	ui->remoteList->addItem(itemRemote);

	UASlist = UASManager::instance()->getUASList();
}


void QGCSwarmControl::RemoveUAS(UASInterface* uas)
{
	QListWidgetItem* item = uasToItemMapping[uas];
	uasToItemMapping.remove(uas);
	
	itemToUasMapping.remove(item);
	
	//ui->listWidget->removeItemWidget(item);
	ui->listWidget->takeItem(ui->listWidget->row(item));
	delete item;

	item = uasToItemRemote[uas];
	uasToItemRemote.remove(uas);

	itemToUasRemote.remove(item);

	ui->remoteList->takeItem(ui->remoteList->row(item));
	delete item;

	UASlist = UASManager::instance()->getUASList();
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

void QGCSwarmControl::startButton_clicked()
{
	//MAV_CMD_OVERRIDE_GOTO=252, /* Hold / continue the current action |MAV_GOTO_DO_HOLD: hold MAV_GOTO_DO_CONTINUE: continue with next item in mission plan| MAV_GOTO_HOLD_AT_CURRENT_POSITION: Hold at current position MAV_GOTO_HOLD_AT_SPECIFIED_POSITION: hold at specified position| MAV_FRAME coordinate frame of hold point| Desired yaw angle in degrees| Latitude / X position| Longitude / Y position| Altitude / Z position|  */
	mavlink_message_t msg;
	mavlink_msg_command_long_pack(mavlink->getSystemId(), mavlink->getComponentId(), &msg, 0, 0, MAV_CMD_OVERRIDE_GOTO, 1, MAV_GOTO_DO_CONTINUE, 0, 0, 0, 0, 0, 0);
	mavlink->sendMessage(msg);
}

void QGCSwarmControl::stopButton_clicked()
{
	mavlink_message_t msg;
	mavlink_msg_command_long_pack(mavlink->getSystemId(), mavlink->getComponentId(), &msg, 0, 0, MAV_CMD_OVERRIDE_GOTO, 1, MAV_GOTO_DO_HOLD, MAV_GOTO_HOLD_AT_CURRENT_POSITION, 0, 0, 0, 0, 0);
	mavlink->sendMessage(msg);
}

void QGCSwarmControl::armButton_clicked()
{
	if (modeIdx >= 0 && modeIdx < modesNum)
    {
        struct full_mode_s mode = modesList[modeIdx];

        mode.baseMode |= MAV_MODE_FLAG_SAFETY_ARMED;

        if (ui->avoidanceBox->checkState() == Qt::Checked)
        {
        	mode.baseMode |= MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;
        }
        else
        {
        	mode.baseMode &= ~MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;
        }

        mode.customMode = 0;

		mavlink_message_t msg;
		mavlink_msg_command_long_pack(mavlink->getSystemId(), mavlink->getComponentId(), &msg, 0, 0, MAV_CMD_DO_SET_MODE, 1, mode.baseMode, mode.customMode, 0, 0, 0, 0, 0);

		mavlink->sendMessage(msg);
	}
}

void QGCSwarmControl::disarmButton_clicked()
{
	if (modeIdx >= 0 && modeIdx < modesNum)
    {
        struct full_mode_s mode = modesList[modeIdx];

        mode.baseMode &= ~MAV_MODE_FLAG_SAFETY_ARMED;

        if (ui->avoidanceBox->checkState() == Qt::Checked)
        {
        	mode.baseMode |= MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;
        }
        else
        {
        	mode.baseMode &= ~MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;
        }
        mode.customMode = 0;

		mavlink_message_t msg;
		mavlink_msg_command_long_pack(mavlink->getSystemId(), mavlink->getComponentId(), &msg, 0, 0, MAV_CMD_DO_SET_MODE, 1, mode.baseMode, mode.customMode, 0, 0, 0, 0, 0);
		mavlink->sendMessage(msg);
	}
}

void QGCSwarmControl::selectButton_clicked()
{
	if (all_selected)
	{
		all_selected = false;
		ui->selectButton->setText(tr("Unselect all"));

		QMap<UASInterface*,QListWidgetItem*>::iterator it;
		for(it=uasToItemRemote.begin();it!=uasToItemRemote.end();++it)
		{
			it.value()->setCheckState(Qt::Checked);
		}
	}
	else
	{
		all_selected = true;
		ui->selectButton->setText(tr("Select all"));

		QMap<UASInterface*,QListWidgetItem*>::iterator it;
		for(it=uasToItemRemote.begin();it!=uasToItemRemote.end();++it)
		{
			it.value()->setCheckState(Qt::Unchecked);
		}
	}
}

void QGCSwarmControl::remoteItem_clicked(QListWidgetItem* item)
{
	if (item->checkState() == Qt::Checked)
	{
		item->setCheckState(Qt::Unchecked);
	}
	else
	{
		item->setCheckState(Qt::Checked);
	}
	
}

void QGCSwarmControl::remoteButton_clicked()
{
	mavlink_message_t msg;
	UASInterface* uas;
	int remoteVal;
	
	QMap<UASInterface*,QListWidgetItem*>::iterator it;
	for(it=uasToItemRemote.begin();it!=uasToItemRemote.end();++it)
	{
		uas = it.key();
		if(it.value()->checkState() == Qt::Checked)
		{
			remoteVal = 1;
		}
		else
		{
			remoteVal = 0;
		}
		mavlink_msg_command_long_pack(mavlink->getSystemId(), mavlink->getComponentId(), &msg, uas->getUASID(), MAV_COMP_ID_SYSTEM_CONTROL, MAV_CMD_DO_PARACHUTE, 1, remoteVal, 0, 0, 0, 0, 0, 0);
		mavlink->sendMessage(msg);
	}
}

void QGCSwarmControl::setMode(int mode)
{
    // Adapt context button mode
    modeIdx = mode;
    ui->modeComboBox->blockSignals(true);
    ui->modeComboBox->setCurrentIndex(mode);
    ui->modeComboBox->blockSignals(false);
}

void QGCSwarmControl::sendNewHomePosition()
{
    mavlink_message_t msg;

    double lat = UASManager::instance()->getHomeLatitude();
    double lon = UASManager::instance()->getHomeLongitude();
    double alt = UASManager::instance()->getHomeAltitude();

    mavlink_msg_command_long_pack(mavlink->getSystemId(), mavlink->getComponentId(), &msg, 0, 0, MAV_CMD_DO_SET_HOME, 1, 0, 0, 0, 0, lat, lon, alt);
    
    mavlink->sendMessage(msg);
}