#ifndef ACQPARAMDIAGRAM_H
#define ACQPARAMDIAGRAM_H

#include <QWidget>
#include <QPixmap>

namespace Ui {
class AcqParamDiagram;
}

/**
 * @brief 采集参数示意图绘制界面类
 * 使用方法:
 * //准备保存起点方式(RecordOffsetMode) 保存起点(int) 保存长度(int)参数
 *
 * //更新图像
 * ui->widgetAcqDiagram->drawDiagram(record_offset_mode , record_offset.toInt(),record_length.toInt());
 */
class AcqParamDiagram : public QWidget
{
    Q_OBJECT

public:
    explicit AcqParamDiagram(QWidget *parent = 0);
    ~AcqParamDiagram();

    /**
     * @brief The RecordOffsetMode enum
     */
	enum RecordOffsetMode
	{
		BEFORE_SHUTTER,
		AFTER_SHUTTER
	};

    /**
     * @brief 绘制示意图
     * @param offset_mode 保存起点方式:触发前 触发后
     * @param offset 保存起点
     * @param record_length 保存长度
     */
	void drawDiagram(RecordOffsetMode offset_mode,int offset, int record_length);

protected:
	/** @brief 重载绘制事件
	@param [in] : QPaintEvent *event : 事件对象
	@return
	@note
	*/
	virtual void paintEvent(QPaintEvent *event) override;

private:
	/** @brief 制作绘图缓冲区
	@param
	@return
	@note
	*/
	void MakePaintBuffer();
private:
    Ui::AcqParamDiagram *ui;

	bool isFirstPaint = true;//是否是初次绘制播放控制控件
	QPixmap paintBuffer;//缓冲示意图
	QPixmap paintBuffer_backup;//绘图缓冲区的备份

	QPixmap trigger_lable_img;//触发帧图标

	RecordOffsetMode offset_mode_{ AFTER_SHUTTER };
	int offset_{ 0 };
	int record_length_{ 1 };

	const float acqAreaHeightStretch = 0.5f;//采集区域高度比例
	const float axisAreaHeightStretch = 0.1f;//坐标轴区域高度比例
	const float triggerLabelHeightStretch = 0.4f;//触发指针高度比例


	const float paddingWidthStretch = 0.02f;//坐标轴左右间距宽度比例
	const float axisWidthStretch = 0.96f;//坐标轴宽度比例

};

#endif // ACQPARAMDIAGRAM_H
