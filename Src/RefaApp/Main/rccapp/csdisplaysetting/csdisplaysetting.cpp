#include "csdisplaysetting.h"
#include "ui_csdisplaysetting.h"
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QStyle>
#include <QMessageBox>

CSDisplaySetting::CSDisplaySetting(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CSDisplaySetting)
{
    ui->setupUi(this);
	InitUI();
	ConnectSignalAndSlot();
}

CSDisplaySetting::~CSDisplaySetting()
{
	m_bSpeedInit = false;
	m_bRsolutionInit = false;
    delete ui;
}

void CSDisplaySetting::SDIVisible(bool bShow)
{
	ui->sdi_playback_title_pushButton->setVisible(bShow);
	HideSDIPlaybackItem(bShow);
}

void CSDisplaySetting::SetColorRevertCheckBoxStatus(const bool bChecked)
{
	ui->color_revert_checkBox->setChecked(bChecked);
}

void CSDisplaySetting::SetluminanceValue(const int value)
{
	ui->luminance_horizontalSlider->setValue(value);
}

void CSDisplaySetting::SetContrastValue(const int value)
{
	ui->contrast_horizontalSlider->setValue(value);
}

void CSDisplaySetting::SetSdiCurrentStatus(const bool bOpened)
{
	QString strStatus = bOpened ? tr("Opened!") : tr("Closed!");
	ui->sdi_current_status_value_label->setText(strStatus);
}

void CSDisplaySetting::SetSdiButtonStatus(bool bOpened)
{
	ui->forward_frame_pushButton->setEnabled(bOpened);
	ui->play_pause_pushButton->setEnabled(bOpened);
	ui->backward_frame_pushButton->setEnabled(bOpened);

	ui->start_sdi_pushButton->setChecked(bOpened);
	QString strText = bOpened ? tr("end") : tr("start");
	ui->start_sdi_pushButton->setText(strText);

	style()->unpolish(ui->sdi_current_status_value_label);
	bOpened ? ui->sdi_current_status_value_label->setObjectName("Opened") : \
		ui->sdi_current_status_value_label->setObjectName("UnOpened");
	style()->polish(ui->sdi_current_status_value_label);

	ui->play_pause_pushButton->setChecked(bOpened);
	on_play_pause_pushButton_clicked(bOpened);
}

void CSDisplaySetting::SetSdiFpsResolsList(const QList<QString>& strList)
{
	for (auto &item : strList)
	{
		ui->resolution_displayRate_comboBox->addItem(item);
	}
}

void CSDisplaySetting::SetSdiDisplaySpeeds(const QStringList& strList)
{
	ui->speed_comboBox->addItems(strList);
}

void CSDisplaySetting::SetCurrentSdiFpsResol(const int index)
{
	ui->resolution_displayRate_comboBox->setCurrentIndex(index);
}

void CSDisplaySetting::SetCurrentSdiDisplaySpeed(const QString & strSpeed)
{
	m_strCurrentSpeed = strSpeed;
	ui->speed_comboBox->setCurrentText(strSpeed);
}

void CSDisplaySetting::SetSecondScreenBtnStatus(const bool bChecked)
{
	QString strTip = bChecked ? tr("Stop Secondary Screen") : tr("Start Secondary Screen");
	ui->start_secondary_screen_pushButton->setText(strTip);
	ui->start_secondary_screen_pushButton->setChecked(bChecked);
	SetSecondScreenStatus(bChecked);
}

void CSDisplaySetting::ClearSdiFpsResolsList()
{
	ui->resolution_displayRate_comboBox->clear();
}

void CSDisplaySetting::ClearSdiDisplaySpeedsList()
{
	ui->speed_comboBox->clear();
}

void CSDisplaySetting::ResetParams()
{
	m_bSpeedInit = false;
	m_bRsolutionInit = false;
}

void CSDisplaySetting::SetSecondScreenStatus(const bool bOpened)
{
	QString strStatus = bOpened ? tr("Opened!") : tr("Closed!");
	ui->secondary_screen_current_status_value_label->setText(strStatus);

	style()->unpolish(ui->secondary_screen_current_status_value_label);
	bOpened ? ui->secondary_screen_current_status_value_label->setObjectName("Opened") : \
		ui->secondary_screen_current_status_value_label->setObjectName("UnOpened");
	style()->polish(ui->secondary_screen_current_status_value_label);
}

void CSDisplaySetting::UpadateFrameBtnStatus(const bool bForbid)
{
	ui->forward_frame_pushButton->setDisabled(bForbid);
	ui->backward_frame_pushButton->setDisabled(bForbid);
}

