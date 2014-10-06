#include "QGCSwarmRemote.h"
#include "ui_QGCSwarmRemote.h"

QGCSwarmRemote::QGCSwarmRemote(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QGCSwarmRemote)
{
    ui->setupUi(this);

	connect(ui->CheckAll, SIGNAL(clicked()), this, SLOT(CheckAllButton_clicked()));
	connect(ui->ClearAll, SIGNAL(clicked()), this, SLOT(ClearAllButton_clicked()));

	connect(ui->listWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(ListWidgetChanged(QListWidgetItem*)));
	connect(ui->listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(ListWidgetClicked(QListWidgetItem*)));

	// Creates fixed items in WidgetList
	QListWidgetItem* item = new QListWidgetItem("test");

	item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
	item->setCheckState(Qt::Unchecked);

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

void QGCSwarmRemote::ListWidgetChanged(QListWidgetItem* item)
{
	qDebug() << "ListWidgetItem changed";
}

void QGCSwarmRemote::ListWidgetClicked(QListWidgetItem* item)
{
	qDebug() <<"ListWidgetItem clicked";
}
