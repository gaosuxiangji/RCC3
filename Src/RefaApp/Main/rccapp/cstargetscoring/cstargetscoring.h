#ifndef CSTARGETSCORING_H
#define CSTARGETSCORING_H

#include <QWidget>
#include <QIntValidator>

namespace Ui {
class CSTargetScoring;
}

class CSTargetScoring : public QWidget
{
    Q_OBJECT
#define NOT_CALFILE	(tr("Please add clibration file!"))
#define NOT_TARGET_POS (tr("Please select the drop location manually!"))
public:
    explicit CSTargetScoring(QWidget *parent = 0);
    ~CSTargetScoring();
public:
	/**************************
	* @brief: 更新报靶设置状态
	* @param:enabled true-可用 false-禁用
	* @return:
	* @author:mpp
	* @date:2022/06/09
	***************************/
	void UpdateTargetScoringSettingStatus(const bool enabled);

	/**************************
	* @brief: 设置滑动条值
	* @param:interval 间距
	* @return:
	* @author:mpp
	* @date:2022/06/09
	***************************/
	void SetGridSliderInterval(const int interval);

	/**************************
	* @brief: 设置滑动条值范围
	* @param:iMin-最小值 iMax-最大值
	* @return:
	* @author:mpp
	* @date:2022/06/09
	***************************/
	void SetGridSliderIntervalRange(const int iMin, const int iMax);

	/**************************
	* @brief: 设置目标位置-手动选择按钮状态
	* @param: bEnalbe true-启用 false-禁用
	* @return:
	* @author:mpp
	* @date:2022/06/10
	***************************/
	void SetPosManualSelectBtnEnabled(bool bEnable);

	/**************************
	* @brief: 设置报靶-手动选择按钮状态
	* @param: bEnalbe true-启用 false-禁用
	* @return:
	* @author:mpp
	* @date:2022/06/10
	***************************/
	void SetTargetManualSelectBtnEnabled(bool bEnable);

	/**************************
	* @brief: 设置落点位置信息
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/10
	***************************/
	void SetTargetScoringInfo(const QPointF & fallPoint, const float & dis);

	/**************************
	* @brief: 设置报靶结果是否存在 
	* @param: bExit true-存在  false-不存在
	* @return:
	* @author:mpp
	* @date:2022/06/10
	***************************/
	void SetScoringTargetResultExit(const bool bExit);

	/**************************
	* @brief: 设置标定文件是否存在
	* @param: bExit true-存在  false-不存在
	* @return:
	* @author:mpp
	* @date:2022/06/10
	***************************/
	void SetCalFileExit(const bool bExit);
private:
	/**************************
	* @brief: 初始化UI
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/27
	***************************/
	void InitUI();

	/**************************
	* @brief: 设置测量网格复选框状态
	* @param:enabled true-可用 false-禁用
	* @return:
	* @author:mpp
	* @date:2022/06/09
	***************************/
	void SetMeasuringGridStatus(const bool enabled);

	/**************************
	* @brief: 设置网格间距状态
	* @param:enabled true-可用 false-禁用
	* @return:
	* @author:mpp
	* @date:2022/06/09
	***************************/
	void SetGridSliderStatus(const bool enabled);

	/**************************
	* @brief: 设置叠加至图像复选框状态
	* @param:enabled true-可用 false-禁用
	* @return:
	* @author:mpp
	* @date:2022/06/09
	***************************/
	void SetOverlayImageStatus(const bool enabled);

	/**************************
	* @brief: 设置落点信息label是否显示
	* @param:bVisible true-显示 false-不显示
	* @return:
	* @author:mpp
	* @date:2022/06/10
	***************************/
	void SetTargetInfoLabelVisible(bool bVisible);
signals:
	/**************************
	* @brief: 测量网格显隐控制信号
	* @param: bShow true-显示 false-隐藏
	* @return:
	* @author:mpp
	* @date:2022/06/10
	***************************/
	void SignalMeasuringGridVisible(const bool bShow);

	/**************************
	* @brief: 目标位置手动选择
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/10
	***************************/
	void SignalPositionSelect();

	/**************************
	* @brief: 报靶手动选择
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/10
	***************************/
	void SignalTargetSelect();

	/**************************
	* @brief: 叠加图像显示信号
	* @param:bShow true-显示叠加图像  false-不显示叠加图像
	* @return:
	* @author:mpp
	* @date:2022/06/10
	***************************/
	void SignalOverlaImageVisible(const bool bShow);

	/**************************
	* @brief: 网格间距变化信号
	* @param:interval -变化后的间距值
	* @return:
	* @author:mpp
	* @date:2022/06/10
	***************************/
	void SignalGridIntervalChanged(const int interval);
private slots:
    void on_measuring_grid_checkBox_clicked(bool checked);

    void on_grid_interval_lineEdit_editingFinished();

    void on_overlay_image_checkBox_clicked(bool checked);

    void on_grid_interval_horizontalSlider_valueChanged(int value);

    void on_manual_select_targetPosition_pushButton_clicked();

    void on_manual_select_targetScoring_pushButton_clicked();

    void on_grid_interval_lineEdit_textChanged(const QString &arg1);

private:
	QIntValidator* m_intValidator;
private:
    Ui::CSTargetScoring *ui;
};

#endif // CSTARGETSCORING_H
