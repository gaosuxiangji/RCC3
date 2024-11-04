#include "cslowlightmode.h"
#include <QMessageBox>
#include <QRegExpValidator>

CSLowLightMode::CSLowLightMode(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	initUI();
	bind();
}

void CSLowLightMode::setlightValue(int value)
{
	ui.light_slider->setValue(value);
	ui.light_lineEdit->setText(QString::number(value));
}


void CSLowLightMode::slotCameraDisconnected()
{
	QMessageBox criticalBox(this);
	criticalBox.setWindowTitle(QObject::tr("RCC"));
	criticalBox.addButton(tr("OK"), QMessageBox::YesRole);
	criticalBox.setIcon(QMessageBox::Critical);
	criticalBox.setText(tr("The camera is disconnected,please check this camera!"));
	criticalBox.exec();

	reject();
}

void CSLowLightMode::initUI()
{
	setWindowFlags(Qt::WindowCloseButtonHint);
	setTitleName(tr("Low Light Mode"));

	//滑块
	ui.light_slider->setMinimum(m_const_value_min);    //最小值
	ui.light_slider->setMaximum(m_const_value_max);    //最大值
	ui.light_slider->setSingleStep(1);    //步长

	QRegExp regx("[0-9]{1,5}");
	QRegExpValidator* validator = new QRegExpValidator(regx);
	ui.light_lineEdit->setValidator(validator);

	//暂时去除勾选框
	ui.open_checkBox->setVisible(false);
}

void CSLowLightMode::bind()
{
	bool ok = connect(ui.light_lineEdit, &QLineEdit::editingFinished, this, &CSLowLightMode::slotLightLineEditFinished);
	ok = connect(ui.light_slider, &QSlider::sliderMoved, this, &CSLowLightMode::slotLightSliderMoved);
	ok = connect(ui.light_slider, &QSlider::valueChanged, this, &CSLowLightMode::signalLowLightValue);
}

void CSLowLightMode::setTitleName(const QString& title_name)
{
	this->setWindowTitle(title_name);
}


void CSLowLightMode::slotLightLineEditFinished()
{
	int iThrehold = ui.light_lineEdit->text().toInt();

	if (iThrehold > m_const_value_max)
	{
		iThrehold = m_const_value_max;
		ui.light_lineEdit->setText(QString::number(iThrehold));
	}
	else if (iThrehold < m_const_value_min)
	{
		iThrehold = m_const_value_min;
		ui.light_lineEdit->setText(QString::number(iThrehold));
	}
	else
	{
		ui.light_lineEdit->setText(QString::number(iThrehold));
	}

	ui.light_slider->setValue(iThrehold);
}

void CSLowLightMode::slotLightSliderMoved(int value)
{
	ui.light_lineEdit->setText(QString::number(value));
}

