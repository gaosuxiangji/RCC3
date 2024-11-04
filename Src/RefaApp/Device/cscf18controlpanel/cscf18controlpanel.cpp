#include "cscf18controlpanel.h"
#include <QMessageBox>
#include "../deviceutils.h"

CSCF18ControlPanel::CSCF18ControlPanel(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	initVessel();
	initUI();
	bind();
	updateControlPanel();
	updateSynControlPanelStatus(false);
}

CSCF18ControlPanel::~CSCF18ControlPanel()
{}

void CSCF18ControlPanel::updateSynControlPanelStatus(bool enable)
{
	ui.channel_1_status_label->setEnabled(enable);
	ui.channel_2_status_label->setEnabled(enable);
	ui.channel_3_status_label->setEnabled(enable);
	ui.channel_4_status_label->setEnabled(enable);
	ui.channel_5_status_label->setEnabled(enable);
	ui.channel_6_status_label->setEnabled(enable);
	ui.channel_7_status_label->setEnabled(enable);
	ui.channel_8_status_label->setEnabled(enable);

	ui.g_channel_1_checkBox->setEnabled(enable);
	ui.g_channel_2_checkBox->setEnabled(enable);
	ui.g_channel_3_checkBox->setEnabled(enable);
	ui.g_channel_4_checkBox->setEnabled(enable);
	ui.g_channel_5_checkBox->setEnabled(enable);
	ui.g_channel_6_checkBox->setEnabled(enable);
	ui.g_channel_7_checkBox->setEnabled(enable);
	ui.g_channel_8_checkBox->setEnabled(enable);

	ui.g_start_btn->setEnabled(enable);
	ui.g_stop_btn->setEnabled(enable);

	ui.channel_1_radio_btn->setEnabled(enable);
	ui.channel_2_radio_btn->setEnabled(enable);
	ui.channel_3_radio_btn->setEnabled(enable);
	ui.channel_4_radio_btn->setEnabled(enable);
	ui.channel_5_radio_btn->setEnabled(enable);
	ui.channel_6_radio_btn->setEnabled(enable);
	ui.channel_7_radio_btn->setEnabled(enable);
	ui.channel_8_radio_btn->setEnabled(enable);

	ui.signal_comboBox->setEnabled(enable);
	ui.pulse_count_lineEdit->setEnabled(enable);
	ui.pulse_count_zero_btn->setEnabled(enable);

	ui.single_channel_start_btn->setEnabled(enable);
	ui.single_channel_stop_btn->setEnabled(enable);
	updateChannelStatus();
}

void CSCF18ControlPanel::updateChnStatus(const int& chnIndex, const CmdChnStatus& chnStatus)
{
	connect(m_checkBox_map[chnIndex], &QCheckBox::clicked, [=] {
		int status = m_checkBox_map[chnIndex]->isChecked() ? 1 : 0;
		m_chn_status_map[chnIndex].globalCtl = status;
		m_chn_status_map[chnIndex].channel = chnIndex;
		SetChnStatus(m_chn_status_map[chnIndex]);
		updateControlPanel();	
	});
	if (m_chn_status_map.contains(chnIndex)) {
		m_chn_status_map.remove(chnIndex);
	}
	m_chn_status_map.insert(chnIndex, chnStatus);
	updateControlPanel();
}

void CSCF18ControlPanel::initUI()
{
	setWindowFlags(Qt::FramelessWindowHint);

	m_channel_radio_group = new QButtonGroup(this);
	m_channel_radio_group->addButton(ui.channel_1_radio_btn, 0);
	m_channel_radio_group->addButton(ui.channel_2_radio_btn, 1);
	m_channel_radio_group->addButton(ui.channel_3_radio_btn, 2);
	m_channel_radio_group->addButton(ui.channel_4_radio_btn, 3);
	m_channel_radio_group->addButton(ui.channel_5_radio_btn, 4);
	m_channel_radio_group->addButton(ui.channel_6_radio_btn, 5);
	m_channel_radio_group->addButton(ui.channel_7_radio_btn, 6);
	m_channel_radio_group->addButton(ui.channel_8_radio_btn, 7);

	ui.g_stop_btn->setEnabled(false);
	
	for (int i = 0; i < 8; ++i) {
		m_channel_label_map[i]->setStyleSheet(m_const_normal_sheetStyle);
		m_channel_label_map[i]->setToolTip(tr("stopped"));
	}

	ui.signal_comboBox->setMaxCount(6);
	QStringList comboBoxList{};
	for (int i = 0; i < 6; ++i) {
		comboBoxList << DeviceUtils::getCF18SignalType(i);
	}
	ui.signal_comboBox->addItems(comboBoxList);
	ui.channel_1_radio_btn->setChecked(true);


	//信息刷新计时器
	m_CF18_info_timer_ptr = new QTimer();
	m_CF18_info_timer_ptr->setInterval(kUpdateCF18InfoInterval);
	QObject::connect(m_CF18_info_timer_ptr, &QTimer::timeout, this, &CSCF18ControlPanel::slotUpdateChannelInfo);
	m_CF18_info_timer_ptr->start();
	
}

