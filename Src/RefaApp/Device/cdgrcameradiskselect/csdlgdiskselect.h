#ifndef CSDLGDISKSELECT_H
#define CSDLGDISKSELECT_H
#include <QDialog>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QMap>
#include "Device/device.h"
#include <qcheckbox.h>
#include <QComboBox>
#include <unordered_map>
namespace Ui {
class csdlgdiskselect;
}

class csdlgdiskselect : public QDialog
{
    Q_OBJECT

public:
    explicit csdlgdiskselect(QSharedPointer<Device> device_ptr,QWidget *parent = 0);
    ~csdlgdiskselect();

private:
    Ui::csdlgdiskselect *ui;
	QString mCurrentDisk;
	QWeakPointer<Device> m_device_ptr;
	//QMap<QString, bool> m_disk_name;
	QMap<QString,QList<QString>>m_disktype;
	QMap<QCheckBox *, QComboBox *> m_WidegtButton;
	uint32_t m_BoxCheckNum = 0;
	uint32_t m_MaxRadioNum = 0;
private:
	void InitUI();
	void SaveDisk();
	void CheckStatusChange(int status);
	void InitMap(const std::vector<HscDiskMessage> &diskmessages);
signals:
	void signalChecked(bool checked);
	
};

#endif // CSDLGDISKSELECT_H
