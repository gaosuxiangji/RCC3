#ifndef CSSECONDSCREEN_H
#define CSSECONDSCREEN_H

#include <QDialog>
#include <QHBoxLayout>
#include "../render/PlayerViewBase.h"
#include <QMouseEvent>

namespace Ui {
class CSSecondScreen;
}

class CSSecondScreen : public QDialog
{
    Q_OBJECT

public:
    explicit CSSecondScreen(const RccFrameInfo& img,int frame_head_type = 1, QWidget *parent = 0);
    ~CSSecondScreen();

public slots:
	/**************************
	* @brief: 更新图像槽函数
	* @param:image 图像信息
	* @return:
	* @author:mpp
	* @date:2022/06/09
	***************************/
	void SlotImageUpdate(const RccFrameInfo& image);

	// Add by Juwc 2022/9/27
	/************************************************************
	* @brief:	重写accept和reject函数，捕获窗口关闭事件
	* @param:	无
	* @return:	无
	************************************************************/
	void accept() override;
	void reject() override;

protected:
	// Add by Juwc 2022/9/27
	/************************************************************
	* @brief:	重写closeEvent函数，处理窗口关闭事件
	* @param:	event 关闭事件
	* @return:	无
	************************************************************/
	void closeEvent(QCloseEvent* event) override;

signals:
	// Add by Juwc 2022/9/27
	/************************************************************
	* @brief:	定义窗口关闭信号
	* @param:	无
	* @return:	无
	************************************************************/
	void SecondScreenCloseSignal();

private:
	/**************************
	* @brief:初始化 
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/09
	***************************/
	void Init();

	/**************************
	* @brief: 初始化UI
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/09
	***************************/
	void InitUI();

	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);

private:
	CPlayerViewBase* m_player;    //播放器
	QPoint m_pt{};    //m_pt=差值=鼠标当前位置-窗口左上角点
	int  frame_head_type_ = 1;
private:
    Ui::CSSecondScreen *ui;
};

#endif // CSSECONDSCREEN_H