void CSCF18ControlPanel::bind()
{
	bool ok = connect(ui.g_start_btn, &QPushButton::clicked, this, &CSCF18ControlPanel::slotGStartBtnClicked);
	ok = connect(ui.g_stop_btn, &QPushButton::clicked, this, &CSCF18ControlPanel::slotGStopBtnClicked);
	ok = connect(m_channel_radio_group, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &CSCF18ControlPanel::slotChannelRadiosGroup);
	ok = connect(ui.single_channel_start_btn, &QPushButton::clicked, this, &CSCF18ControlPanel::slotSingleStartBtnClicked);
	ok = connect(ui.single_channel_stop_btn, &QPushButton::clicked, this, &CSCF18ControlPanel::slotSingleStopBtnClicked);
	ok = connect(ui.signal_comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index) {
		if (m_chn_status_map.size() > index) {
			m_chn_status_map[m_single_radio_id].single = index;
			SetChnStatus(m_chn_status_map[m_single_radio_id]);
			updateControlPanel();
		}
	});
	ok = connect(ui.pulse_count_zero_btn, &QPushButton::clicked, [=] {
		m_chn_status_map[m_single_radio_id].outputReset = 1;
		SetChnStatus(m_chn_status_map[m_single_radio_id]);
		updateControlPanel();
	});
}

void CSCF18ControlPanel::initVessel()
{
	{//通道label
		m_channel_label_map.insert(0, ui.channel_1_status_label);
		m_channel_label_map.insert(1, ui.channel_2_status_label);
		m_channel_label_map.insert(2, ui.channel_3_status_label);
		m_channel_label_map.insert(3, ui.channel_4_status_label);
		m_channel_label_map.insert(4, ui.channel_5_status_label);
		m_channel_label_map.insert(5, ui.channel_6_status_label);
		m_channel_label_map.insert(6, ui.channel_7_status_label);
		m_channel_label_map.insert(7, ui.channel_8_status_label);
	}

	{//通道checkBox
		m_checkBox_map.insert(0, ui.g_channel_1_checkBox);
		m_checkBox_map.insert(1, ui.g_channel_2_checkBox);
		m_checkBox_map.insert(2, ui.g_channel_3_checkBox);
		m_checkBox_map.insert(3, ui.g_channel_4_checkBox);
		m_checkBox_map.insert(4, ui.g_channel_5_checkBox);
		m_checkBox_map.insert(5, ui.g_channel_6_checkBox);
		m_checkBox_map.insert(6, ui.g_channel_7_checkBox);
		m_checkBox_map.insert(7, ui.g_channel_8_checkBox);
	}

	{//通道radioButton
		m_radioButton_map.insert(0, ui.channel_1_radio_btn);
		m_radioButton_map.insert(1, ui.channel_2_radio_btn);
		m_radioButton_map.insert(2, ui.channel_3_radio_btn);
		m_radioButton_map.insert(3, ui.channel_4_radio_btn);
		m_radioButton_map.insert(4, ui.channel_5_radio_btn);
		m_radioButton_map.insert(5, ui.channel_6_radio_btn);
		m_radioButton_map.insert(6, ui.channel_7_radio_btn);
		m_radioButton_map.insert(7, ui.channel_8_radio_btn);
	}

	{//通道异常提示
		m_abnormal_tips_map.insert(0, tr("Channel1"));
		m_abnormal_tips_map.insert(1, tr("Channel2"));
		m_abnormal_tips_map.insert(2, tr("Channel3"));
		m_abnormal_tips_map.insert(3, tr("Channel4"));
		m_abnormal_tips_map.insert(4, tr("Channel5"));
		m_abnormal_tips_map.insert(5, tr("Channel6"));
		m_abnormal_tips_map.insert(6, tr("Channel7"));
		m_abnormal_tips_map.insert(7, tr("Channel8"));
	}
}

void CSCF18ControlPanel::updateChannelStatus()
{
	for (int i = 0; i < m_chn_status_map.size(); ++i) {
		if ((1 == m_chn_status_map[i].currentStatus)&&(m_channel_label_map[i]->isEnabled())) {
			m_channel_label_map[i]->setStyleSheet(m_const_green_sheetStyle);
			m_channel_label_map[i]->setToolTip(tr("running"));
		}
		else {
			m_channel_label_map[i]->setStyleSheet(m_const_normal_sheetStyle);
			m_channel_label_map[i]->setToolTip(tr("stopped"));
		}
	}
}

