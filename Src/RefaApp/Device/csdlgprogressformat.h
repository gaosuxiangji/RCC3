#ifndef CSDLGPROGRESSFORMAT_H
#define CSDLGPROGRESSFORMAT_H

#include <QDialog>
#include <QTimer>
#include <QElapsedTimer>
#include "Device/device.h"
#include <QSharedPointer>
#include <QWeakPointer>
namespace Ui {
class CSDlgProgressFormat;
}

//格式化视频存储进度条窗口
class CSDlgProgressFormat : public QDialog
{
    Q_OBJECT

public:
    explicit CSDlgProgressFormat(QSharedPointer<Device> device_ptr,QWidget *parent = 0);
    ~CSDlgProgressFormat();

public slots:


	/**
	* @brief	更新进度显示
	* @param	progress 进度
	* @note		进度到100时直接退出
	*/
	void SlotUpdateProgress(int progress);

private slots: 
	/**
	* @brief	查询超时
	* @note		超时则关闭对话框
	*/
	void SlotCheckTimeOut();
	
private:
	void InitUI();
private:
	QWeakPointer<Device> m_device_ptr;

	QPointer<QTimer> m_timer{ nullptr };//查询超时计时器
	QElapsedTimer m_elapsed_timer;//刷新用时计时器
	const qint64 kTimeout{ 10000 };//刷新超时

    Ui::CSDlgProgressFormat *ui;
};

#endif // CSDLGPROGRESSFORMAT_H
