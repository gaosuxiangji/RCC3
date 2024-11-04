#include "localctrlwidget.h"
#include "ui_localctrlwidget.h"

#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QDateTime>

#include "Device/device.h"
#include "Device/deviceutils.h"
#include "System/SystemSettings/systemsettingsmanager.h"
#include "Common/PathUtils/pathutils.h"
#include "PlayerController.h"
#include "Common/ErrorCodeUtils/errorcodeutils.h"
#include "Video/VideoItemManager/videoitemmanager.h"
#include "Video/Export/exportingwidget.h"
#include "Video/Export/exportutils.h"
#include "Common/UIUtils/uiutils.h"
#include "Measurer/basicmeasurer.h"
#include "CalibrationController.h"
#include "ControllerManager.h"
#include "ChessboardHomographyCalibrator.h"
#include "ProjectController.h"
#include "common/KeyBoardControl/keyboardcontrol.h"

#undef max

LocalCtrlWidget::LocalCtrlWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LocalCtrlWidget),
	calibrator(ControllerManager::GetController<CalibrationController>()->CreateCalibrator(PRJ_MODE_MONO, CALI_MODE_CHESSBOARD_HOMOGRAPHY))
{
    ui->setupUi(this);

	ControllerManager::GetController<ProjectController>()->SetProjectMode(PRJ_MODE_MONO);//设定为单目模式

    initUi();
}

LocalCtrlWidget::~LocalCtrlWidget()
{
    delete ui;
}

void LocalCtrlWidget::slotCalibrationVideoExists(bool exists)
{
	ui->pushButtonCalibrate->setEnabled(exists);
}

void LocalCtrlWidget::slotReceiveCurrentFrame(const RMAImage& image)
{
	calibrationImage = image;
}

void LocalCtrlWidget::toVideoSettingsRootWidget()
{
    ui->stackedWidgetVideoSettings->setCurrentWidget(ui->pageVideoSettingsRoot);
}

void LocalCtrlWidget::toOtherSettingsWidget()
{
	ui->stackedWidgetVideoSettings->setCurrentWidget(ui->pageOtherSettings);
	toOtherSettingsWidgetRootPage();
}

void LocalCtrlWidget::toOtherSettingsWidgetRootPage()
{
	ui->stackedWidgetOtherSettings->setCurrentWidget(ui->pageOtherSettingsRoot);
}

void LocalCtrlWidget::toLuminanceContrastWidget()
{
	ui->stackedWidgetOtherSettings->setCurrentWidget(ui->pageLuminanceContrast);
}

void LocalCtrlWidget::toVideoFormatWidget()
{
	ui->stackedWidgetOtherSettings->setCurrentWidget(ui->pageVideoFormat);
}

void LocalCtrlWidget::setOsdVisible(bool visible)
{
	VideoItem video_item = VideoItemManager::instance().getVideoItem(current_video_id_);
	if (video_item.isValid())
	{
		// 应用
		video_item.setOsdVisible(visible);
		VideoItemManager::instance().setVideoItem(current_video_id_, video_item);

        emit osdVisibleChanged(visible);

		//更新其他设置显示内容
		setOtherSettingsText();
	}
}

void LocalCtrlWidget::setVideoFormat(const QVariant & format)
{
	VideoItem video_item = VideoItemManager::instance().getVideoItem(current_video_id_);
	if (video_item.isValid())
	{
		// 应用
		video_item.setVideoFormat(format.toInt());
		VideoItemManager::instance().setVideoItem(current_video_id_, video_item);

		// 更新UI
		updateVideoFormatCombo(video_item.getId(), video_item.getVideoFormat());
	
		//更新其他设置显示内容
		setOtherSettingsText();
	}
}

