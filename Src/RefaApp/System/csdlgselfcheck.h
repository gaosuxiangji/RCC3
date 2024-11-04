#ifndef CSDLGSELFCHECK_H
#define CSDLGSELFCHECK_H

#include <QDialog>
#include <QTimer>
#include <QPointer>
#include "Device/devicemanager.h"

namespace Ui {
class CSDlgSelfCheck;
}


/**
 * @brief 设备自检对话框
 */
class CSDlgSelfCheck : public QDialog
{
    Q_OBJECT

public:
    explicit CSDlgSelfCheck(QWidget *parent = 0);
    ~CSDlgSelfCheck();


private slots:
	
	void processSelfCheck();

private:
	
	void InitUI();

	QPointer<QTimer> m_timer_ptr; // 自检刷新定时器

	QList<QSharedPointer<Device>> m_device_list;//当前设备列表
	int m_current_device_index = 0;//当前设备序号


    Ui::CSDlgSelfCheck *ui;
};

#endif // CSDLGSELFCHECK_H
