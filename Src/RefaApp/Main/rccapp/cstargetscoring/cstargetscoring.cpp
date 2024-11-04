#include "cstargetscoring.h"
#include "ui_cstargetscoring.h"

CSTargetScoring::CSTargetScoring(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CSTargetScoring)
{
    ui->setupUi(this);
	InitUI();
}

CSTargetScoring::~CSTargetScoring()
{
    delete ui;
}

void CSTargetScoring::UpdateTargetScoringSettingStatus(const bool enabled)
{
	SetMeasuringGridStatus(enabled);
	SetPosManualSelectBtnEnabled(enabled);
	SetGridSliderStatus(enabled);
	SetTargetManualSelectBtnEnabled(enabled);
	SetOverlayImageStatus(enabled);
}

void CSTargetScoring::SetGridSliderInterval(const int interval)
{
	ui->grid_interval_horizontalSlider->setValue(interval);
}

void CSTargetScoring::SetGridSliderIntervalRange(const int iMin, const int iMax)
{
	ui->grid_interval_horizontalSlider->setMinimum(iMin); 
	ui->grid_interval_horizontalSlider->setMaximum(iMax);
}

void CSTargetScoring::SetPosManualSelectBtnEnabled(bool bEnable)
{
	ui->manual_select_targetPosition_pushButton->setEnabled(bEnable);
	ui->pos_tip_label->hide();
}

void CSTargetScoring::SetTargetManualSelectBtnEnabled(bool bEnable)
{
	ui->manual_select_targetScoring_pushButton->setEnabled(bEnable);
	ui->scoring_tip_label->hide();
}

void CSTargetScoring::SetTargetScoringInfo(const QPointF & fallPoint, const float & dis)
{
	QString strPos = QString("(%1,%2)").arg(fallPoint.x()).arg(fallPoint.y());
	ui->fallPosition_coordinate_value_label->setText(strPos);

	QString strDis = QString("%1m").arg(dis);
	ui->fallPosition_distance_value_label->setText(strDis);
}

void CSTargetScoring::SetScoringTargetResultExit(const bool bExit)
{
	ui->common_info_label->setText(NOT_TARGET_POS);
	SetTargetInfoLabelVisible(bExit);
	ui->common_info_label->setVisible(!bExit);
	ui->overlay_image_checkBox->setEnabled(bExit);
	if (ui->overlay_image_checkBox->isChecked())
	{
		on_overlay_image_checkBox_clicked(true);
	}
}

void CSTargetScoring::SetCalFileExit(const bool bExit)
{
	SetTargetInfoLabelVisible(bExit);
	if (!bExit)
	{
		ui->common_info_label->setText(NOT_CALFILE);
	}
	
}

void CSTargetScoring::on_measuring_grid_checkBox_clicked(bool checked)
{
	emit SignalMeasuringGridVisible(checked);
}

void CSTargetScoring::InitUI()
{
	//去除标题框
	setWindowFlags(Qt::FramelessWindowHint);
	ui->targetInfo_widget->setStyleSheet("background-color:white;");

	//网格间距滑动条-先给一个默认值
	ui->grid_interval_horizontalSlider->setMinimum(0);    //最小值
	ui->grid_interval_horizontalSlider->setMaximum(100);    //最大值
	ui->grid_interval_horizontalSlider->setSingleStep(1);    //步长

	m_intValidator = new QIntValidator;
	ui->grid_interval_lineEdit->setValidator(m_intValidator);

	ui->common_info_label->setText(NOT_TARGET_POS);

	
	ui->pos_tip_label->setText(QObject::tr("<font color = red>%1</font>").arg("*") + tr("Please click the target location in the large image on the left!"));
	ui->scoring_tip_label->setText(QObject::tr("<font color = red>%1</font>").arg("*") + tr("Please click on the drop location in the large image on the left!"));
	ui->pos_tip_label->setVisible(false);
	ui->scoring_tip_label->setVisible(false);

	ui->common_info_label->setObjectName("CommonInfo");
	ui->fallPosition_coordinate_value_label->setObjectName("ScoringValue");
	ui->fallPosition_coordinate_name_label->setObjectName("ScoringName");
	ui->fallPosition_distance_value_label->setObjectName("ScoringValue");
	ui->fallPosition_distance_name_label->setObjectName("ScoringName");

	SetTargetInfoLabelVisible(false);
}

void CSTargetScoring::SetMeasuringGridStatus(const bool enabled)
{
	ui->measuring_grid_checkBox->setEnabled(enabled);
}

void CSTargetScoring::SetGridSliderStatus(const bool enabled)
{
	ui->grid_interval_horizontalSlider->setEnabled(enabled);
	ui->grid_interval_lineEdit->setEnabled(enabled);
}

void CSTargetScoring::SetOverlayImageStatus(const bool enabled)
{
	ui->overlay_image_checkBox->setEnabled(enabled);
	if (ui->overlay_image_checkBox->isChecked())
	{
		on_overlay_image_checkBox_clicked(true);
	}
}

void CSTargetScoring::SetTargetInfoLabelVisible(bool bVisible)
{
	ui->fallPosition_coordinate_value_label->setVisible(bVisible);
	ui->fallPosition_coordinate_name_label->setVisible(bVisible);
	ui->fallPosition_distance_value_label->setVisible(bVisible);
	ui->fallPosition_distance_name_label->setVisible(bVisible);
}

void CSTargetScoring::on_grid_interval_lineEdit_editingFinished()
{
	int value = ui->grid_interval_lineEdit->text().toInt();
	ui->grid_interval_horizontalSlider->setValue(value);
}

void CSTargetScoring::on_overlay_image_checkBox_clicked(bool checked)
{
	emit SignalOverlaImageVisible(checked);
}

void CSTargetScoring::on_grid_interval_horizontalSlider_valueChanged(int value)
{
	ui->grid_interval_lineEdit->setText(QString::number(value));
	emit SignalGridIntervalChanged(value);
}

void CSTargetScoring::on_manual_select_targetPosition_pushButton_clicked()
{
	SetPosManualSelectBtnEnabled(false);
	SetTargetManualSelectBtnEnabled(false);
	emit SignalPositionSelect();
	ui->pos_tip_label->setVisible(true);
}

void CSTargetScoring::on_manual_select_targetScoring_pushButton_clicked()
{
	SetTargetManualSelectBtnEnabled(false);
	SetPosManualSelectBtnEnabled(false);
	emit SignalTargetSelect();
	ui->scoring_tip_label->setVisible(true);
}

void CSTargetScoring::on_grid_interval_lineEdit_textChanged(const QString &arg1)
{
	int value = arg1.toInt();
	value = (value > 100) ? 100 : value;
	ui->grid_interval_lineEdit->setText(QString::number(value));
}
