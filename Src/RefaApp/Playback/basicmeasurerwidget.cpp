#include "basicmeasurerwidget.h"
#include "ui_basicmeasurerwidget.h"
#include "Measurer/basicmeasurer.h"
#include "Common/UIUtils/uiutils.h"
#include "System/SystemSettings/systemsettingsmanager.h"
#include <QFileDialog>
#include <QDebug>
#include <QDateTime>

void ElideText(const QFont& font, int maxSize, const QString& src, QString& dst);

BasicMeasurerWidget::BasicMeasurerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BasicMeasurerWidget)
{
    ui->setupUi(this);

	InitData();
	InitUI();

	connect(ui->pushButtonClear, &QToolButton::clicked, this, &BasicMeasurerWidget::sigClearCurrentMeasureModeFeatures);
}

BasicMeasurerWidget::~BasicMeasurerWidget()
{
    delete ui;
}

void BasicMeasurerWidget::UpdateCalibrationFilePath(const QString& filePath)
{
	calibrationFilePath = filePath;
	UpdateCalibrationFilePathLineEdit();
	UpdateCalibrationFileButton();
}

void BasicMeasurerWidget::SetVideoExistsFlag(bool exists)
{
	ui->toolButtonPointMeasure->setEnabled(exists);
	ui->toolButtonLineMeasure->setEnabled(exists);
	ui->pushButtonClear->setEnabled(exists);

	//ui->toolButtonLineMeasure->click();
}

void BasicMeasurerWidget::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);

	UpdateCalibrationFilePathLineEdit();
}

void BasicMeasurerWidget::slotTabWidgetCurrentIndexChanged(int index)
{
	if (0 == index)
	{
		if (ui->toolButtonLineMeasure->isEnabled() && ui->toolButtonLineMeasure->isChecked())
		{
			ui->toolButtonLineMeasure->click();
		}
		else if (ui->toolButtonPointMeasure->isEnabled() && ui->toolButtonPointMeasure->isChecked())
		{
			ui->toolButtonPointMeasure->click();
		}
	}
	else if (1 == index)
	{
		if (ui->toolButtonLineMeasure->isEnabled())
		{
			ui->toolButtonLineMeasure->click();
		}
	}
}

void BasicMeasurerWidget::on_toolButtonDelete_clicked(bool checked)
{
	Q_UNUSED(checked);

	QString msg = QString("%1\r\n%2")
		.arg(tr("Delete file?"))
		.arg(tr("After clicking OK, the system will delete the calibration file."));
	if (UIUtils::showQuestionMsgBox(this, msg, 1))
	{
		BasicMeasurer::RemoveParam();
		calibrationFilePath.clear();
		UpdateCalibrationFilePathLineEdit();
		UpdateCalibrationFileButton();
	}
}

void BasicMeasurerWidget::on_toolButtonImport_clicked(bool checked)
{
	Q_UNUSED(checked);

	QString default_dir_path;
	QString calibration_file_path = QFileDialog::getOpenFileName(this, tr("Import calibration file"), default_dir_path, tr("Calibration files (*.yml)"));
	if (calibration_file_path.isEmpty())
	{
		return;
	}

	//判断有无读权限
	if (!QFileInfo(calibration_file_path).isReadable())
	{
		QString caption = tr("Warning");
		uint64_t res = 2;
		QString value = QString("0x%1").arg(res, 8, 16, QChar('0'));
		QString warn_desc = tr("No access rights.");
		QString msg_text = QString("%1(%2): %3").arg(caption).arg(value).arg(warn_desc);
		UIUtils::showWarnMsgBox(this, msg_text);
		return;
	}

	do
	{
		//CalibrationParam calibrationParam;
		if (!BasicMeasurer::CalibrationParamExists())
		{
			break;
		}

		QString msg = QString("%1\r\n%2")
			.arg(tr("Warning"))
			.arg(tr("Calibration file exists, do you want to overwrite this file?"));
		if (UIUtils::showQuestionMsgBox(this, msg))
		{
			break;
		}
		else
		{
			return;
		}
	} while (false);

	auto res = BasicMeasurer::load(calibration_file_path);
	if (1 == res)//导入失败
	{
		QString caption = tr("Error");
		QString value = QString("0x%1").arg(res, 8, 16, QChar('0'));
		QString error_desc = tr("File corruption or system crash, unable to import the file.");
		QString msg_text = QString("%1(%2): %3").arg(caption).arg(value).arg(error_desc);
		UIUtils::showErrorMsgBox(this, msg_text);
		return;
	}
	else if (2 == res)//只能导入单目标定参数文件
	{
		QString caption = tr("Warning");
		QString value = QString("0x%1").arg(res, 8, 16, QChar('0'));
		QString warn_desc = tr("Only monocular calibration file can be imported."); 
		QString msg_text = QString("%1(%2): %3").arg(caption).arg(value).arg(warn_desc);
		UIUtils::showWarnMsgBox(this, msg_text);
		return;
	}

	calibrationFilePath = calibration_file_path;
	UpdateCalibrationFilePathLineEdit();
	UpdateCalibrationFileButton();
}

