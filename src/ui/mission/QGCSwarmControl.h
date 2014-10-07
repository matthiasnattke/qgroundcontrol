#ifndef QGCSWARMCONTROL_H
#define QGCSWARMCONTROL_H

#include <QWidget>
#include "MAVLinkProtocol.h"
#include "UASManager.h"
#include "UASInterface.h"
#include "LinkManager.h"
#include <QListWidget>
#include <QMap>


namespace Ui {
class QGCSwarmControl;
}

class QGCSwarmControl : public QWidget
{
    Q_OBJECT

public:
    explicit QGCSwarmControl(QWidget *parent = 0);
    ~QGCSwarmControl();

public slots:

private slots:
    void continueAllButton_clicked();
	void Return2startButton_clicked();
	void launchScenario_clicked();
	void autoLanding_clicked();
	void autoTakeoff_clicked();

	void startLogging_clicked();
	void stopLogging_clicked();

	void UASCreated(UASInterface* uas);
	void RemoveUAS(UASInterface* uas);
	void ListWidgetClicked(QListWidgetItem* item);

	void newActiveUAS(UASInterface* uas);
	void scenarioListClicked(QListWidgetItem* item);

private:
    Ui::QGCSwarmControl *ui;

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

#endif // QGCSWARMCONTROL_H
