#include "csdlggainsetting.h"
#include "ui_csdlggainsetting.h"
#include "Device/device.h"
#include "Device/deviceutils.h"
#include <QMessageBox>

CSDlgGainSetting::CSDlgGainSetting(QSharedPointer<Device> device_ptr, QWidget *parent) :
    QDialog(parent),
	m_device_ptr(device_ptr),
    ui(new Ui::CSDlgGainSetting)
{
    ui->setupUi(this);

	InitUI();
	bind();
}

CSDlgGainSetting::~CSDlgGainSetting()
{
    delete ui;
}

void CSDlgGainSetting::on_comboBox_analogGain_IndexChanged(int index)
{
	if (m_device_ptr.isNull())
	{
		return;
	}
	if (m_current_index == index)
	{
		return;
	}
	//设置当前选项
	m_device_ptr->setProperty(Device::PropAnalogGain, ui->comboBox_analogGain->itemData(index));
	m_current_index = index;
}

void CSDlgGainSetting::on_pushButton_default_clicked()
{
	if (m_range_module_manage)
	{
		m_range_module_manage->setRangeLowValue(0);
		m_range_module_manage->setRangeHighValue(1023);
	}
	if (ui->widget_lut->IsEnabled())
	{
		ui->widget_lut->Default();
	}
}

void CSDlgGainSetting::on_pushButton_ok_clicked()
{
	close();
}

void CSDlgGainSetting::slotDisconnect2GainSet()
{
	QMessageBox::critical(this, QObject::tr("RCC"), tr("The camera is disconnected,please check this camera!"), QMessageBox::Yes);
	accept();
}

void CSDlgGainSetting::InitUI()
{
	// 去除问号按钮
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	if (m_device_ptr.isNull())
	{
		return;
	}

	//判断是否支持模拟增益
	if (m_device_ptr->IsAnalogGainSupported())
	{

		//获取设备支持的模拟增益值
		QVariantList support_analogGain;
		m_device_ptr->getSupportedProperties(Device::PropAnalogGain, support_analogGain);
		//将支持的选项放入下拉框
		for (auto analog_gain_item : support_analogGain)
		{
			ui->comboBox_analogGain->addItem(DeviceUtils::getAnalogGainText(analog_gain_item.toInt()), analog_gain_item.toInt());
		}
		//选中当前选项
		QVariant current_analog_gain;
		current_analog_gain = m_device_ptr->getProperty(Device::PropAnalogGain);
		m_current_index = ui->comboBox_analogGain->findData(current_analog_gain.toInt());
		if (m_current_index > -1)
		{
			ui->comboBox_analogGain->setCurrentIndex(m_current_index);
		}
		if (ui->comboBox_analogGain->count() <= 1)
		{
			ui->comboBox_analogGain->setEnabled(false);
		}
	}
	else
	{
		ui->comboBox_analogGain->setEnabled(false);
	}

	m_range_module_manage = new RangeModuleManage(ui->range_widget);
	QHBoxLayout* range_hl = new QHBoxLayout(ui->range_widget);
	range_hl->setContentsMargins(0, 0, 0, 0);
	range_hl->addWidget(m_range_module_manage);

	//数字增益
	ui->widget_lut->setDevice(m_device_ptr);
	ui->pushButton_default->setEnabled(ui->widget_lut->IsEnabled());

	int nBitsPerPixel = m_device_ptr->getProperty(Device::PropPixelBitDepth).toInt();
	m_range_module_manage->setLineEditRangeMax(std::pow(2, nBitsPerPixel) - 1);

	updateDisplayValue();
}

void CSDlgGainSetting::bind()
{
	//连接combox信号
	bool ok = connect(ui->comboBox_analogGain, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, &CSDlgGainSetting::on_comboBox_analogGain_IndexChanged);
	ok = connect(m_range_module_manage, &RangeModuleManage::signalRangeLowValue, this, [=](int lowValue) {
		ui->widget_lut->updatePtInfo(QPoint(lowValue, 0), QPoint(m_range_module_manage->getHighValue(),1023)); });
	ok = connect(m_range_module_manage, &RangeModuleManage::signalRangeHighValue, this, [=](int highValue) {
		ui->widget_lut->updatePtInfo(QPoint(m_range_module_manage->getLowerValue(), 0), QPoint(highValue, 1023)); });
	ok = connect(ui->widget_lut, &CSLutCurveEditWidget::valueChanged, this, [=]() {
		updateDisplayValue(); });
}

void CSDlgGainSetting::updateDisplayValue()
{
	if (m_device_ptr.isNull())
	{
		return;
	}
	QPoint lower_pt, upper_pt;
	ui->widget_lut->getPtInfo(lower_pt, upper_pt);

	if (m_range_module_manage)
	{
		m_range_module_manage->updateDisplayValue(lower_pt.x(), upper_pt.x());
	}
}