void LocalCtrlWidget::initUi()
{
	// 初始显示视频设置
	ui->tabWidget->setCurrentIndex(0);
	toVideoSettingsRootWidget();

    // 初始化视频列表
	initVideoListUi();

	//其他设置
	ui->widgetOtherSettings->setTitle(tr("Other Settings"));
	connect(ui->widgetOtherSettings, &TouchComboWidget::clicked, this, &LocalCtrlWidget::toOtherSettingsWidget);
	connect(ui->pushButtonBackToVideoSettingRoot, &QPushButton::clicked, this, &LocalCtrlWidget::toVideoSettingsRootWidget);
	
	// 水印信息
	initOsdUi();

    // 亮度对比度
    initLuminanceAndContrastUi();

	// 保存格式
    initVideoFormatUi();

	//更新其他设置显示内容
	setOtherSettingsText();

    // 导出
    initExportUi();

	//初始化标定参数界面
	InitCalibrationUiConnect();

	//初始化测量界面消息绑定
	InitBasicMeasureUiConnect();
	ui->doubleSpinBoxSquareSize->setRange(0.001, std::numeric_limits<double>::max());
	ui->doubleSpinBoxSquareSize->setAttribute(Qt::WA_InputMethodEnabled, true);
	ui->doubleSpinBoxSquareSize->setInputMethodHints(inputMethodHints() | Qt::InputMethodHint::ImhDigitsOnly);
	ui->doubleSpinBoxSquareSize->installEventFilter(this);
	ui->doubleSpinBoxSquareSize->clearFocus();
	connect(ui->doubleSpinBoxSquareSize, &QDoubleSpinBox::editingFinished, this, [this] {
		ui->doubleSpinBoxSquareSize->clearFocus();
	});
}

void LocalCtrlWidget::initVideoListUi()
{
	// 当前视频项切换关联
	connect(ui->widgetVideoList, &VideoListWidget::currentItemChanged, this, &LocalCtrlWidget::onCurrentVideoItemChanged);

    // 选中视频改变关联
    connect(ui->widgetVideoList, &VideoListWidget::selectedItemsChanged, this, &LocalCtrlWidget::onSelectedVideoItemsChanged);

    // 删除视频响应关联
    connect(ui->widgetVideoList, &VideoListWidget::itemsRemoved, this, &LocalCtrlWidget::onVideoItemsRemoved);
}

void LocalCtrlWidget::updateVideoListUi()
{

}

void LocalCtrlWidget::initLuminanceAndContrastUi()
{
    ui->widgetLuminanceContrast->setTitle(QString("%1/%2").arg(tr("Luminance")).arg(tr("Contrast")));
    ui->sliderLuminance->setRange(0, 100);
    ui->sliderContrast->setRange(0, 100);

    updateLuminanceAndContrastUi(QVariant(), 50, 50);

    // 组合框、返回响应关联
    connect(ui->widgetLuminanceContrast, &TouchComboWidget::clicked, this, &LocalCtrlWidget::toLuminanceContrastWidget);
    connect(ui->pushButtonLuminanceContrastBack, &QPushButton::clicked, this, &LocalCtrlWidget::toOtherSettingsWidgetRootPage);

    // 滑动条响应关联
    connect(ui->sliderLuminance, &QSlider::valueChanged, this, &LocalCtrlWidget::onLuminanceChanged);
    connect(ui->sliderContrast, &QSlider::valueChanged, this, &LocalCtrlWidget::onContrastChanged);
}

void LocalCtrlWidget::updateLuminanceAndContrastUi(const QVariant & video_id, int luminance, int contrast)
{
    // 组合框
    updateLuminanceAndContrastCombo(video_id, luminance, contrast);


    // 滑动条
    ui->sliderLuminance->setValue(luminance);
    ui->sliderContrast->setValue(contrast);
    ui->labelContrast->setText(QString::number(contrast));

    // 标签
    updateLuminanceLabel(luminance);
    updateContrastLabel(contrast);
}

void LocalCtrlWidget::updateLuminanceAndContrastCombo(const QVariant &video_id, int luminance, int contrast)
{
    QString current_text = QString("%1: %2, %3: %4").arg(tr("Luminance")).arg(luminance).arg(tr("Contrast")).arg(contrast);
    ui->widgetLuminanceContrast->setCurrentText(current_text);
    ui->widgetLuminanceContrast->setEnabled(video_id.isValid());
}