void CSCF18ControlPanel::updateChnParam(const int& chnId)
{
	if (m_chn_status_map[chnId].globalCtl) {
		ui.single_channel_start_btn->setEnabled(false);
		ui.single_channel_stop_btn->setEnabled(false);
	}
	else {
		if (m_chn_status_map[chnId].currentStatus) {
			ui.single_channel_start_btn->setEnabled(false);
			ui.single_channel_stop_btn->setEnabled(true);
		}
		else {
			ui.single_channel_start_btn->setEnabled(true);
			ui.single_channel_stop_btn->setEnabled(false);
		}
	}

	if (m_radioButton_map[chnId]->isChecked()) {
		ui.pulse_count_lineEdit->setText(QString::number(m_chn_status_map[chnId].outputCnt));
		ui.signal_comboBox->blockSignals(true);
		ui.signal_comboBox->setCurrentIndex(m_chn_status_map[chnId].single);
		ui.signal_comboBox->blockSignals(false);
	}
}

void CSCF18ControlPanel::updateGControlCheckBox(const int& chnId)
{
	if (m_chn_status_map[chnId].globalCtl) {
		m_checkBox_map[chnId]->setChecked(true);
	}
	else {
		m_checkBox_map[chnId]->setChecked(false);
	}

	if (m_chn_status_map[chnId].currentStatus) {
		m_checkBox_map[chnId]->setEnabled(false);
	}
	else {
		m_checkBox_map[chnId]->setEnabled(true);
	}
}

void CSCF18ControlPanel::updateControlPanel()
{
	updateChannelStatus();
	for (int i = 0; i < m_chn_status_map.size(); ++i) {
		if (m_radioButton_map[i]->isChecked()) {
			updateChnParam(i);
		}
		updateGControlCheckBox(i);
	}
	bool b_g_start_enable = false;
	bool b_g_stop_enable = false;
	for (int i = 0; i < m_chn_status_map.size(); ++i) {
		if (m_chn_status_map[i].globalCtl && !m_chn_status_map[i].currentStatus) {
			b_g_start_enable = true;
		}

		if (m_chn_status_map[i].globalCtl && m_chn_status_map[i].currentStatus) {
			b_g_stop_enable = true;
		}
	}
	ui.g_start_btn->setEnabled(b_g_start_enable);
	ui.g_stop_btn->setEnabled(b_g_stop_enable);
}

void CSCF18ControlPanel::slotChannelRadiosGroup(int id)
{
	m_single_radio_id = id;
	slotUpdateChannelInfo();
}

void CSCF18ControlPanel::slotSingleStartBtnClicked()
{
	if (SetChannelSwitch(0, m_single_radio_id, 1)) {
		CmdChnStatus current_status{};
		GetChnStatus(m_single_radio_id, current_status);
		m_chn_status_map[m_single_radio_id] = current_status;
	}
	else {
		QString message = tr("The network connection is abnormal,\nand this channel cannot be started!");
		QMessageBox::critical(this, QObject::tr("RCC"), message, QMessageBox::Yes);
		return;
	}
	updateControlPanel();
}

void CSCF18ControlPanel::slotSingleStopBtnClicked()
{
	if (SetChannelSwitch(0, m_single_radio_id, 0)) {
		CmdChnStatus current_status{};
		GetChnStatus(m_single_radio_id, current_status);
		m_chn_status_map[m_single_radio_id] = current_status;
		updateControlPanel();
	}
}

void CSCF18ControlPanel::slotUpdateChannelInfo()
{
	CmdChnStatus current_status{};
	if (GetChnStatus(m_single_radio_id, current_status))
	{
		m_chn_status_map[m_single_radio_id] = current_status;
		updateControlPanel();
	}

}

void CSCF18ControlPanel::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		for (int i = 0; i < ui.signal_comboBox->count(); ++i) {
			ui.signal_comboBox->setItemText(i, DeviceUtils::getCF18SignalType(i));
		}

		ui.retranslateUi(this);
	}

	QWidget::changeEvent(event);
}

void CSCF18ControlPanel::slotGStartBtnClicked()
{
	bool start_channel = false;
	std::vector<int>start_fail_channels{};

	if (SetChannelSwitch(1, 0, 1)) {
		for (int i = 0; i < m_chn_status_map.size(); ++i) {
			CmdChnStatus current_status{};
			if (GetChnStatus(i, current_status)) {
				m_chn_status_map[i] = current_status;
			}
		}
	}

	for (int i = 0; i < m_chn_status_map.size(); ++i) {
		if (m_chn_status_map[i].globalCtl && !m_chn_status_map[i].currentStatus) {
			start_fail_channels.push_back(i);
		}
	}

	updateControlPanel();

	if (start_fail_channels.size() > 0) {
		QString message = tr("The network connection is abnormal,\nand the following channels cannot be started!");
			for (auto item : start_fail_channels) {
				message += ("\n" + m_abnormal_tips_map[item]);
			}
			QMessageBox::critical(this, QObject::tr("RCC"), message, QMessageBox::Yes);
	}
}

void CSCF18ControlPanel::slotGStopBtnClicked()
{
	if (SetChannelSwitch(1, 0, 0)) {
		for (int i = 0; i < m_chn_status_map.size(); ++i) {
			CmdChnStatus current_status{};
			if (GetChnStatus(i, current_status)) {
				m_chn_status_map[i] = current_status;
			}
		}
	}
	updateControlPanel();
}
