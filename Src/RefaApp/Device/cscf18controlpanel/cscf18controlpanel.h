#ifndef CSCF18CONTROLPANEL_H
#define CSCF18CONTROLPANEL_H


#include <QtWidgets/QWidget>
#include "ui_cscf18controlpanel.h"
#include <QMap>
#include <QButtonGroup>
#include <QKeyEvent>
#include <QPointer>
#include <QTimer>
#include "../CF18/CF18SyncTrigger.h"

class CSCF18ControlPanel : public QWidget
{
	Q_OBJECT
public:
	CSCF18ControlPanel(QWidget *parent = Q_NULLPTR);
	~CSCF18ControlPanel();

	void updateSynControlPanelStatus(bool enable);
	void updateChnStatus(const int& chnIndex, const CmdChnStatus& chnStatus);
private:
	void initUI();
	void bind();
	void initVessel();
	void updateChannelStatus();
	void updateChnParam(const int& chnId);
	void updateGControlCheckBox(const int& chnId);
	void updateControlPanel();
private slots:
	void slotGStartBtnClicked();
	void slotGStopBtnClicked();

	void slotChannelRadiosGroup(int id);

	void slotSingleStartBtnClicked();
	void slotSingleStopBtnClicked();
	void slotUpdateChannelInfo();

protected:
	/**
	*@brief 变化事件
	*@param [in] : * event : QEvent，事件指针
	**/
	void changeEvent(QEvent *event) override;
private:
	const QString m_const_green_sheetStyle = "QLabel{border-radius:9px;border:1px solid black;background:#33FF00;color:white;}";
	const QString m_const_normal_sheetStyle = "QLabel{border-radius:9px;border:1px solid black;background:transparent;color:black;}";
	const QString m_const_running{ tr("running") };
	const QString m_const_stop{ tr("stopped") };

	QMap<int, QLabel*>m_channel_label_map{};
	QMap<int, QCheckBox*>m_checkBox_map{};
	QMap<int, QRadioButton*>m_radioButton_map{};
	QMap<int, QString> m_abnormal_tips_map{};
	QButtonGroup* m_channel_radio_group{};
	QMap<int, CmdChnStatus> m_chn_status_map{};



	//CF18相关
	const int kUpdateCF18InfoInterval{ 2000 }; // 更新CF18信息间隔
	QPointer<QTimer> m_CF18_info_timer_ptr; //CF18更新定时器

	int m_single_radio_id{ 0 };
private:
	Ui::CSCF18ControlPanelClass ui;
};
#endif // CSCF18CONTROLPANEL_H