void LocalCtrlWidget::updateLuminanceLabel(int luminance)
{
    ui->labelLuminance->setText(QString::number(luminance));
}

void LocalCtrlWidget::updateContrastLabel(int contrast)
{
    ui->labelContrast->setText(QString::number(contrast));
}

void LocalCtrlWidget::initOsdUi()
{
    ui->widgetOsd->setText(tr("OSD"));

    updateOsdUi(QVariant(), true);

    connect(ui->widgetOsd, &TouchCheckWidget::clicked, this, &LocalCtrlWidget::setOsdVisible);
}

void LocalCtrlWidget::updateOsdUi(const QVariant & video_id, bool osd_visible)
{
    ui->widgetOsd->setChecked(osd_visible);
    ui->widgetOsd->setEnabled(video_id.isValid());
}

void LocalCtrlWidget::initVideoFormatUi()
{
    ui->widgetVideoFormat->setTitle(tr("Video Format"));

    updateVideoFormatUi(QVariant(), VIDEO_AVI);

    connect(ui->widgetVideoFormat, &TouchComboWidget::clicked, this, &LocalCtrlWidget::toVideoFormatWidget);
    connect(ui->pageVideoFormat, &TouchOptionsWidget::backButtonClicked, this, &LocalCtrlWidget::toOtherSettingsWidgetRootPage);
	connect(ui->pageVideoFormat, &TouchOptionsWidget::currentOptionChanged, this, &LocalCtrlWidget::setVideoFormat);
}

void LocalCtrlWidget::updateVideoFormatUi(const QVariant & video_id, int video_format)
{
	//清除
	ui->pageVideoFormat->clearOptions();

	//添加支持选项 
	QVariantList format_var_list;
	if (PlayerController::GetVideoFileFormat(video_id.toString())== VIDEO_FMT_RHVD)
	{
		format_var_list << VIDEO_RHVD;
	}
	format_var_list << VIDEO_AVI << VIDEO_MP4 << VIDEO_BMP << VIDEO_JPG << VIDEO_TIF;
	for (auto format_var : format_var_list)
	{
		ui->pageVideoFormat->addOption(format_var, DeviceUtils::getVideoFormatText(format_var.toInt()));
	}	

	//选中按钮 更新显示
	if (!format_var_list.contains(video_format) && !format_var_list.isEmpty())
	{
		video_format = format_var_list.front().toInt();
		setVideoFormat(QVariant(video_format));
	}
	ui->pageVideoFormat->setCurrentOption(QVariant(video_format));
	updateVideoFormatCombo(video_id, video_format);
}

void LocalCtrlWidget::updateVideoFormatCombo(const QVariant & video_id, int video_format)
{
	ui->widgetVideoFormat->setCurrentText(DeviceUtils::getVideoFormatText(video_format));
	ui->widgetVideoFormat->setEnabled(video_id.isValid());
}

void LocalCtrlWidget::initExportUi()
{
    updateExportUi(QVariantList());
}

void LocalCtrlWidget::InitCalibrationUiConnect()
{
	connect(ui->tabWidget, &QTabWidget::currentChanged, ui->widgetCalibrationParam, &BasicMeasurerWidget::slotTabWidgetCurrentIndexChanged);
}

void LocalCtrlWidget::updateExportUi(const QVariantList &selected_video_id_list)
{
    ui->pushButtonExport->setEnabled(!selected_video_id_list.isEmpty());
}

void LocalCtrlWidget::InitBasicMeasureUiConnect()
{
	connect(ui->widgetCalibrationParam, &BasicMeasurerWidget::sigMeasurePoint, this, &LocalCtrlWidget::sigMeasurePoint);
	connect(ui->widgetCalibrationParam, &BasicMeasurerWidget::sigMeausreLine, this, &LocalCtrlWidget::sigMeasureLine);
	connect(ui->widgetCalibrationParam, &BasicMeasurerWidget::sigClearCurrentMeasureModeFeatures, this, &LocalCtrlWidget::sigClearMeasureFeatures);
}