void BasicMeasurerWidget::on_toolButtonExport_clicked(bool checked)
{
	Q_UNUSED(checked);

	//默认保存文件命名: 标定-yyyyMMdd-hhmmss.yml
	QDateTime time = QDateTime::currentDateTime();
	QString name = QString("%1-%2.yml").arg(tr("Calibration")).arg(time.toString("yyyyMMdd-hhmmss"));
	QString default_dir_path = SystemSettingsManager::instance().getCalibrationDirectory()+ QString("/")+name;
	QString calibration_file_path = QFileDialog::getSaveFileName(this, tr("Export calibration file"), default_dir_path, tr("Calibration files (*.yml)"));
	if (calibration_file_path.isEmpty())
	{
		return;
	}

	QDir dir(calibration_file_path);
	if (!dir.cdUp())
	{
		return;
	}
	if (!QFileInfo(dir.absolutePath()).isWritable())
	{
		QString caption = tr("Warning");
		uint64_t res = 2;
		QString value = QString("0x%1").arg(res, 8, 16, QChar('0'));
		QString warn_desc = tr("No access rights.");
		QString msg_text = QString("%1(%2): %3").arg(caption).arg(value).arg(warn_desc);
		UIUtils::showWarnMsgBox(this, msg_text);
		return;
	}
	SystemSettingsManager::instance().setCalibrationDirectory(dir.path());
	auto res = BasicMeasurer::save(calibration_file_path);
	Q_UNUSED(res);
}

void BasicMeasurerWidget::on_toolButtonLineMeasure_clicked(bool checked)
{
	if (ui->toolButtonPointMeasure->isChecked())
		ui->toolButtonPointMeasure->setChecked(false);

	//qDebug() << "toolButtonLineMeasure clicked";

	emit sigMeausreLine(checked);
}

void BasicMeasurerWidget::on_toolButtonPointMeasure_clicked(bool checked)
{
	if (ui->toolButtonLineMeasure->isChecked())
		ui->toolButtonLineMeasure->setChecked(false);

	emit sigMeasurePoint(checked);
}

void BasicMeasurerWidget::InitData()
{
	defaultCalibrationFilePathLineEditText = tr("No calibration file!");

	//CalibrationParam calibrationParam;
	if (BasicMeasurer::CalibrationParamExists(/*calibrationParam*/))
	{
		calibrationFilePath = BasicMeasurer::GetCalibrationFilePath();
	}

	//calibrationFilePath = "1111111111111111vvvvvvvvvv";
}

void BasicMeasurerWidget::InitUI()
{
	UpdateCalibrationFilePathLineEdit();
	UpdateCalibrationFileButton();
}

void BasicMeasurerWidget::UpdateCalibrationFilePathLineEdit()
{
	QString lineEditText = calibrationFilePath.isEmpty() ? defaultCalibrationFilePathLineEditText : calibrationFilePath;
	QString text;
	ElideText(ui->lineEdit_CalibrationFilePath->font(), ui->lineEdit_CalibrationFilePath->width(), lineEditText, text);
	ui->lineEdit_CalibrationFilePath->setText(text);
}

void BasicMeasurerWidget::UpdateCalibrationFileButton()
{
	ui->toolButtonDelete->setDisabled(calibrationFilePath.isEmpty());
	ui->toolButtonExport->setDisabled(calibrationFilePath.isEmpty());
}

void ElideText(const QFont& font, int maxSize, const QString& src, QString& dst)
{
	QFontMetrics fontMetrics(font);
	dst = fontMetrics.elidedText(src, Qt::ElideRight, maxSize);
}