void CSDisplaySetting::SlotLumSliderReleased()
{
	emit SignalLuminanceChanged(ui->luminance_horizontalSlider->value());
}

void CSDisplaySetting::SlotContrastSliderReleased()
{
	emit SignalContrastChanged(ui->contrast_horizontalSlider->value());
}

void CSDisplaySetting::on_image_process_pushButton_clicked(bool checked)
{
	style()->unpolish(m_labelImgProcessTitleIcon);
	checked ? m_labelImgProcessTitleIcon->setObjectName("TitleIconChecked") : m_labelImgProcessTitleIcon->setObjectName("TitleIcon");
	style()->polish(m_labelImgProcessTitleIcon);
	HideImageProcessItem(checked);
}

void CSDisplaySetting::InitUI()
{
	//去除标题框
	setWindowFlags(Qt::FramelessWindowHint);

	ui->image_process_pushButton->setObjectName("TitleButton");
	ui->sdi_playback_title_pushButton->setObjectName("TitleButton");
	ui->secondary_screen_title_pushButton->setObjectName("TitleButton");

	m_labelImgProcessTitleText = new QLabel(ui->image_process_pushButton);
	m_labelImgProcessTitleText->setObjectName("TitleText");
	m_labelImgProcessTitleText->setText(tr("Image Process"));
	m_labelImgProcessTitleIcon = new QLabel(ui->image_process_pushButton);
	m_labelImgProcessTitleIcon->setFixedSize(m_labelImgProcessTitleText->height()/3, m_labelImgProcessTitleText->height()/3);
	m_labelImgProcessTitleIcon->setObjectName("TitleIconChecked");
	m_labelSDITitleText = new QLabel(ui->sdi_playback_title_pushButton);
	m_labelSDITitleText->setObjectName("TitleText");
	m_labelSDITitleText->setText(tr("SDI Playback"));
	m_labelSDITitleIcon = new QLabel(ui->sdi_playback_title_pushButton);
	m_labelSDITitleIcon->setFixedSize(m_labelSDITitleText->height() / 3, m_labelSDITitleText->height() / 3);
	m_labelSDITitleIcon->setObjectName("TitleIconChecked");
	m_labelSecondaryScreenTitleText = new QLabel(ui->secondary_screen_title_pushButton);
	m_labelSecondaryScreenTitleText->setObjectName("TitleText");
	m_labelSecondaryScreenTitleText->setText(tr("Show Secondary Screen"));
	m_labelSecondaryScreenTitleIcon = new QLabel(ui->secondary_screen_title_pushButton);
	m_labelSecondaryScreenTitleIcon->setFixedSize(m_labelSecondaryScreenTitleText->height() / 3, m_labelSecondaryScreenTitleText->height() / 3);
	m_labelSecondaryScreenTitleIcon->setObjectName("TitleIconChecked");

	
	QHBoxLayout* layoutImgProcess = new QHBoxLayout(ui->image_process_pushButton);
	layoutImgProcess->setContentsMargins(6, 0, 6, 0);
	layoutImgProcess->setSpacing(0);
	ui->image_process_pushButton->setLayout(layoutImgProcess);
	layoutImgProcess->addWidget(m_labelImgProcessTitleText);
	QSpacerItem* vItemImg = new QSpacerItem(40, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
	layoutImgProcess->addSpacerItem(vItemImg);
	layoutImgProcess->addWidget(m_labelImgProcessTitleIcon);
	

	QHBoxLayout* layoutSDI = new QHBoxLayout(ui->sdi_playback_title_pushButton);
	layoutSDI->setContentsMargins(6, 0, 6, 0);
	layoutSDI->setSpacing(0);
	ui->sdi_playback_title_pushButton->setLayout(layoutSDI);
	layoutSDI->addWidget(m_labelSDITitleText);
	QSpacerItem* vItemSDI= new QSpacerItem(40, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
	layoutSDI->addSpacerItem(vItemSDI);
	layoutSDI->addWidget(m_labelSDITitleIcon);

	QHBoxLayout* layoutSecondaryScreen = new QHBoxLayout(ui->secondary_screen_title_pushButton);
	layoutSecondaryScreen->setContentsMargins(6, 0, 6, 0);
	layoutSecondaryScreen->setSpacing(0);
	ui->secondary_screen_title_pushButton->setLayout(layoutSecondaryScreen);
	layoutSecondaryScreen->addWidget(m_labelSecondaryScreenTitleText);
	QSpacerItem* vItemSecodary = new QSpacerItem(40, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
	layoutSecondaryScreen->addSpacerItem(vItemSecodary);
	layoutSecondaryScreen->addWidget(m_labelSecondaryScreenTitleIcon);

	ui->secondary_screen_current_status_name_label->setText(QObject::tr("<font color = red>%1</font>").arg("*") + tr("Current Status::"));
	ui->sdi_current_status_name_label->setText(QObject::tr("<font color = red>%1</font>").arg("*") + tr("Current Status::"));

	//亮度滑动条
	ui->luminance_horizontalSlider->setMinimum(0);    //最小值
	ui->luminance_horizontalSlider->setMaximum(100);    //最大值
	ui->luminance_horizontalSlider->setSingleStep(1);    //步长

	//对比度滑动条
	ui->contrast_horizontalSlider->setMinimum(0);    //最小值
	ui->contrast_horizontalSlider->setMaximum(100);    //最大值
	ui->contrast_horizontalSlider->setSingleStep(1);    //步长

	QIntValidator* IntValidator = new QIntValidator;
	//亮度滑动条编辑框
	ui->luminance_lineEdit->setValidator(IntValidator);
	ui->luminance_lineEdit->setText(QString::number(0));
	//对比度滑动条编辑框
	ui->contrast_lineEdit->setValidator(IntValidator);
	ui->contrast_lineEdit->setText(QString::number(0));

	ui->play_pause_pushButton->setObjectName("SDIPlayPauseControl");
	ui->play_pause_pushButton->setToolTip(tr("Play"));

	ui->sdi_current_status_value_label->setText(tr("Closed!"));
	ui->secondary_screen_current_status_value_label->setText(tr("Closed!"));

	ui->sdi_current_status_value_label->setObjectName("UnOpened");
	ui->secondary_screen_current_status_value_label->setObjectName("UnOpened");
	ui->sdi_current_status_name_label->setObjectName("UnOpened");
	ui->secondary_screen_current_status_name_label->setObjectName("UnOpened");

	ui->start_sdi_pushButton->setObjectName("PlaySettingBtn");
	ui->start_secondary_screen_pushButton->setObjectName("PlaySettingBtn");
}

void CSDisplaySetting::HideImageProcessItem(bool checked)
{
	ui->color_revert_checkBox->setVisible(checked);
	ui->luminance_label->setVisible(checked);
	ui->luminance_horizontalSlider->setVisible(checked);
	ui->luminance_lineEdit->setVisible(checked);
	ui->contrast_label->setVisible(checked);
	ui->contrast_horizontalSlider->setVisible(checked);
	ui->contrast_lineEdit->setVisible(checked);
}

void CSDisplaySetting::HideSDIPlaybackItem(bool checked)
{
	ui->sdi_playback_control_groupBox->setVisible(checked);
	ui->start_sdi_pushButton->setVisible(checked);
	ui->sdi_current_status_name_label->setVisible(checked);
	ui->sdi_current_status_value_label->setVisible(checked);
}

void CSDisplaySetting::HideSecodaryScreenItem(bool checked)
{
	ui->secondary_screen_info_groupBox->setVisible(checked);
	ui->start_secondary_screen_pushButton->setVisible(checked);
	ui->secondary_screen_current_status_name_label->setVisible(checked);
	ui->secondary_screen_current_status_value_label->setVisible(checked);
}

void CSDisplaySetting::ConnectSignalAndSlot()
{
	//bool ok = connect(ui->luminance_horizontalSlider, &QSlider::sliderReleased, this, &CSDisplaySetting::SlotLumSliderReleased, Qt::QueuedConnection);
	//ok = connect(ui->contrast_horizontalSlider, &QSlider::sliderReleased, this, &CSDisplaySetting::SlotContrastSliderReleased, Qt::QueuedConnection);
	//Q_UNUSED(ok);
}

void CSDisplaySetting::on_sdi_playback_title_pushButton_clicked(bool checked)
{
	style()->unpolish(m_labelSDITitleIcon);
	checked ? m_labelSDITitleIcon->setObjectName("TitleIconChecked") : m_labelSDITitleIcon->setObjectName("TitleIcon");
	style()->polish(m_labelSDITitleIcon);
	HideSDIPlaybackItem(checked);
}

void CSDisplaySetting::on_secondary_screen_title_pushButton_clicked(bool checked)
{
	style()->unpolish(m_labelSecondaryScreenTitleIcon);
	checked ? m_labelSecondaryScreenTitleIcon->setObjectName("TitleIconChecked") : m_labelSecondaryScreenTitleIcon->setObjectName("TitleIcon");
	style()->polish(m_labelSecondaryScreenTitleIcon);
	HideSecodaryScreenItem(checked);
}

void CSDisplaySetting::on_color_revert_checkBox_clicked(bool checked)
{
	emit SignalRevertCheckBoxStatusChanged(checked);
}

void CSDisplaySetting::on_luminance_horizontalSlider_valueChanged(int value)
{
	ui->luminance_lineEdit->setText(QString::number(value));
	SlotLumSliderReleased();
}

void CSDisplaySetting::on_luminance_lineEdit_editingFinished()
{
	ui->luminance_horizontalSlider->setValue(ui->luminance_lineEdit->text().toInt());
}

void CSDisplaySetting::on_contrast_lineEdit_editingFinished()
{
	ui->contrast_horizontalSlider->setValue(ui->contrast_lineEdit->text().toInt());
}

void CSDisplaySetting::on_resolution_displayRate_comboBox_currentIndexChanged(const QString &arg1)
{

}

void CSDisplaySetting::on_forward_frame_pushButton_clicked()
{
	emit SignalSdiForwardStep();
}

void CSDisplaySetting::on_play_pause_pushButton_clicked(bool checked)
{
	if (ui->start_sdi_pushButton->isChecked())
	{
		UpadateFrameBtnStatus(checked);
	}
	QString strTip = checked ? tr("Pause") : tr("Play");
	ui->play_pause_pushButton->setToolTip(strTip);
	emit SignalSdiDisplayControl(checked);
}

void CSDisplaySetting::on_backward_frame_pushButton_clicked()
{
	emit SignalSdiBackwardStep();
}

void CSDisplaySetting::on_speed_comboBox_currentIndexChanged(const QString &arg1)
{
	if (!m_bSpeedInit)
	{
		m_bSpeedInit = true;
		return;
	}
	m_strCurrentSpeed = arg1;
	emit SignalSdiSpeedValue(arg1.toDouble());
}

void CSDisplaySetting::on_start_sdi_pushButton_clicked(bool checked)
{
	if(!checked)
	{
		QMessageBox infoBox(this);
		infoBox.setWindowTitle(POPUP_TITLE);
		infoBox.addButton(tr("OK"), QMessageBox::YesRole);
		infoBox.addButton(tr("NO"), QMessageBox::NoRole);
		infoBox.setText(tr("Exit SDI playback?\nAfter clicking OK,when exiting the current SDI playback"));
		if (1 == infoBox.exec())
		{
			ui->start_sdi_pushButton->setChecked(true);
			return;
		}
	}

	QString strTip = checked ? tr("end") : tr("start");
	ui->start_sdi_pushButton->setText(strTip);
	emit SignalSdiSwitchControl(checked,m_strCurrentSpeed.toDouble());
}

void CSDisplaySetting::on_start_secondary_screen_pushButton_clicked(bool checked)
{
	if (!checked)
	{
		QMessageBox infoBox(this);
		infoBox.setWindowTitle(POPUP_TITLE);
		infoBox.addButton(tr("OK"), QMessageBox::YesRole);
		infoBox.addButton(tr("NO"), QMessageBox::NoRole);
		infoBox.setText(tr("Exit screen cast?\nAfter clicking OK,will exit the current projection display"));
		if (1 == infoBox.exec())
		{
			ui->start_secondary_screen_pushButton->setChecked(true);
			return;
		}
	}
	
	QString strTip = checked ? tr("Stop Secondary Screen") : tr("Start Secondary Screen");
	ui->start_secondary_screen_pushButton->setText(strTip);
	SetSecondScreenStatus(checked);
	emit SignalSdiSecondScreen(checked);
}

void CSDisplaySetting::on_contrast_horizontalSlider_valueChanged(int value)
{
	ui->contrast_lineEdit->setText(QString::number(value));
	SlotContrastSliderReleased();
}

void CSDisplaySetting::on_resolution_displayRate_comboBox_currentIndexChanged(int index)
{
	if (!m_bRsolutionInit)
	{
		m_bRsolutionInit = true;
		return;
	}
	emit SignalSdiFpsResolsListIndex(index);
}

void CSDisplaySetting::on_speed_comboBox_currentIndexChanged(int index)
{

}

void CSDisplaySetting::on_luminance_lineEdit_textChanged(const QString &arg1)
{
	int value = arg1.toInt();
	value = (value > 100) ? 100 : value;
	ui->luminance_lineEdit->setText(QString::number(value));
}

void CSDisplaySetting::on_contrast_lineEdit_textChanged(const QString &arg1)
{
	int value = arg1.toInt();
	value = (value > 100) ? 100 : value;
	ui->contrast_lineEdit->setText(QString::number(value));
}
