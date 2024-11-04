#ifndef CSDLGIMAGETRAINNING_H
#define CSDLGIMAGETRAINNING_H

#include <QDialog>
#include <QSharedPointer>
#include <QButtonGroup>
#include <QPointer>
#include <functional>
#include <QCloseEvent>
namespace Ui {
class CSDlgImageTrainning;
}
class Device;
/**
 * @brief 图像训练对话框
 */
class CSDlgImageTrainning : public QDialog
{
    Q_OBJECT

public:
    explicit CSDlgImageTrainning(QWidget *parent = 0);
    ~CSDlgImageTrainning();

	/**
	 * @brief 设置当前设备
	 * @param device 设备指针
	 * @note 根据设备情况刷新当前界面使能
	 */
	void setDevice(QSharedPointer<Device> device) ;

protected:
	void closeEvent(QCloseEvent *e) override;
protected:
	/**
	*@brief 变化事件
	*@param [in] : * event : QEvent，事件指针
	**/
	void changeEvent(QEvent *event) override;
signals:
	void signalTaskFinished(bool hide_window);

private slots:
    void on_pushButtonMainBoard_clicked();

    void on_pushButtonSlaveBoard_clicked();

	/**
	 * @brief 槽函数 测试模式按钮组被点击
	 * @param id 按键id
	 */
	void slotTestModeButtonsClicked(int id);

	/**
	 * @brief 任务执行完成
	 * @param hide_window 是否隐藏窗口
	 */
	void slotTaskFinished(bool hide_window);
private:

	typedef std::function<void()> Task;

	/**
	 * @brief 启动任务 
	 * @param
	 */
	void startTask(Task task);

	void doEnableImageTraining(bool enable, bool hide);
	void doImageTraining(bool master);

private:
	//按钮组
	QPointer<QButtonGroup> m_button_group_test_mode;
	//当前设备
	QSharedPointer<Device> m_device;
    Ui::CSDlgImageTrainning *ui;
};

#endif // CSDLGIMAGETRAINNING_H
