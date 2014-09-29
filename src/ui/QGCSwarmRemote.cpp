#include "QGCSwarmRemote.h"
#include "ui_QGCSwarmRemote.h"

QGCSwarmRemote::QGCSwarmRemote(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QGCSwarmRemote)
{
    ui->setupUi(this);

	connect(ui->CheckAll,SIGNAL(clicked()),this,SLOT(CheckAllButton_clicked()));
	connect(ui->ClearAll,SIGNAL(clicked()),this,SLOT(ClearAllButton_clicked()));

	//Creates fixed items in WidgetList
	QListWidgetItem* item = new QListWidgetItem("test");
	ui->listWidget->addItem(item);
}

QGCSwarmRemote::~QGCSwarmRemote()
{
    delete ui;
}

void QGCSwarmRemote::CheckAllButton_clicked()
{
	qDebug() << "CheckAllButton clicked";
}

void QGCSwarmRemote::ClearAllButton_clicked()
{
	qDebug() << "ClearAllButton clicked";
}