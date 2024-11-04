#ifndef RECOVERINGDATAWIDGET_H
#define RECOVERINGDATAWIDGET_H

#include <QDialog>
#include <QPointer>
#include <QTimer>


namespace Ui {
class recoveringdatawidget;
}

/**
 * @brief 数据恢复进度模态对话框
 * 使用方法:
 * 需要搜索设备时调用startSearchDevices(),搜索结束后返回搜索到的设备数量
 * 设备详细信息存放在devicemanager中
 * 	DeviceSearchWidget search_widget;
 *  slotSearchFinished(search_widget.startSearchDevices());
 */
class recoveringdatawidget : public QDialog
{
    Q_OBJECT

public:
    explicit recoveringdatawidget(QWidget *parent = 0);
    ~recoveringdatawidget();

    /**
     * @brief 开始显示进度
     * @return 对话框返回值
     */
	int ShowProgressDlg();

protected:
	void closeEvent(QCloseEvent *) override;

public slots:
	void onProgressChanged(int progress);

	void onSearchFinished();

	void onTimer();

private:	
	void initUI();

private:

	QTimer *timer;

	int cur_progress_{ 0 };

	//数据恢复是否成功标志
	bool m_recover_success = false;

    Ui::recoveringdatawidget *ui;
};

#endif // DEVICESEARCHWIDGET_H