bool LocalCtrlWidget::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == ui->doubleSpinBoxSquareSize)
	{
		if (event->type() == QEvent::FocusIn)
		{
			KeyBoardControl::OpenKeyBoard();
		}
		else if (event->type() == QEvent::FocusOut)
		{
			KeyBoardControl::CloseKeyBoard();
		}
	}
	return QWidget::eventFilter(watched, event);
}

void LocalCtrlWidget::on_pushButtonImportVideo_clicked()
{
    QString dir = SystemSettingsManager::instance().getWorkingDirectory();
    do
    {
        // 导入视频
        QString caption = tr("Import Video");
        QString filter = tr("Videos (*.rhvd *.avi *.mp4)");
        QString video_file_path = QFileDialog::getOpenFileName(this, caption, dir, filter);
        if (video_file_path.isEmpty())
        {
            break;
        }

        // 访问权限检查
        if (!PathUtils::isReadable(video_file_path))
        {
            dir = QFileInfo(video_file_path).absolutePath();
            continue;
        }

        // 是否已导入
        if (VideoItemManager::instance().containsVideoItem(video_file_path))
        {
            UIUtils::showInfoMsgBox(this, tr("Video File Already Exists."));
            break;
        }

        // 导入视频文件
        PlayerController player_controller;
        quint64 res = player_controller.OpenVideo(video_file_path);
        if (res != 0)
        {
            ErrorCodeUtils::handle(this, res);
            break;
        }

        QVariant video_id = video_file_path;
        QString video_name = QFileInfo(video_file_path).completeBaseName();

        // 纳入视频文件管理
        VideoItem video_item(video_id, VideoItem::Local);
		video_item.setName(video_name);
		auto video_info = player_controller.GetVideoInfo();
		video_item.setVideoFormat(ExportUtils::toVideoFormat(video_info->GetVideoFormat()));
		video_item.setRoi(QRect(0, 0, video_info->GetWidth(), video_info->GetHeight()));
        VideoItemManager::instance().addVideoItem(video_item);

        // 添加到视频列表
        VideoListItem video_list_item;
        video_list_item.id = video_id;
        video_list_item.name = video_name;
        ui->widgetVideoList->addItem(video_list_item);

        // 选中视频
        ui->widgetVideoList->setCurrentItem(video_id);

        break;
    } while(1);
}

void LocalCtrlWidget::on_pushButtonImportImageSequence_clicked()
{
    // 导入视频序列
	QString dir = SystemSettingsManager::instance().getWorkingDirectory();
	do 
	{
		QString caption = tr("Import Image Sequence");
		QFileDialog::Options options = QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks;
		QString image_sequence_path = QFileDialog::getExistingDirectory(this, caption, dir, options);
		if (image_sequence_path.isEmpty())
		{
			break;
		}

		// 访问权限检查
		if (!PathUtils::isReadable(image_sequence_path))
		{
			dir = QFileInfo(image_sequence_path).absolutePath();
			continue;
		}

		// 是否已导入
		if (VideoItemManager::instance().containsVideoItem(image_sequence_path))
		{
			UIUtils::showInfoMsgBox(this, tr("Image Sequence Already Exists."));
			break;
		}

		// 导入图片序列
		PlayerController player_controller;
		quint64 res = player_controller.OpenVideo(image_sequence_path);
		if (res != 0)
		{
			ErrorCodeUtils::handle(this, res);
			break;
		}

		QVariant video_id = image_sequence_path;
		QString video_name = QFileInfo(image_sequence_path).fileName();

		// 纳入视频文件管理
		VideoItem video_item(video_id, VideoItem::Local);
		video_item.setName(video_name);
		auto video_info = player_controller.GetVideoInfo();
		video_item.setVideoFormat(ExportUtils::toVideoFormat(video_info->GetVideoFormat()));
		video_item.setRoi(QRect(0, 0, video_info->GetWidth(), video_info->GetHeight()));
		VideoItemManager::instance().addVideoItem(video_item);

		// 添加到视频列表
		VideoListItem video_list_item;
		video_list_item.id = video_id;
		video_list_item.name = video_name;
		ui->widgetVideoList->addItem(video_list_item);

		// 选中视频
		ui->widgetVideoList->setCurrentItem(video_id);

		break;
	} while (1);
}

