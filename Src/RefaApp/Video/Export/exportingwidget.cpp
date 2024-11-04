#include "exportingwidget.h"
#include "ui_exportingwidget.h"

#include <QCloseEvent>
#include <QDir>

#include "Common/UIExplorer/uiexplorer.h"
#include "Common/UIUtils/uiutils.h"
#include "VideoExportController.h"
#include "ISPUtil.h"
#include "CAGISPStruct.h"
#include "exportutils.h"
#include "PlayerController.h"
#include "localvideoexportpreprocessdelegate.h"
#include "SystemStateDefine.h"

ExportingWidget::ExportingWidget(QWidget *parent) :
	QDialog(parent),
    ui(new Ui::ExportingWidget)
{
    ui->setupUi(this);

    initUi();
}

ExportingWidget::~ExportingWidget()
{
    delete ui;
}

void ExportingWidget::startExport(const QList<VideoItem> &video_items)
{
    if (video_items.isEmpty())
    {
        return;
    }
	bexport_finished_ = false;

    video_items_ = video_items;

    int min = 0;
    int max = 100 * video_items.size();
    ui->progressBar->setRange(min, max);

    if (!export_ctrl_ptr_)
    {
        export_ctrl_ptr_.reset(new VideoExportController());
        connect(export_ctrl_ptr_.get(), &VideoExportController::sigExportProgressChanged, this, &ExportingWidget::onExportProgressChanged);
        connect(export_ctrl_ptr_.get(), &VideoExportController::sigExportRes, this, &ExportingWidget::onExportFinished);
    }

    exportVideo(video_items_.first());

	ui->pushButtonClose->setEnabled(true);
    
	exec();
}

void ExportingWidget::stopExport()
{
    export_ctrl_ptr_->CancelExport();
}

void ExportingWidget::closeEvent(QCloseEvent *event)
{
	if (!bexport_finished_)
    {
        QString title = tr("Break Export?");
        QString content = tr("After clicking OK, the system will automatically stop the export.");
        QString msg = QString("%1\r\n%2").arg(title).arg(content);
        if (!UIUtils::showQuestionMsgBox(this, msg, 1))
        {
            event->ignore();
            return;
        }

        stopExport();
    }

    event->accept();
}

void ExportingWidget::onExportProgressChanged(float progress)
{
    float value = current_video_index_ * 100 + progress;

    // 更新进度
    ui->progressBar->setValue((int)value);

    // 更新百分比
    float percent = value * 100 / ui->progressBar->maximum();
    ui->labelProgress->setText(QString("%1%").arg(percent, 0, 'f', 2));
}

void ExportingWidget::onExportFinished(bool res, float progress)
{
    result_map_[current_video_index_] = res;

    current_video_index_++;
    if (current_video_index_ < video_items_.size())
    {
        exportVideo(video_items_.at(current_video_index_));
    }
    else
    {
        // 导出结果检查
        QStringList video_error;
        for (auto iter = result_map_.begin(); iter != result_map_.end(); iter++)
        {
            if (!iter.value())
            {
                video_error << QString(tr("*%1")).arg(video_items_.at(iter.key()).getName());
            }
        }

        if (!video_error.isEmpty())
        {
            QString msg = QString("%1\r\n%2").arg(tr("The following videos export error:")).arg(video_error.join("\r\n"));
            UIUtils::showInfoMsgBox(this, msg);
        }

		// 关闭界面
		bexport_finished_ = true;
		close();
    }
}

void ExportingWidget::initUi()
{
    // 设置标题
    setWindowTitle(UIExplorer::instance().getProductName());

    // 去除问号按钮
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->pushButtonClose->setEnabled(false);

    connect(ui->pushButtonClose, &QPushButton::clicked, this, &ExportingWidget::close);
}

void ExportingWidget::exportVideo(const VideoItem &video_item)
{
	PlayerController player_controller;

	quint64 res = player_controller.OpenVideo(video_item.getId().toString());
	if (RMAErrorCode::RMA_ERR_NONE != res)
	{
		//导出错误
		QString msg = QString("%1%2").arg(tr("Export Error :")).arg(QString("0x%1").arg(res, sizeof(res) * 2, 16, QChar('0')));
		UIUtils::showErrorMsgBox(this, msg);
		bexport_finished_ = true;
		close();
		return;
	}

	ISPParams isp_params = player_controller.GetISPHandle()->GetISPParams();

	// 亮度/对比度
	isp_params.SetIlluminate(video_item.getLuminance());
	isp_params.SetContrast(video_item.getContrast());

	// 裁剪区域
	QRect roi = video_item.getRoi();
	ISPStruct::Rect_ISP rect;
	rect.x = roi.x();
	rect.y = roi.y();
	rect.width = roi.width();
	rect.height = roi.height();
	isp_params.SetRoi(rect);


	std::shared_ptr<SingleVideoExportParam> single_video_exp_param = std::make_shared<SingleVideoExportParam>();
	single_video_exp_param->srcFilePath = QStringList(video_item.getId().toString());
	single_video_exp_param->isp_param = isp_params;
	std::shared_ptr<LocalVideoExportPreprocessDelegate> video_export_preprocess_delegate = std::make_shared<LocalVideoExportPreprocessDelegate>(single_video_exp_param, this);
	
	LocalVideoExportPreprocessDelegate::LocalExtraParams param;
	param.roi = { (int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height };
	param.benabled_watermark = video_item.getProperty(VideoItem::OsdVisible, true).toBool();
	param.video_name = video_item.getName();

	video_export_preprocess_delegate->setExtraParams(param);
	export_ctrl_ptr_->SetVideoExportPreprocessDelegate(video_export_preprocess_delegate);

	std::shared_ptr<VideoExportDestinationParam> video_export_dst_param = std::make_shared<VideoExportDestinationParam>();
	video_export_dst_param->beginFrameIndex = video_item.getBeginFrameIndex();
	video_export_dst_param->endFrameIndex = video_item.getEndFrameIndex();
	video_export_dst_param->colorPattern = player_controller.GetISPHandle()->GetImageColorPattern();
	video_export_dst_param->dstFileFmt = ExportUtils::toMediaFormat(video_item.getVideoFormat());
	video_export_dst_param->dstFilePath = ExportUtils::getExportVideoPath(video_item);
	if (VIDEO_FMT_BMP == video_export_dst_param->dstFileFmt
		|| VIDEO_FMT_JPG == video_export_dst_param->dstFileFmt
		|| VIDEO_FMT_PNG == video_export_dst_param->dstFileFmt
		|| VIDEO_FMT_TIF == video_export_dst_param->dstFileFmt
		)
	{
		if (!video_export_dst_param->dstFilePath.isEmpty())
		{
			QDir dir;
			if (!dir.exists(video_export_dst_param->dstFilePath))
			{
				dir.mkpath(video_export_dst_param->dstFilePath);
			}
		}
	}
	video_export_dst_param->frameStep = video_item.getFrameStep();
	video_export_dst_param->fps = player_controller.GetVideoInfo()->GetAcquisitionFps();
	video_export_dst_param->width = rect.width;
	video_export_dst_param->height = rect.height;
	export_ctrl_ptr_->SetVideoExportDestinationParam(video_export_dst_param);

    export_ctrl_ptr_->StartExport();

    // 更新提示
    QString tip = QString(tr("Exporting %1 (Total: %2)...")).arg(current_video_index_ + 1).arg(video_items_.size());
    ui->labelTip->setText(tip);
}
