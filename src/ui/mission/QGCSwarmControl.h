#ifndef QGCSWARMCONTROL_H
#define QGCSWARMCONTROL_H

#include <QWidget>
#include "MAVLinkProtocol.h"
#include "UASManager.h"
#include "UASInterface.h"
#include "LinkManager.h"
#include <QListWidget>
#include <QMap>
#include "UASControlWidget.h"

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

	void textMessageReceived(UASInterface* uas, QString message);
	void textEmit(int uasid, int component, int severity, QString message);

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

	void refreshView();

	void setComfort_clicked();

	void startButton_clicked();
	void stopButton_clicked();

	void armButton_clicked();
	void disarmButton_clicked();

	void selectButton_clicked();

	void remoteItem_clicked(QListWidgetItem* item);

	void remoteButton_clicked();

	void setMode(int mode);
private:
    Ui::QGCSwarmControl *ui;

protected:
	MAVLinkProtocol* mavlink;     ///< Reference to the MAVLink instance
	UASInterface *uas;
	QList<UASInterface*> UASlist;
	QList<LinkInterface*> links; ///< List of links this UAS can be reached by

	QMap<UASInterface*,QListWidgetItem*> uasToItemMapping;
	QMap<QListWidgetItem*,UASInterface*> itemToUasMapping;

	QMap<UASInterface*,QListWidgetItem*> uasToItemRemote;
	QMap<QListWidgetItem*,UASInterface*> itemToUasRemote;	

	UASInterface *uas_previous;

	QListWidgetItem* scenarioSelected;

	QTimer updateTimer; ///< Only update at 1 Hz to not overload the GUI
	static const unsigned int updateInterval; ///< The update interval of the refresh function

	bool all_selected;

	struct full_mode_s *modesList;  ///< Modes list for the current UAS
    int modesNum;                   ///< Number of modes in list for the current UAS
    int modeIdx;                    ///< Current uas mode index

signals:
	void uasTextReceived(UASInterface* uas, QString message);

};

#endif // QGCSWARMCONTROL_H
