#include "QGCSwarmRemote.h"
#include "ui_QGCSwarmRemote.h"

QGCSwarmRemote::QGCSwarmRemote(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QGCSwarmRemote)
{
    ui->setupUi(this);
}

QGCSwarmRemote::~QGCSwarmRemote()
{
    delete ui;
}