void LocalCtrlWidget::onCurrentVideoItemChanged(const QVariant &video_id)
{
    current_video_id_ = video_id;

    VideoItem video_item = VideoItemManager::instance().getVideoItem(video_id);

    // 更新亮度对比度界面
    updateLuminanceAndContrastUi(video_id, video_item.getLuminance(), video_item.getContrast());

    // 更新水印信息界面
    updateOsdUi(video_id, video_item.isOsdVisible());

    // 更新保存格式界面
    updateVideoFormatUi(video_id, video_item.getVideoFormat());

	//更新其他设置显示内容
	setOtherSettingsText();

	//更新标定按钮状态
	if (!ui->pushButtonCalibrate->isEnabled())
	{
		ui->pushButtonCalibrate->setEnabled(true);
		ui->widgetCalibrationParam->SetVideoExistsFlag(true);
	}

    emit currentVideoItemChanged(video_id);
	ui->widgetVideoList->onMouseClickedEnabled();
}

void LocalCtrlWidget::onSelectedVideoItemsChanged(const QVariantList &video_id_list)
{
    updateExportUi(video_id_list);
}

void LocalCtrlWidget::onVideoItemsRemoved(const QVariantList &video_id_list)
{
    for (auto video_id : video_id_list)
    {
        VideoItemManager::instance().removeVideoItem(video_id);
    }

    // 更新导出UI
    updateExportUi(ui->widgetVideoList->getSelectedItems());
}

void LocalCtrlWidget::onLuminanceChanged(int value)
{
    VideoItem video_item = VideoItemManager::instance().getVideoItem(current_video_id_);
    if (video_item.isValid())
    {
        video_item.setLuminance(value);
        VideoItemManager::instance().setVideoItem(current_video_id_, video_item);

        // 更新UI
        updateLuminanceAndContrastCombo(video_item.getId(), video_item.getLuminance(), video_item.getContrast());
        updateLuminanceLabel(video_item.getLuminance());

        emit luminanceAndContrastChanged(video_item.getLuminance(), video_item.getContrast());
    
		//更新其他设置显示内容
		setOtherSettingsText();
	}
}

void LocalCtrlWidget::onContrastChanged(int value)
{
    VideoItem video_item = VideoItemManager::instance().getVideoItem(current_video_id_);
    if (video_item.isValid())
    {
        video_item.setContrast(value);
        VideoItemManager::instance().setVideoItem(current_video_id_, video_item);

        // 更新UI
        updateLuminanceAndContrastCombo(video_item.getId(), video_item.getLuminance(), video_item.getContrast());
        updateContrastLabel(video_item.getContrast());

        emit luminanceAndContrastChanged(video_item.getLuminance(), video_item.getContrast());
    
		//更新其他设置显示内容
		setOtherSettingsText();
	}
}

void LocalCtrlWidget::on_pushButtonExport_clicked()
{
    QString export_path;
    QString dir = SystemSettingsManager::instance().getWorkingDirectory();
    do
    {
        QString caption = tr("Export Video");
        QFileDialog::Options options = QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks;
        QString path = QFileDialog::getExistingDirectory(this, caption, dir, options);
        if (path.isEmpty())
        {
            break;
        }

        // 访问权限检查
        if (!PathUtils::isReadable(path))
        {
            dir = QFileInfo(path).absolutePath();
            continue;
        }

        export_path = path;

        break;
    } while(1);

    if (export_path.isEmpty())
    {
        return;
    }

    QList<VideoItem> video_items;
    QVariantList video_id_list = ui->widgetVideoList->getSelectedItems();
    for (auto video_id : video_id_list)
    {
        VideoItem video_item = VideoItemManager::instance().getVideoItem(video_id);
        video_item.setExportPath(export_path);
        video_items << video_item;
    }
    ExportingWidget widget(this);
    widget.startExport(video_items);
}

