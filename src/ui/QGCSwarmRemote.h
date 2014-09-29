#ifndef QGCSWARMREMOTE_H
#define QGCSWARMREMOTE_H

#include <QWidget>
#include "MAVLinkProtocol.h"
#include "UASManager.h"
#include "UASInterface.h"
#include "LinkManager.h"
#include <QListWidget>
#include <QMap>


namespace Ui {
class QGCSwarmRemote;
}

class QGCSwarmRemote : public QWidget
{
    Q_OBJECT

public:
    explicit QGCSwarmRemote(QWidget *parent = 0);
    ~QGCSwarmRemote();

public slots:

private slots:

private:
    Ui::QGCSwarmRemote *ui;

protected:
	MAVLinkProtocol* mavlink;     ///< Reference to the MAVLink instance
	UASInterface *uas;
	QList<UASInterface*> UASlist;
	QList<LinkInterface*> links; ///< List of links this UAS can be reached by

	QMap<UASInterface*,QListWidgetItem*> uasToItemMapping;
	QMap<QListWidgetItem*,UASInterface*> itemToUasMapping;

	UASInterface *uas_previous;

	QListWidgetItem* scenarioSelected;

};

#endif // QGCSWARMREMOTE_H
