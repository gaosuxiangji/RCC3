#include "csdevicetemperaturepanel.h"
#include "ui_csdevicetemperaturepanel.h"
#include "System/SystemSettings/systemsettingsmanager.h"
CSDeviceTemperaturePanel::CSDeviceTemperaturePanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CSDeviceTemperaturePanel)
{
    ui->setupUi(this);

	map_type2Widget_.insert("ARM",ui->lineEditArm);
	map_type2Widget_.insert("ARMCHIP",ui->lineEditArm);
	map_type2Widget_.insert("MAINBOARD",ui->lineEditMainBoard);
	map_type2Widget_.insert("SLAVEBOARD",ui->lineEditSlaveBoard);
	map_type2Widget_.insert("FPGA",ui->lineEditFPGA);
	map_type2Widget_.insert("CMOS",ui->lineEditCMOS);
	map_type2Widget_.insert("CHAMBER",ui->lineEditChamber);

	map_range_ = SystemSettingsManager::instance().getTemperatureThreshold();
}

CSDeviceTemperaturePanel::~CSDeviceTemperaturePanel()
{
    delete ui;
}

void CSDeviceTemperaturePanel::setTemperatureInfo(QString temperature_info)
{
	for (auto widgets : map_type2Widget_)
	{
		if (widgets)
		{
			widgets->setStyleSheet("");
			widgets->setText("--");
		}
	}

	if (!temperature_info.isEmpty())
	{
		QStringList listType2TemperatureStr = temperature_info.split(";");
		for(auto type2TempStr : listType2TemperatureStr)
		{
			QStringList tempSplits = type2TempStr.split(":");
			if (tempSplits.size() == 2)
			{
				QString type = tempSplits.at(0);
				type = type.toUpper();
				if (map_type2Widget_.find(type) != map_type2Widget_.end())
				{
					QLineEdit* line_edit = map_type2Widget_[type];
					if (line_edit)
					{
						//输出信息
						QString tempreature = tempSplits.at(1);
						bool ok = false;
						int temp = tempreature.toInt(&ok);
						if (ok)
						{
							if (map_range_.find(type) != map_range_.end() && map_range_[type].first <= temp  &&  temp <= map_range_[type].second)
							{
								line_edit->setStyleSheet("color:#FF9900");
							}
							else
							{
								line_edit->setStyleSheet("color:#FF0000");
							}
							line_edit->setText(QString::number(temp));
						}
						else
						{
							line_edit->setStyleSheet("");
						}
						
					}
				}
			}
		}
	}
	/*else
	{
		for (auto widgets : map_type2Widget_)
		{
			if (widgets)
			{
				widgets->setStyleSheet("");
				widgets->setText(init_str_);
			}
		}
	}*/
}

void CSDeviceTemperaturePanel::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		ui->retranslateUi(this);

	}

	QWidget::changeEvent(event);
}