void LocalCtrlWidget::on_pushButtonCalibrate_clicked(bool checked)
{
	Q_UNUSED(checked);

	emit sigBeginCalibrate();

	//获取标定图像
	//emit sigRequestCurrentFrame();

	if (calibrationImage.Empty())
		return;
	calibrator->GetLocalBehaviourHandle<ChessboardHomographyCalibrator::Behaviour>()->SetChessboardSquareSize(ui->doubleSpinBoxSquareSize->value());
	
	std::list<ImageFeaturePt> featureList;
	quint64 res = calibrator->AutoDetectFeatures(&calibrationImage, featureList);
	if (res != 0 || featureList.empty())
	{
		QString caption = tr("Warning");
		QString value = QString("0x%1").arg(2, 8, 16, QChar('0'));
		QString warn_desc = tr("Chessboard points was not detected on current frame, unable to calibrate!");
		QString msg_text = QString("%1(%2): %3").arg(caption).arg(value).arg(warn_desc);
		UIUtils::showWarnMsgBox(this, msg_text);
		return;
	}
	
	chessboardFeatureID=calibrator->AddFeatures(calibrationImage.GetVideoId(), calibrationImage.GetAbsoluteFrameIndex(), featureList.front());//显示棋盘格角点

	auto temp_param = BasicMeasurer::GetCalibrationParams();//获取当前系统中的标定参数
	
	bool bCalibrationParamExists = BasicMeasurer::CalibrationParamExists();
	if (!calibrator->Calibrate())
	{
		QString caption = tr("Error");
		QString value = QString("0x%1").arg(1, 8, 16, QChar('0'));
		QString error_desc = tr("System crash, fail to calibrate!");
		QString msg_text = QString("%1(%2): %3").arg(caption).arg(value).arg(error_desc);
		UIUtils::showErrorMsgBox(this, msg_text);
		return;
	}

	if (bCalibrationParamExists)//标定参数已存在
	{
		QString msg = QString("%1\r\n%2")
			.arg(tr("Warning"))
			.arg(tr("Calibration file exists, do you want to overwrite this file?"));
		if (!UIUtils::showQuestionMsgBox(this, msg))
		{
			bool res=BasicMeasurer::InsertCalibrationParams(temp_param);//恢复原先的标定参数
			Q_UNUSED(res);
			return;
		}
	}

	//获取当前时间
	QDateTime currentTime = QDateTime::currentDateTime();
	QString currentTime_str = currentTime.toString("yyyyMMddhhmmss");
	ui->widgetCalibrationParam->UpdateCalibrationFilePath(tr("Calibration") + currentTime_str);

	emit sigEndCalibrate();

	QString msg = tr("Calibration done!");
	UIUtils::showInfoMsgBox(this, msg);

	calibrator->RemoveFeatures(chessboardFeatureID);//清除棋盘格角点
}

void LocalCtrlWidget::setOtherSettingsText()
{
	VideoItem cur_item = VideoItemManager::instance().getVideoItem(current_video_id_);
	if (cur_item.isValid())
	{
		QString text = QString("%1, %2").arg(
			QString("%1:%2").arg(ui->widgetOsd->text(), cur_item.isOsdVisible() ? tr("Opened") : tr("Closed")),
			DeviceUtils::getVideoFormatText(cur_item.getProperty(VideoItem::PropType::VideoFormat).toInt())
		);
		ui->widgetOtherSettings->setCurrentText(text);
	}
}