#include "csdlgdevicevideoexport.h"
#include "ui_csdlgdevicevideoexport.h"
#include "Device/devicemanager.h"
//#include "Device/device.h"

#include "Device/imageprocessor.h"
#include "Device/deviceutils.h"
#include "System/SystemSettings/systemsettingsmanager.h"
#include "System/FunctionCustomizer/FunctionCustomizer.h"
#include "Video/VideoUtils/videoutils.h"
#include "Video/Export/exportutils.h"
#include "Video/checkboxdelegate.h"
#include "Common/PathUtils/pathutils.h"
#include "Common/UIUtils/uiutils.h"
#include "Common/AgErrorCode/agerrorcode.h"
#include <QImageReader>
#include <QListView>
#include <QCloseEvent>
#include <QStorageInfo>
#include <QFileDialog>
#include <QDesktopServices>
#include <QDateTime>
#include <QBitmap>
#include "Common/LogUtils/logutils.h"
#include "Main/rccapp/render/PlayerViewBase.h"
#include <QMessageBox>
#include <QToolTip>
#include "ToolTipsShowWidget.h"
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <QRegExpValidator>
#include <QLineEdit>

CSImageGraphiceItem::CSImageGraphiceItem(const QPixmap &pixmap, QGraphicsItem *parent):
	QGraphicsPixmapItem(pixmap, parent)
{
}

void CSImageGraphiceItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if (event)
	{
		if (event->type() == QEvent::Type::GraphicsSceneMouseMove)
		{
			auto pos = event->pos();
			QRectF rc = this->parentItem()->boundingRect();
			auto ParentPos = this->mapToParent(pos);
			if (!rc.contains(ParentPos))
			{
				return;
			}
		}
	}
	QGraphicsPixmapItem::mouseMoveEvent(event);
}

CSDlgDeviceVideoExport::CSDlgDeviceVideoExport(const std::pair<int32_t, VideoItem> &video_item, bool auto_export, QWidget *parent) :
	QDialog(parent),
	m_enable_auto_export(auto_export),
//	m_video_item(video_item),
	ui(new Ui::CSDlgDeviceVideoExport)
{
    ui->setupUi(this);
	ui->tbv_videolist->setAttribute(Qt::WA_Hover, true);
	ui->tbv_videolist->setMouseTracking(true);
	m_video_items.push_back(video_item);

	//获取设备指针
	QString device_ip = VideoUtils::parseDeviceIp(m_video_items[0].second.getId());
	//int vid = VideoUtils::parseVideoSegmentId(m_video_items[0].getId());
	m_device_ptr = DeviceManager::instance().getDevice(device_ip);
	//Q_ASSERT(m_device_ptr);
	auto device_ptr = m_device_ptr.lock();
	m_export_cache_frame = std::make_shared<ExportCacheFrame>(m_device_ptr, m_video_items.size());;
	if (device_ptr) m_header_type_ = device_ptr->getFrameHeadType();
// 	//获取当前设备的协议格式和当前视频的协议格式
// 	if (device_ptr)
// 	{
// 		m_device_stream_type = device_ptr->getProperty(Device::PropStreamType).toInt();
// 	}
// 	m_video_stream_type = m_video_item.getStreamType();
//    QTimer::singleShot(10, this, &CSDlgDeviceVideoExport::InitUI);
    InitUI();
	SetWatermarkerCtrEnable(false);
}

CSDlgDeviceVideoExport::CSDlgDeviceVideoExport(const std::vector<std::pair<int32_t, VideoItem>> &video_items, bool auto_export, QWidget *parent) :
	QDialog(parent),
	m_enable_auto_export(auto_export),
	ui(new Ui::CSDlgDeviceVideoExport)
{
	m_video_items = video_items;
//	if (m_video_items.empty()) return;
//	if (!m_video_items.empty()) m_video_item = m_video_items[0];
	ui->setupUi(this);
	ui->tbv_videolist->setAttribute(Qt::WA_Hover, true);
	ui->tbv_videolist->setMouseTracking(true);
	
	//获取设备指针
	QString device_ip = VideoUtils::parseDeviceIp(m_video_items[0].second.getId());
//	int vid = VideoUtils::parseVideoSegmentId(m_video_items[0].getId());
	m_device_ptr = DeviceManager::instance().getDevice(device_ip);
	m_export_cache_frame =  std::make_shared<ExportCacheFrame>(m_device_ptr, m_video_items.size());
	auto device_ptr = m_device_ptr.lock();
	if (device_ptr) m_header_type_ = device_ptr->getFrameHeadType();
	//Q_ASSERT(m_device_ptr);
	//auto device_ptr = m_device_ptr.lock();
	// 	//获取当前设备的协议格式和当前视频的协议格式
	// 	if (device_ptr)
	// 	{
	// 		m_device_stream_type = device_ptr->getProperty(Device::PropStreamType).toInt();
	// 	}
	// 	m_video_stream_type = m_video_item.getStreamType();
//    QTimer::singleShot(10, this, &CSDlgDeviceVideoExport::InitUI);
    InitUI();
}


CSDlgDeviceVideoExport::~CSDlgDeviceVideoExport()
{
	stopExport();
	delete ui;
}

void CSDlgDeviceVideoExport::progressChanged(DeviceHandle handle, uint32_t value, uint32_t state)
{
	Q_UNUSED(handle);
	emit signalProgressChanged(value, state);
}

void CSDlgDeviceVideoExport::closeEvent(QCloseEvent *event)
{
	//判断是否需要停止导出
	if (b_exporting)
	{
		QString title = tr("Break Export?");
		QString content = tr("After clicking OK, the system will automatically stop the export.");
		QString msg = QString("%1\r\n%2").arg(title).arg(content);
		if (!UIUtils::showQuestionMsgBox(this, msg, 1))
		{
			event->ignore();
			return;
		}
		m_export_result = false;
		b_exporting = false;
		stopExport();
	}
	event->accept();
}

void CSDlgDeviceVideoExport::on_pushButton_default_clicked()
{
	SystemSettingsManager::instance().setVideoFormat(ui->comboBox_save_format->currentText());
	SystemSettingsManager::instance().setVideoCorlor(ui->comboBox_color_mode->currentText());
	//播放帧率
	int nCurrentIndex = ui->comboBox_display_rate->currentIndex();
	uint32_t uiFps = 25;
	if (nCurrentIndex < m_listPlaySpeed.count())
	{
		uiFps = m_listPlaySpeed[nCurrentIndex];
	}
	SystemSettingsManager::instance().setDisplayfps((int)uiFps);
	SystemSettingsManager::instance().setSkipFrame(ui->doubleSpinBox_jump->text().toDouble());
	SystemSettingsManager::instance().setSkipUnit(ui->comboBox_save_unit->currentIndex());
	SystemSettingsManager::instance().setVideoRuleName(ui->lineEdit_video_name->text());
	SystemSettingsManager::instance().setDefaultVideoWatermarkEnabled(ui->checkBox_watermark->isChecked());

#ifdef saveWatermarkMap
	if (!m_videoOpts[mCrtIdx].waterMarks.isEmpty())
	{
		QMap<QString, QStringList> watermarkMap;
		for (auto watermarkIter : m_videoOpts[mCrtIdx].waterMarks)
		{
			QString fileName = watermarkIter.strFileName;
			QStringList watermarkParam;
			double widthPath = watermarkIter.pItem->x() / m_videoOpts[mCrtIdx].nRoiWidth;
			double heightPath = watermarkIter.pItem->y() / m_videoOpts[mCrtIdx].nRoiHeight;
			watermarkParam.push_back(watermarkIter.strFileID);  //文件索引
			watermarkParam.push_back(QString::number(widthPath));  //位置X百分比
			watermarkParam.push_back(QString::number(heightPath));  //位置Y百分比
			watermarkParam.push_back(QString::number(watermarkIter.nWidth));
			watermarkParam.push_back(QString::number(watermarkIter.nHeight));
			watermarkParam.push_back(QString::number(watermarkIter.nTransparent));  //透明度
			watermarkParam.push_back(QString::number(watermarkIter.nRotate));  //旋转角
			watermarkParam.push_back(QString::number(watermarkIter.nWaterSize));  //大小百分比
			watermarkMap.insert(fileName, watermarkParam);
		}
		SystemSettingsManager::instance().setVideoWatermarkInfo(watermarkMap);
	}
#endif
	UIUtils::showInfoMsgBox(this, tr("Successfully saved the default configuration!"));
}

void CSDlgDeviceVideoExport::on_pushButton_start_clicked()
{
	QString strWorkFilePath = ui->lineEdit_save_path->text();
	QDir dirPath(strWorkFilePath);
	if (!dirPath.exists()) 
	{
		QString message = tr("The video export path does not exist, please reselect the path.");
		QMessageBox::information(this, QObject::tr("RCC"), message, QMessageBox::Yes, QMessageBox::Yes);
		return;
	}
	// [2023/2/20 rgq]: 添加导出中不能改编功能
	if (ui->tbv_videolist)
	{
		ui->tbv_videolist->setEditTriggers(QAbstractItemView::NoEditTriggers);
	}

	if (!m_enable_auto_export && ui->lineEdit_browse_video_name->text().isEmpty()) {
		QString message = tr("Video name is empty.");
		QMessageBox::information(this, QObject::tr("RCC"), message, QMessageBox::Yes, QMessageBox::Yes);
		return;
	}
	//修改水印信息
	UpdateVideoSaveInfo(mCrtIdx, m_videoOpts[mCrtIdx]);
	m_nExortIdx = 0;
	doExport(m_nExortIdx);
}

void CSDlgDeviceVideoExport::on_pushButton_cancel_clicked()
{
	//直接关闭对话框,其余操作在closeEvent中完成
	close();
}

void CSDlgDeviceVideoExport::on_pushButton_browse_clicked()
{
	//开启路径选择对话框,结果保存在编辑框中
	QString default_dir_path = ui->lineEdit_save_path->text();
	do
	{
		// 选择工作路径
		QString dir_path = QFileDialog::getExistingDirectory(this, tr("Select the file save directory"), default_dir_path, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
		if (dir_path.isEmpty())
		{
			break;
		}

		// 读写权限检查
		if (!PathUtils::isReadable(dir_path) || !PathUtils::isWritable(dir_path))
		{
			UIUtils::showInfoMsgBox(this, tr("No access right!"));
			default_dir_path = dir_path;
			continue;
		}

		// 磁盘空间检查
		uint64_t min_free_space = 100 * 1024 * 1024;
		QStorageInfo storage_info = QStorageInfo(dir_path);
		if (storage_info.bytesAvailable() < min_free_space)
		{
			UIUtils::showInfoMsgBox(this, tr("Out Of Disk Space!"));
			default_dir_path = dir_path;
			continue;
		}

		// 设置工作路径
		ui->lineEdit_save_path->setText(dir_path);
		ui->lineEdit_save_path->setToolTip(dir_path);

		for (auto &opt : m_videoOpts) {
			//item.setExportPath(dir_path);
			opt.export_path = dir_path;
		}
		SystemSettingsManager::instance().setWorkingDirectory(dir_path);
		//刷新磁盘空间
		double free_space_GB = double(storage_info.bytesAvailable()) / 1024 / 1024 / 1024;
		ui->label_remain_space->setText(tr("Disk Space Remaining:") + " " + QString::number(free_space_GB, 'f', 1) + QString("GB"));
		UpdateExportButtonStatus();
		break;
	} while (1);
}

void CSDlgDeviceVideoExport::on_pushButton_open_clicked()
{
	QDesktopServices::openUrl(QUrl("file:///" + ui->lineEdit_save_path->text()));
}

void CSDlgDeviceVideoExport::on_pushButton_addWatermark_clicked()
{
	//开启路径选择对话框,结果保存在编辑框中
	QString default_dir_path;
	do
	{
		// 选择工作路径
		QStringList dir_path = QFileDialog::getOpenFileNames(this, tr("Select the watermark image"), tr(""), tr("Select Image(*.bmp *.jpg *.jpeg *.png);;Bmp(*.bmp);;JPG(*.jpg *.jpeg);;PNG(*.png)"), nullptr, QFileDialog::DontResolveSymlinks);
		if (dir_path.isEmpty())
		{
			break;
		}

		if (m_watermark_table_model_ptr->rowCount() + dir_path.size() > constMaxImageSize)
		{
			QString strText;
			strText = tr("Up to %1 watermarks can be added!").arg(QString::number(constMaxImageSize));
			UIUtils::showInfoMsgBox(this, strText);
			break;
		}
		bool bAdd = false;
		CSImageGraphiceItem *pNewItem = nullptr;
		QStandardItem *newwaterInfo = nullptr;
		for (auto temp : dir_path)
		{
			QFileInfo fileInfo = QFileInfo(temp);
			if (!fileInfo.exists())
			{
				continue;
			}
			QString strSuffix = fileInfo.suffix();
			if (strSuffix.compare("url", Qt::CaseInsensitive) == 0)
			{
				continue;
			}

			QString strFileName = fileInfo.fileName();
			int nWatersize = m_watermark_list.size();
			for (int i = 0; i < nWatersize; )
			{
				if (m_watermark_list[i].strFileName == strFileName)
				{
					QString strAllSuffix = tr(".");
					strAllSuffix += strSuffix;
					QString strAddName = tr("-copy");
					strAddName += strAllSuffix;
					strFileName.replace(strAllSuffix, strAddName);
					i = 0;
					continue;
				}
				i++;
			}

			//更新水印列表
			QStandardItem* button = new QStandardItem(QIcon(tr(":/images/window_control_icon_close.png")), tr("close"));
			button->setToolTip(tr("close"));
			QStandardItem * name = new QStandardItem();
			name->setText(strFileName);
			name->setToolTip(temp);
			name->setData(strFileName);
			QStandardItem *item = new QStandardItem();
			QList<QStandardItem *> waterInfo;
			waterInfo << item << name << button;
			m_watermark_table_model_ptr->appendRow(waterInfo);

			//读图
			QImageReader image_reader;
			image_reader.setDecideFormatFromContent(true);
			image_reader.setFileName(temp);
			QImage img = image_reader.read();
			//设置图元参数
			QPixmap pImage = QPixmap::fromImage(img);
			CSImageGraphiceItem *pSubItem = new CSImageGraphiceItem(pImage);
			pSubItem->setParentItem(m_RectItem);
			pSubItem->setPos(m_RectItem->boundingRect().center());
			pSubItem->setOffset(-pImage.width() / 2, -pImage.height() / 2);
			pSubItem->setFlag(QGraphicsItem::ItemIsMovable);
			pSubItem->setFlag(QGraphicsItem::ItemClipsToShape);
			pSubItem->setFlag(QGraphicsItem::ItemIsFocusable);
			pSubItem->setFlag(QGraphicsItem::ItemIsSelectable);
			pSubItem->setFlag(QGraphicsItem::ItemIgnoresParentOpacity);
			pSubItem->setAcceptHoverEvents(true);

			TWaterMakerInfo info;
			info.strFileID = temp;
			info.strFileName = strFileName;
			info.nWidth = pImage.width() / 2;
			info.nHeight = pImage.height() / 2;
			info.pItem = pSubItem;
			m_watermark_list.append(info);
			//复选框勾选
			emit SignalChangeCheckBoxState(m_watermark_table_model_ptr, item->index(),info.bVisible);

			pNewItem = pSubItem;
			newwaterInfo = name;
			bAdd = true;
		}
		if (!bAdd)
		{
			break;
		}
		if (m_pSelectedWaterItem)
		{
			m_pSelectedWaterItem->setSelected(false);
		}
		m_pSelectedWaterItem = pNewItem;
		m_pSelectedWaterItem->setSelected(true);
		if (newwaterInfo)
		{
			ui->tableViewWatermark->selectRow(newwaterInfo->row());
		}

		InitWatermarkerCtr();
		ui->tableViewWatermark->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
		ui->tableViewWatermark->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
		ui->tableViewWatermark->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
		ui->tableViewWatermark->horizontalHeader()->resizeSection(0, 20);
		ui->tableViewWatermark->horizontalHeader()->resizeSection(2, 20);
		break;
	} while (1);

	if (m_pSelectedWaterItem)
	{
		SetWatermarkerCtrEnable(m_pSelectedWaterItem->isVisible());
	}
	else
	{
		SetWatermarkerCtrEnable(false);
	}
}

void CSDlgDeviceVideoExport::on_comboBox_save_format_activated(int index)
{
	VideoSaveOpt &opt = m_videoOpts[mCrtIdx];
	opt.videoformat = opt.SupportVideoFormats.at(index);
	if (!isSaveImage(opt.videoformat))
	{
		//ui->spinBox_display_rate->setEnabled(!b_exporting && true);
		ui->comboBox_display_rate->setEnabled(!b_exporting && true);
	}
	else
	{
		//ui->spinBox_display_rate->setEnabled(false);
		ui->comboBox_display_rate->setEnabled(false);
	}
	UpdateFileName(opt);

	UpdateEnableWatermarker(opt);
	updateAviCompressOption(opt);
	updateDepthHint(opt);
	UpdateShowFormatString(opt);
	UpdateWaterMarkOption(opt);

	bool bAddWater = false;
	if (opt.enableVideoInfo && GetWaterMarkOptionEnable(opt))
	{
		bAddWater = true;
	}
	(opt.videoformat == VIDEO_RHVD) ? SetWatermarkerCtrEnable(false) : SetWatermarkerCtrEnable(true && (m_watermark_table_model_ptr->rowCount() != 0) && (m_pSelectedWaterItem != nullptr));
	m_video_items[mCrtIdx].second.setOsdVisible(bAddWater);
// 	QImage image;
// 	if (getImage(mCrtIdx, image))
// 	{
// 		if (m_RectItem)
// 		{
// 			QPixmap pimage = QPixmap::fromImage(image);
// 			((QGraphicsPixmapItem*)m_RectItem)->setPixmap(pimage);
// 		}
// 	}
}

void CSDlgDeviceVideoExport::on_comboBox_color_mode_activated(int index)
{
	if (mCrtIdx >= m_videoOpts.size())
	{
		return;
	}
	m_videoOpts[mCrtIdx].videoCorlor = m_videoOpts[mCrtIdx].SupportVideoColors.at(index);
}

void CSDlgDeviceVideoExport::on_comboBox_add_file_name_currentIndexChanged(int index)
{
	if (0 == index)
	{
		return;
	}
	bool bOverLength = false;
	ErrorFileNameValid nRet = EFNV_Success;

	int endPos = ui->lineEdit_video_name->cursorPosition();
	if (m_nVideoNameEditPos > -1)
	{
		if (m_nVideoNameEditPos > endPos)
		{
			m_nVideoNameEditPos = endPos;
		}
		endPos = m_nVideoNameEditPos;
	}
	ui->lineEdit_video_name->setSelection(0, endPos);
	QString frontText = ui->lineEdit_video_name->selectedText();
	QString strText = ui->comboBox_add_file_name->currentData().toString();
	QString strFileName = ui->lineEdit_video_name->text();
	if (strText == conststrDefaultName)
	{
		if (isSaveImage(m_videoOpts[mCrtIdx].videoformat))
		{
			QString strImageName = conststrRecordtime;
			strImageName += "/";
			strImageName += conststrIP;
			strImageName += "-";
			strImageName += conststrTimestamp;
			strImageName += "-";
			strImageName += conststrFramenumber;
			strFileName = strImageName;
		}
		else
		{
			QString strVideoName = conststrIP;
			strVideoName += "-";
			strVideoName += conststrTimestamp;
			strFileName = strVideoName;
		}
		QString strPathText = ui->lineEdit_save_path->text();
		nRet = isPathFileValid(strFileName, strPathText, m_videoOpts[mCrtIdx]);
		if (nRet != EFNV_Success)
		{
			bOverLength = true;
		}
		else
		{
			if (isSaveImage(m_videoOpts[mCrtIdx].videoformat))
			{
				m_videoOpts[mCrtIdx].ImageRuleName = strFileName;
			}
			else
			{
				m_videoOpts[mCrtIdx].videoRuleName = strFileName;
			}
			UpdateFileName(m_videoOpts[mCrtIdx]);
		}
	}
	else if (!strText.isEmpty())
	{
		QString replaceText;
		if (!strFileName.size())
		{
			replaceText = strText;
		}
		else
		{
			if (!frontText.size())
			{
				replaceText = strText + tr("-");
			}
			else if (frontText.at(frontText.size() - 1) == '-')
			{
				replaceText = frontText + strText;
			}
			else
			{
				replaceText = frontText + tr("-") + strText;
			}
		}		
		ui->lineEdit_video_name->insert(replaceText);

		strFileName = ui->lineEdit_video_name->text();
		QString strText = ui->lineEdit_save_path->text();
		nRet = isPathFileValid(strFileName, strText, m_videoOpts[mCrtIdx]);
		if (nRet != EFNV_Success)
		{
			bOverLength = true;
		}
		else
		{
			if (isSaveImage(m_videoOpts[mCrtIdx].videoformat))
			{
				m_videoOpts[mCrtIdx].ImageRuleName = strFileName;
			}
			else
			{
				m_videoOpts[mCrtIdx].videoRuleName = strFileName;

			}
			UpdateFileName(m_videoOpts[mCrtIdx]);
		}
	}
	if (0 != ui->comboBox_add_file_name->currentIndex())
	{
		ui->comboBox_add_file_name->setCurrentIndex(0);
	}
	if (bOverLength)
	{
		UIUtils::showInfoMsgBox(this, getFileNameErrorString(nRet));
	}
}

void CSDlgDeviceVideoExport::on_lineEdit_video_name_textChanged(const QString &arg1)
{
	QString strText = ui->lineEdit_save_path->text();
	QString strFileName = ui->lineEdit_video_name->text();
	bool bImage = isSaveImage(m_videoOpts[mCrtIdx].videoformat);
	if (strFileName.isEmpty())
	{
		if (bImage)
		{
			QString strImageName = conststrRecordtime;
			strImageName += "/";
			strImageName += conststrIP;
			strImageName += "-";
			strImageName += conststrTimestamp;
			strImageName += "-";
			strImageName += conststrFramenumber;
			strFileName = strImageName;
		}
		else
		{
			QString strVideoName = conststrIP;
			strVideoName += "-";
			strVideoName += conststrTimestamp;
			strFileName = strVideoName;
		}
		QString strNewFileName = FormatStringReplace(strFileName, bImage);
		strText += "/";
		strText += strNewFileName;
		strText += ".";
		strText += ExportUtils::formatToStr(m_videoOpts[mCrtIdx].videoformat);
		if (ui->lineEdit_browse_video_name)
		{
			ui->lineEdit_browse_video_name->setText(strNewFileName);
			ui->lineEdit_browse_video_name->setToolTip(strText);
		}
		if (!m_bPathIsValid)
		{
			UpdateExportButtonStatus();
		}
		m_strVideName = "";
		return;
	}
	ErrorFileNameValid nRet = isPathFileValid(strFileName, strText, m_videoOpts[mCrtIdx]);
	if (EFNV_Success != nRet)
	{
		ui->lineEdit_video_name->setText(m_strVideName);
// 		if (bImage)
// 		{
// 			ui->lineEdit_video_name->setText(m_videoOpts[mCrtIdx].ImageRuleName);
// 		}
// 		else
// 		{
// 			ui->lineEdit_video_name->setText(m_videoOpts[mCrtIdx].videoRuleName);
// 		}
		int nHeight = ui->lineEdit_video_name->height();
		QPoint pos = ui->lineEdit_video_name->mapToGlobal(QPoint(0, nHeight));
		QString strText = getFileNameErrorString(nRet);
		int nWidth = ui->lineEdit_video_name->width();
		QRect rect(pos.x(), pos.y(), nWidth, 25);
		//QToolTip::showText(pos, strText, this, rect);
		//UIUtils::showInfoMsgBox(this, strText);

		ToolTipsShowWidget dlg(this);
		dlg.setRectAndText(rect, strText);
		dlg.exec();
	}
	else
	{
		m_strVideName = strFileName;
		QString strNewFileName = FormatStringReplace(strFileName, bImage);
		strText += "/";
		strText += strNewFileName;
		strText += ".";
		strText += ExportUtils::formatToStr(m_videoOpts[mCrtIdx].videoformat);
		if (ui->lineEdit_browse_video_name)
		{
			ui->lineEdit_browse_video_name->setText(strNewFileName);
			ui->lineEdit_browse_video_name->setToolTip(strText);
			//m_video_items[mCrtIdx].setRuleName(strFileName);
		}
		if (!m_bPathIsValid)
		{
			UpdateExportButtonStatus();
		}
	}
}

void CSDlgDeviceVideoExport::on_lineEdit_video_name_editingFinished()
{
	QString strFileName = ui->lineEdit_video_name->text();
	if (strFileName.isEmpty())
	{
		if (isSaveImage(m_videoOpts[mCrtIdx].videoformat))
		{
			QString strImageName = conststrRecordtime;
			strImageName += "/";
			strImageName += conststrIP;
			strImageName += "-";
			strImageName += conststrTimestamp;
			strImageName += "-";
			strImageName += conststrFramenumber;
			m_videoOpts[mCrtIdx].ImageRuleName = strImageName;
		}
		else
		{
			QString strVideoName = conststrIP;
			strVideoName += "-";
			strVideoName += conststrTimestamp;
			m_videoOpts[mCrtIdx].videoRuleName = strVideoName;
		}
	}
	else
	{
		if (isSaveImage(m_videoOpts[mCrtIdx].videoformat))
		{
			int nPos = strFileName.indexOf("/");
			if (nPos < 0)
			{
				QString strPathName = conststrRecordtime + "/" + strFileName;
				strFileName = strPathName;
			}
			m_videoOpts[mCrtIdx].ImageRuleName = strFileName;
		}
		else
		{
			m_videoOpts[mCrtIdx].videoRuleName = strFileName;
		}
		UpdateFileName(m_videoOpts[mCrtIdx]);
	}
}

void CSDlgDeviceVideoExport::on_spinBox_transparent_editingFinished()
{
	int uiValue = ui->spinBox_transparent->value();
	if (uiValue >= constPercentValueMin && uiValue <= constPercentValueMax)
	{
		if (ui->slider_transparent->value() != uiValue)
		{
			ui->slider_transparent->setValue(uiValue);
		}
	}
}

void CSDlgDeviceVideoExport::on_spinBox_rotate_editingFinished()
{
	int uiValue = ui->spinBox_rotate->value();
	if (uiValue >= constAngleValueMin && uiValue <= constAngleValueMax)
	{
		if (ui->slider_rotate->value() != uiValue)
		{
			ui->slider_rotate->setValue(uiValue);
		}
	}
}

void CSDlgDeviceVideoExport::on_spinBox_watermark_editingFinished()
{
	int uiValue = ui->spinBox_watermark->value();
	if (uiValue >= constZoomValueMin && uiValue <= constZoomValueMax)
	{
		if (ui->slider_watermark->value() != uiValue)
		{
			ui->slider_watermark->setValue(uiValue);
		}
	}
}

void CSDlgDeviceVideoExport::on_doubleSpinBox_jump_editingFinished()
{
	double dbValue = ui->doubleSpinBox_jump->value();
	int uiValue = 0;
	if (ui->comboBox_save_unit->currentIndex() == eFrame)
	{
		uiValue = dbValue;
	}
	else if (ui->comboBox_save_unit->currentIndex() == eMillonSec)
	{
		auto fps = m_videoOpts[mCrtIdx].fps;
		uiValue = round(dbValue * fps / 1000.0);
		dbValue = uiValue * 1000.0 / fps;
		ui->doubleSpinBox_jump->setValue(dbValue);
	}
	else if (ui->comboBox_save_unit->currentIndex() == eSecond)
	{
		auto fps = m_videoOpts[mCrtIdx].fps;
		uiValue = round(dbValue * fps);
		dbValue = uiValue * 1.0 / fps;
		ui->doubleSpinBox_jump->setValue(dbValue);
	}
	m_videoOpts[mCrtIdx].skipFrame = uiValue;
}

void CSDlgDeviceVideoExport::on_comboBox_save_unit_activated(int index)
{
	auto start = m_videoOpts[mCrtIdx].uiStart;
	auto end = m_videoOpts[mCrtIdx].uiEnd;

	int uiValue = m_videoOpts[mCrtIdx].skipFrame;
	double dbValue = uiValue;
	if (index == eFrame)
	{
		ui->doubleSpinBox_jump->setDecimals(0);
		ui->doubleSpinBox_jump->setRange(0, end-start+1);
	}
	else if (index == eMillonSec)
	{
		ui->doubleSpinBox_jump->setDecimals(1);
		auto fps = m_videoOpts[mCrtIdx].fps;
		double range = (double)(end - start + 1) / fps * 1000;
		ui->doubleSpinBox_jump->setRange(0, range);
		dbValue = uiValue * 1000.0 / fps;
	}
	else if (index == eSecond)
	{
		ui->doubleSpinBox_jump->setDecimals(1);
		auto fps = m_videoOpts[mCrtIdx].fps;
		double range = (double)(end - start + 1) / fps;
		ui->doubleSpinBox_jump->setRange(0, range);
		dbValue = uiValue * 1.0 / fps;
	}
	ui->doubleSpinBox_jump->setValue(dbValue);
}

void CSDlgDeviceVideoExport::on_checkBox_watermark_clicked(bool checked) 
{
	auto device_ptr = m_device_ptr.lock();
	if (device_ptr)
	{
		//保存叠加水印状态
		device_ptr->setProperty(Device::PropWatermarkEnable, checked);
		m_videoOpts[mCrtIdx].enableVideoInfo = checked;
		m_video_items[mCrtIdx].second.setOsdVisible(checked);
	}
	QImage image;
	if (getImage(mCrtIdx, image))
	{
		if (m_RectItem)
		{
			QPixmap pimage = QPixmap::fromImage(image);
			((QGraphicsPixmapItem*)m_RectItem)->setPixmap(pimage);
		}
	}
}

void CSDlgDeviceVideoExport::on_checkBox_enableAVIcompress_clicked(bool checked)
{
	//保存avi压缩状态
	//SystemSettingsManager::instance().setAviCompressEnabled(checked);
	m_videoOpts[mCrtIdx].enableAVIcompress = checked;

}

void CSDlgDeviceVideoExport::saveOptToItm(VideoSaveOpt opt, VideoItem &item)
{
	if (isSaveImage(opt.videoformat))
	{
		item.setRuleName(opt.ImageRuleName);
	}
	else
	{
		item.setRuleName(opt.videoRuleName);
	}
	item.setVideoFormat(opt.videoformat);
	item.setDisplayMode(opt.videoCorlor);
	item.setBeginFrameIndex(opt.uiStart);
	item.setEndFrameIndex(opt.uiEnd);
	item.setOsdVisible(opt.enableVideoInfo);
	item.setExportFrameRate(opt.displayfps);
	item.setAVIcompress(opt.enableAVIcompress);

	auto unit = opt.skipUnit;
	double nskipFrame = opt.skipFrame;
	item.setFrameStep(nskipFrame);
	item.setExportPath(opt.export_path);
}

bool CSDlgDeviceVideoExport::doExport(int index)
{
	if (index >= m_video_items.size()) return false;
	auto &item = m_video_items[index].second;
	int crt_idx = m_video_items[index].first;
	
	int op_index = index;
	if (item.getId() == m_videoOpts[index].ID)
	{
		saveOptToItm(m_videoOpts[index], item);
	}
	else
	{
		op_index = 0;
		for (auto opt : m_videoOpts)
		{
			if (item.getId() == opt.ID)
			{
				saveOptToItm(opt, item);
				break;
			}
			++op_index;
		}
	}

	//水印
	int8_t mark = getWaterMarkValue(index);
	item.setOsdVisible(mark);

	//avi压缩
	//SystemSettingsManager::instance().setAviCompressEnabled(ui->checkBox_enableAVIcompress->isChecked());

	//导出之前确保路径存在
	QDir dir;
	dir.mkpath(item.getExportPath());

	auto type = m_videoOpts[op_index].stream_type;
	int height = m_videoOpts[op_index].nRoiHeight;
	if ((type != TYPE_RAW8 && type != TYPE_RGB8888) && height < 128)
	{
		QString error_desc;
		AgErrorCodeInfo error_code_info{};
		if (AgErrorCode::instance().get(0x0201000D, error_code_info))
		{
			error_desc = error_code_info.desc;
		}
		UIUtils::showInfoMsgBox(this,error_desc);
		return false;
	}

	//开始导出
	startExport();
	return true;
}

void CSDlgDeviceVideoExport::slotProgressChanged(uint32_t value, uint32_t state)
{
	//导出进度刷新
	ui->progressBar->setValue(value);

	//界面显示导出状态
	int count = ui->progressBar->maximum() - ui->progressBar->minimum() + 1;
	//if (value < count) value += 1;
	QString progress_str = tr("Exporting %1/%2 frames.")
		.arg(QString::number(value))
		.arg(QString::number(count));
	ui->label_progress->setText(progress_str);

	//返回状态响应
	if (state == HSC_EOF)//当前视频已导出到结尾
	{
		boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
		if (m_nExortIdx < m_video_items.size() - 1) {
			m_nExortIdx++;
			//InitUI();
			//初始化进度条状态
			//int min = 0, max = 0;
			//max = m_video_items[mCrtIdx].getEndFrameIndex() - m_video_items[mCrtIdx].getBeginFrameIndex();

			//ui->progressBar->setRange(min, max);
			//ui->progressBar->setValue(min);
			doExport(m_nExortIdx);
		}

		//直接关闭界面
		else if (m_nExortIdx == m_video_items.size() - 1) {
			b_exporting = false;
			ui->pushButton_start->setEnabled(!b_exporting);
			m_export_result = true;
			close();
		}
	}
	else if ((state == HSC_OK) || (HSC_CANCEL == state))//导出正常,传回进度
	{
		return;
	}
	else//导出异常,传回错误码
	{
		QString caption = QObject::tr("Error Code");
		QString value = QString("0x%1").arg(state, 8, 16, QChar('0'));

		QString error_desc;
		int level = 0;
		AgErrorCodeInfo error_code_info{};
		if (AgErrorCode::instance().get(state, error_code_info))
		{
			error_desc = error_code_info.desc;
			level = error_code_info.level;
		}

		QString msg_text = QString("%1(%2):%4").arg(caption).arg(value).arg(error_desc);
		msg_text = msg_text.trimmed();


		QString msg = QString("%1%2").arg(tr("Export Error :")).arg(msg_text);
		UIUtils::showInfoMsgBox(this, msg);
		stopExport();
		close();
	}
}

QList<VideoSaveOpt> CSDlgDeviceVideoExport::loadVideoInfo(QList<QPair<int32_t, VideoItem>> &items)
{
	QList<VideoSaveOpt> videoOpts;
	bool bGetSupportedVideoFormats = false;
	QList<VideoFormat> video_format_list;
	QList<HscDisplayMode> color_mode_list;
	bool bIsCamera = true;
	HscIntRange width_range{};
	HscIntRange height_range{};
	for (auto item_pair :items)
	{
		int index = item_pair.first;
		auto item = item_pair.second;
		VideoSaveOpt opt;
		if (!bGetSupportedVideoFormats)
		{
			QString device_ip = VideoUtils::parseDeviceIp(item.getId());
			auto device_ptr = DeviceManager::instance().getDevice(device_ip);
			if (device_ptr)
			{
				device_ptr->GetSupportedVideoFormats((StreamType)item.getStreamType(), video_format_list);
				device_ptr->GetSupportedDisplayModes((StreamType)item.getStreamType(), color_mode_list);
				bIsCamera = device_ptr->IsCamera();

				device_ptr->getRoiWidthRange(0, width_range);
				device_ptr->getRoiHeightRange(0, height_range);
			}
			else
			{
				video_format_list.push_back(VIDEO_AVI);
				color_mode_list.push_back(HscDisplayMode::HSC_DISPLAY_MONO);
			}
			bGetSupportedVideoFormats = true;
		}

		opt.bIsCamera = bIsCamera;
		opt.nBpp = (int)item.getValidBitsPerPixel();
		opt.width_range = width_range;
		opt.height_range = height_range;
		opt.SupportVideoFormats = video_format_list;
		opt.SupportVideoColors = color_mode_list;
		opt.stream_type = (StreamType)item.getStreamType();
		opt.displayfps = 25;
		opt.fps = item.getFPS();
		opt.videoformat = (VideoFormat)item.getVideoFormat();
		opt.videoCorlor = (HscDisplayMode)item.getDisplayMode();
		opt.skipFrame = item.getFrameStep();
		opt.enableVideoInfo = item.isOsdVisible();
		opt.ID = item.getId();
		QString strVideoName = conststrIP;
		strVideoName += "-";
		strVideoName += conststrTimestamp;
		opt.videoRuleName = strVideoName;
		QString strImageName = conststrRecordtime;
		strImageName += "/";
		strImageName += conststrIP;
		strImageName += "-";
		strImageName += conststrTimestamp;
		strImageName += "-";
		strImageName += conststrFramenumber;
		opt.ImageRuleName = strImageName;
		opt.export_path = item.getExportPath();
		opt.video_index = index;
		opt.uiStart = item.getBeginFrameIndex();
		opt.uiEnd = item.getEndFrameIndex();
		opt.nRoiHeight = item.getRoi().height();
		opt.nRoiWidth = item.getRoi().width();
		videoOpts.append(opt);
	}
	return videoOpts;
}

void CSDlgDeviceVideoExport::ReadSettings()
{
	QString defaultVideoFormat;
	bool bVideoFormat = SystemSettingsManager::instance().getVideoFormat(defaultVideoFormat);
	QString defaultVideoCorlor;
	bool bVideoCorlor = SystemSettingsManager::instance().getVideoCorlor(defaultVideoCorlor);
	QString videoRuleName;
	bool bVideoRuleName = SystemSettingsManager::instance().getVideoRuleName(videoRuleName);
	bool bDefaultWatermark = SystemSettingsManager::instance().isDefaultVideoWatermarkEnabled();
	int nSkipUint = 0;
	bool bDefaultSkip = SystemSettingsManager::instance().getSkipUnit(nSkipUint);
	//设置视频导出信息
	for (auto &opt : m_videoOpts)
	{
		//保存格式
		if (bVideoFormat)
		{
			for (auto videoformat : opt.SupportVideoFormats)
			{
				QString svideoformat = DeviceUtils::getVideoFormatText(videoformat);
				if (svideoformat == defaultVideoFormat)
				{
					opt.videoformat = videoformat;
				}
			}
		}
		if (bVideoCorlor)
		{
			//色彩模式	
			for (auto colorformat : opt.SupportVideoColors)
			{
				QString sColorformat = DeviceUtils::getColorModeText(colorformat);
				if (sColorformat == defaultVideoCorlor)
				{
					opt.videoCorlor = colorformat;
				}
			}
		}
		//播放帧率
		opt.displayfps = SystemSettingsManager::instance().getDisplayfps();
		if (bDefaultSkip)
		{
			//抽帧
			double dbSkipFrame = SystemSettingsManager::instance().getSkipFrame();
			if (nSkipUint == eFrame)
			{
				int nFrame = dbSkipFrame;
				if (nFrame > opt.uiEnd - opt.uiStart + 1)
				{
					nFrame = opt.uiEnd - opt.uiStart + 1;
				}
				if (nFrame < 0)
				{
					nFrame = 0;
				}
				opt.skipFrame = nFrame;
				opt.skipUnit = nSkipUint;
			}
			else if (nSkipUint == eMillonSec)
			{
				int nFrame = round(dbSkipFrame * opt.fps / 1000.0);
				if (nFrame > opt.uiEnd - opt.uiStart + 1)
				{
					nFrame = opt.uiEnd - opt.uiStart + 1;
				}
				if (nFrame < 0)
				{
					nFrame = 0;
				}
				
				opt.skipFrame = nFrame;
				opt.skipUnit = nSkipUint;
			}
			else if (nSkipUint == eSecond)
			{
				int nFrame = round(dbSkipFrame * opt.fps);
				if (nFrame > opt.uiEnd - opt.uiStart + 1)
				{
					nFrame = opt.uiEnd - opt.uiStart + 1;
				}
				if (nFrame < 0)
				{
					nFrame = 0;
				}
				opt.skipFrame = nFrame;
				opt.skipUnit = nSkipUint;
			}
		}
		//命名规则		
		if (bVideoRuleName)
		{
			VideoFormat videoformat = VIDEO_RAW;
			bool bFind = true;
			if (defaultVideoFormat.compare(DeviceUtils::getVideoFormatText(VIDEO_RHVD)) == 0)
			{
				videoformat = VIDEO_RHVD;
			}
			else if (defaultVideoFormat.compare(DeviceUtils::getVideoFormatText(VIDEO_AVI)) == 0)
			{
				videoformat = VIDEO_AVI;
			}
			else if (defaultVideoFormat.compare(DeviceUtils::getVideoFormatText(VIDEO_BMP)) == 0)
			{
				videoformat = VIDEO_BMP;
			}
			else if (defaultVideoFormat.compare(DeviceUtils::getVideoFormatText(VIDEO_JPG)) == 0)
			{
				videoformat = VIDEO_JPG;
			}
			else if (defaultVideoFormat.compare(DeviceUtils::getVideoFormatText(VIDEO_PNG)) == 0)
			{
				videoformat = VIDEO_PNG;
			}
			else if (defaultVideoFormat.compare(DeviceUtils::getVideoFormatText(VIDEO_MP4)) == 0)
			{
				videoformat = VIDEO_MP4;
			}
			else if (defaultVideoFormat.compare(DeviceUtils::getVideoFormatText(VIDEO_TIF)) == 0)
			{
				videoformat = VIDEO_TIF;
			}
			else if (defaultVideoFormat.compare(DeviceUtils::getVideoFormatText(VIDEO_RAW)) == 0)
			{
				videoformat = VIDEO_RAW;
			}
			else 
			{
				bFind = false;
			}
			if (bFind)
			{
				if (isSaveImage(videoformat))
				{
					opt.ImageRuleName = videoRuleName;
				}
				else
				{
					opt.videoRuleName = videoRuleName;
				}
			}
		}
		//叠加默认水印
		if (GetWaterMarkOptionEnable(opt))
		{
			opt.enableVideoInfo = bDefaultWatermark;
		}
		else
		{
			opt.enableVideoInfo = false;
		}

#ifdef saveWatermarkMap
		//叠加水印图片
		QMap<QString, QVariant> defaultWatermarkMap;
		defaultWatermarkMap = SystemSettingsManager::instance().getVideoWatermarkInfo();
		if (!defaultWatermarkMap.isEmpty())
		{
			for (auto iter = defaultWatermarkMap.begin(); iter != defaultWatermarkMap.end(); iter++)
			{
				QStringList watermarkParam = iter->toStringList();
				if (watermarkParam.size() != 8) continue;
				QFile file(watermarkParam.at(0));
				if (!file.exists()) continue;
				TWaterMakerInfo info;
				info.strFileName = iter.key();
				info.strFileID = watermarkParam.at(0);
				info.X = watermarkParam.at(1).toDouble()*(opt.nRoiWidth);
				info.Y = watermarkParam.at(2).toDouble()*(opt.nRoiHeight);
				info.nWidth = watermarkParam.at(3).toInt();
				info.nHeight = watermarkParam.at(4).toInt();
				info.nTransparent = watermarkParam.at(5).toInt();
				info.nRotate = watermarkParam.at(6).toInt();
				info.nWaterSize = watermarkParam.at(7).toInt();
				opt.waterMarks.append(info);
			}
		}
#endif
	}
	int i = 0;
	for (auto &videoiter : m_video_items)
	{
		bool bWaterMark = m_videoOpts[i].enableVideoInfo;
		videoiter.second.setOsdVisible(bWaterMark);
		++i;
	}

	auto currentOpt = m_videoOpts[mCrtIdx];
	//avi压缩勾选
	updateAviCompressOption(currentOpt);
	//更新位深提示
	updateDepthHint(currentOpt);
	//默认水印信息勾选
	UpdateWaterMarkOption(currentOpt);
	//更新formatCombobox下拉框
	UpdateShowFormatString(currentOpt);
}

void CSDlgDeviceVideoExport::UpdateVideoUI(VideoSaveOpt opt)
{
    ui->comboBox_save_format->clear();
	int nSelCur = -1;
    for (int i = 0; i < opt.SupportVideoFormats.size(); i++)
    {
        ui->comboBox_save_format->addItem(
            DeviceUtils::getVideoFormatText(opt.SupportVideoFormats.at(i)),
			opt.SupportVideoFormats.at(i));
		if (nSelCur == -1)
		{
			QString strFormatText = DeviceUtils::getVideoFormatText(opt.SupportVideoFormats.at(i));
			if (strFormatText.compare(DeviceUtils::getVideoFormatText(opt.videoformat)) == 0)
			{
				nSelCur = i;
			}
		}
    }

	ui->comboBox_color_mode->clear();
    for (int i = 0; i < opt.SupportVideoColors.size(); i++)
    {
        ui->comboBox_color_mode->addItem(
            DeviceUtils::getColorModeText(opt.SupportVideoColors.at(i)),
            opt.SupportVideoColors.at(i));
    }
	if (nSelCur == -1)
	{
		if (opt.SupportVideoFormats.size() > 0)
		{
			opt.videoformat = opt.SupportVideoFormats[0];
			m_videoOpts[mCrtIdx].videoformat = opt.videoformat;
		}
	}
    ui->comboBox_save_format->setCurrentText( DeviceUtils::getVideoFormatText(opt.videoformat));
	ui->comboBox_color_mode->setCurrentText(DeviceUtils::getColorModeText(opt.videoCorlor));
	//播放帧率
	int nCurrentIndex = m_listPlaySpeed.indexOf(uint32_t(opt.displayfps));
	if (nCurrentIndex >= 0 && nCurrentIndex < m_listPlaySpeed.count())
	{
	}
	else { 
		nCurrentIndex = m_listPlaySpeed.indexOf(25);
	}
	ui->comboBox_display_rate->setCurrentIndex(nCurrentIndex);
    //ui->spinBox_display_rate->setValue((int)uiFps);
	if (isSaveImage(opt.videoformat))
	{
		m_strVideName = opt.ImageRuleName;
		ui->lineEdit_video_name->setText(opt.ImageRuleName);
		ui->lineEdit_browse_video_name->setText(FormatStringReplace(opt.ImageRuleName, true));
	}
	else
	{
		m_strVideName = opt.videoRuleName;
		ui->lineEdit_video_name->setText(opt.videoRuleName);
		ui->lineEdit_browse_video_name->setText(FormatStringReplace(opt.videoRuleName, false));
	}

	ui->comboBox_save_unit->setCurrentIndex(opt.skipUnit);
	ui->doubleSpinBox_jump->setRange(0, getRange(opt));
	if (opt.skipUnit == eFrame)
	{
		ui->doubleSpinBox_jump->setDecimals(0);
		ui->doubleSpinBox_jump->setValue((int32_t)opt.skipFrame);
	}
	else if (opt.skipUnit == eMillonSec)
	{
		ui->doubleSpinBox_jump->setDecimals(1);
		ui->doubleSpinBox_jump->setValue(opt.skipFrame * 1000.0 / opt.fps);
	}
	else if (opt.skipUnit == eSecond)
	{
		ui->doubleSpinBox_jump->setDecimals(1);
		ui->doubleSpinBox_jump->setValue(opt.skipFrame * 1.0 / opt.fps);
	}

	//ui->lineEdit_jump_frame->setText(QString::number(opt.skipFrame));
	if (opt.videoformat == VIDEO_MP4 || opt.videoformat == VIDEO_AVI || opt.videoformat == VIDEO_RHVD || opt.videoformat == VIDEO_RAW)
	{
		//ui->spinBox_display_rate->setEnabled(!b_exporting && true);
		ui->comboBox_display_rate->setEnabled(!b_exporting && true);
	}
	else
	{
		//ui->spinBox_display_rate->setEnabled(false);
		ui->comboBox_display_rate->setEnabled(false);
	}

	UpdateShowFormatString(opt);
	updateAviCompressOption(opt);
	updateDepthHint(opt);
	UpdateWaterMarkOption(opt);
}

void CSDlgDeviceVideoExport::UpdateVideoSaveInfo(int idx, VideoSaveOpt &opt)
{
	if (b_exporting) {
		return;
	}

	// 保存格式
	int nIndex = ui->comboBox_save_format->currentIndex();
	bool bFind = false;
	if (nIndex >= 0)
	{
		if (nIndex < opt.SupportVideoFormats.size())
		{
			opt.videoformat = opt.SupportVideoFormats.at(nIndex);
			bFind = true;
		}
	}
	if (!bFind)
	{
		opt.videoformat = opt.SupportVideoFormats.at(0);
	}

	// 色彩模式
	bFind = false;
	nIndex = ui->comboBox_color_mode->currentIndex();
	if (nIndex >= 0)
	{
		if (nIndex < opt.SupportVideoColors.size())
		{
			opt.videoCorlor = opt.SupportVideoColors.at(nIndex);
			bFind = true;
		}
	}
	if (!bFind)
	{
		opt.videoCorlor = opt.SupportVideoColors.at(0);
	}

	// 视频名称
	QString strViewName = ui->lineEdit_video_name->text();
	if (!strViewName.isEmpty())
	{
		if (isSaveImage(opt.videoformat))
		{
			opt.ImageRuleName = strViewName;
		}
		else
		{
			opt.videoRuleName = strViewName;
		}
	}

	//播放帧率
	int nCurrentIndex = ui->comboBox_display_rate->currentIndex();
	uint32_t uiFps = 25;
	if (nCurrentIndex < m_listPlaySpeed.count())
	{
		uiFps = m_listPlaySpeed[nCurrentIndex];
	}
	opt.displayfps = (int)uiFps;

	//抽帧信息
	//int nskipFrame = ui->lineEdit_jump_frame->text().toInt();
	auto unit = ui->comboBox_save_unit->currentIndex();
	double nskipFrame = ui->doubleSpinBox_jump->value();
	int uiValue = nskipFrame;
	if (unit == eMillonSec)
	{
		auto fps = m_videoOpts[mCrtIdx].fps;
		uiValue = round(nskipFrame * fps / 1000.0);
	}
	else if (unit == eSecond)
	{
		auto fps = m_videoOpts[mCrtIdx].fps;
		uiValue = round(nskipFrame * fps);
	}
	opt.skipFrame = uiValue;
	opt.skipUnit = unit;

	//视频水印使能
	bFind = false;
	if (ui->checkBox_watermark)
	{
		if (ui->checkBox_watermark->isEnabled() && ui->checkBox_watermark->isChecked())
		{
			bFind = true;
		}
	}
	opt.enableVideoInfo = bFind;

	// 导出范围
	QList<VideoInfoRecordList> record = m_pModeVideoList->GetSelectInfoList();
	if (record.size() > mCrtIdx)
	{
		for (auto info : record)
		{
			if (info.nIndex == mCrtIdx)
			{
				uint64_t uiStart = info.videoInfo.uiStartFrame;
				uint64_t uiEnd = info.videoInfo.uiEndFrame;
				if (uiStart > 0)
				{
					uiStart--;
				}
				if (uiEnd > 0)
				{
					uiEnd--;
				}
				opt.uiStart = uiStart;
				opt.uiEnd = uiEnd;
				break;
			}
		}
	}

	// 水印信息
	bool bEnableItem = false;
	for (auto &temp : m_watermark_list)
	{
		QGraphicsItem *item = temp.pItem;
		if (item)
		{
			temp.nTransparent = 100 - item->opacity() * 100;
			temp.nRotate = item->rotation();
			temp.nWaterSize = item->scale() * 100;
			temp.X = item->pos().x();
			temp.Y = item->pos().y();
			item->setSelected(false);
			temp.bVisible = temp.pItem->isVisible();
			if (temp.bVisible)
			{
				bEnableItem = true;
			}
		}
	}
	bool bAddWatermark = false;
	if (m_watermark_list.size() > 0)
	{
		if (bEnableItem && m_RectItem)
		{
			//存在已经启用的自定义水印,将用户编辑的水印转换成一张带有alpha通道的图像

			//留存原图 
			QPixmap oldImage = ((QGraphicsPixmapItem*)m_RectItem)->pixmap();

			//创建带有透明度的背景
			QPixmap pimage(oldImage.width(), oldImage.height());
			pimage.fill(QColor(0,0,0,255));//透明背景
			((QGraphicsPixmapItem*)m_RectItem)->setPixmap(pimage);	
			m_RectItem->setOpacity(0);//隐藏背景(所有子项需要设置ItemIgnoresParentOpacity,才不会被影响)

			//准备带有alpha通道的空白水印图
			QRectF size = m_RectItem->boundingRect();
			size.setTopLeft(m_RectItem->scenePos());
			size.setWidth(m_RectItem->boundingRect().width());
			size.setHeight(m_RectItem->boundingRect().height());
			QPixmap pix(size.width(), size.height());
			pix.fill(QColor(0, 0, 0, 0));
			QPainter painter(&pix);
			scene->setBackgroundBrush(QBrush(QColor(0, 0, 0, 0)));//透明背景
			scene->render(&painter, m_RectItem->boundingRect(), size);
			scene->setBackgroundBrush(QBrush(Qt::gray));//还原成灰色背景

			((QGraphicsPixmapItem*)m_RectItem)->setPixmap(oldImage);//还原原图
			m_RectItem->setOpacity(1);//显示背景图

			// [2022/11/5 rgq]: 获取全部自定义水印
			QImage qImage = pix.toImage();
			int vid = VideoUtils::parseVideoSegmentId(opt.ID);
			if (m_export_cache_frame) m_export_cache_frame->updateWaterMark(vid, qImage);
			bAddWatermark = true;

			//调试,显示最终水印图
// 			cv::Mat test_mat;
// 			CPlayerViewBase::QImage2CvMat(qImage, test_mat);
// 			cv::imshow("", test_mat);
// 			cv::waitKey(100);
		}
	}
	if (!bAddWatermark)
	{
		int vid = VideoUtils::parseVideoSegmentId(opt.ID);
		// [2022/11/6 rgq]: 添加清空水印的操作
		if (m_export_cache_frame) m_export_cache_frame->deleteWaterMark(vid);
	}
}

bool CSDlgDeviceVideoExport::getImage(int index, QImage &image)
{
	if (!m_export_cache_frame) return false;
	int vid = VideoUtils::parseVideoSegmentId(m_video_items[index].second.getId());
	cv::Mat mat;
	bool rslt = m_export_cache_frame->getFrame(vid, m_video_items[index].second, mat);
	if (!rslt) return false;

	CPlayerViewBase::cvMat2QImage(mat, image);
	return true;
}

bool CSDlgDeviceVideoExport::GetWaterMarkOptionEnable(VideoSaveOpt opt)
{
	if (!opt.bIsCamera)
	{
		return false;
	}
	VideoFormat video_format = opt.videoformat;
	if (video_format == VIDEO_RHVD)
	{
		return false;
	}

	StreamType stream_type = opt.stream_type;
	if (video_format == VIDEO_TIF && opt.nBpp == 16)//16bit tif水印暂时屏蔽
	{
		return false;
	}
	if (stream_type == TYPE_RAW8 || stream_type == TYPE_RGB888 || stream_type == TYPE_YUV420)
	{
		return true;
	}
	return false;
}

bool CSDlgDeviceVideoExport::GetEnableExport()
{
	QRect rect = m_video_items[mCrtIdx].second.getRoi();
	if (m_height_range.min > rect.height() || m_width_range.min > rect.width())
	{
		return false;
	}
	return true;
}

void CSDlgDeviceVideoExport::startExport()
{
	auto item = m_video_items[m_nExortIdx].second;
	int vid = VideoUtils::parseVideoSegmentId(item.getId());
	m_export_cache_frame->stop();
	uint8_t timestamp_type = 1;
	if (m_device_ptr) timestamp_type = m_device_ptr.lock()->getFrameHeadType();

	//获取相机句柄
	auto device_ptr = DeviceManager::instance().getDevice(VideoUtils::parseDeviceIp(item.getId()));
	if (device_ptr)
	{
		DeviceState state = device_ptr->getState();
		if (state != Exporting && state != Connected && state != Previewing && state != Replaying && state != ToExport)
		{
			QString error_desc;
			AgErrorCodeInfo error_code_info{};
			if (AgErrorCode::instance().get(0x02010001, error_code_info))
			{
				error_desc = error_code_info.desc;
			}

			UIUtils::showInfoMsgBox(this, error_desc);
			return;
		}
	}
	else
	{
		QString error_desc;
		AgErrorCodeInfo error_code_info{};
		if (AgErrorCode::instance().get(0x02010001, error_code_info))
		{
			error_desc = error_code_info.desc;
		}

		UIUtils::showInfoMsgBox(this, error_desc);
		return;
	}

	//切换导出状态
	b_exporting = true;
	updateWidgetsState(m_videoOpts[m_nExortIdx]);
	
	// 导出视频时先创建目录，不然不能创建视频，创建成功与否不去应该后续操作
	if (!isSaveImage(VideoFormat(item.getProperty(VideoItem::PropType::VideoFormat).toInt())))
	{
		CreateVideoDir(item.getExportPath(), FormatStringReplace(item.getRuleName(), false));
	}

	//录制之前应用且保存参数
	if (device_ptr)
	{
		device_ptr->setProperty(Device::PropWatermarkEnable, ui->checkBox_watermark->isChecked());
		device_ptr->setProperty(Device::PropVideoFormat, (int)m_video_items[mCrtIdx].second.getVideoFormat());
	}

	int count = item.getEndFrameIndex() - item.getBeginFrameIndex() + 1;
	qint64 nFrameStep = item.getProperty(VideoItem::PropType::FrameStep).toInt();
	nFrameStep += 1;
	if (nFrameStep < 1)
	{
		nFrameStep = 1;
	}
	else if (nFrameStep > count)
	{
		nFrameStep = count;
	}
	//准备全部导出参数
	ExportVideoInfo export_info{};
	export_info.v_ID = VideoUtils::parseVideoSegmentId(m_video_items[m_nExortIdx].second.getId());
	export_info.start_index = item.getProperty(VideoItem::PropType::BeginFrameIndex).toInt();
	export_info.end_index = item.getProperty(VideoItem::PropType::EndFrameIndex).toInt();
	export_info.frame_interval = nFrameStep;
	export_info.stream_type = StreamType(item.getProperty(VideoItem::PropType::StreamType).toInt());
	export_info.v_format = VideoFormat(item.getProperty(VideoItem::PropType::VideoFormat).toInt());
	export_info.enable_avi_compression = item.getAVIcompress();
	export_info.enable_watermark = item.getProperty(VideoItem::PropType::OsdVisible).toBool();
	if (device_ptr)
	{
		export_info.enable_data_correction = device_ptr->IsDataCorrectionSupported();
		export_info.data_correction_frame_offset = device_ptr->getProperty(Device::PropAngleDataCorrection).toInt();
		export_info.dev_index = device_ptr->GetDeviceIndex();
	}
	auto roi = item.getProperty(VideoItem::PropType::Roi).toRect();
	export_info.roi = { (uint16_t)roi.x(), (uint16_t)roi.y(), (uint16_t)roi.height(), (uint16_t)roi.width() };
	std::string path = ExportUtils::getExportVideoPathSelfDef(item).toUtf8().toStdString();
	//std::string path = ExportUtils::getExportVideoPath(m_video_items[mCrtIdx]).toUtf8().toStdString();
	strcpy(export_info.v_path, path.data());

	export_info.watermark_cb = std::bind(&CSDlgDeviceVideoExport::paintWatermark, this
		, std::placeholders::_1, std::placeholders::_2,
		std::placeholders::_3, std::placeholders::_4,
		std::placeholders::_5, std::placeholders::_6, std::placeholders::_7);
	export_info.luminance = item.getProperty(VideoItem::PropType::Luminance, 50).toInt();
	export_info.contrast = item.getProperty(VideoItem::PropType::Contrast, 50).toInt();
	export_info.enable_anti_color = item.isAntiColorEnable();
	export_info.display_mode = item.getDisplayMode();
	export_info.frame_rate = item.getExportFrameRate();
	export_info.enable_piv = item.getProperty(VideoItem::PropType::EnablePiv, false).toBool();
	//export_info.m_video_name_type = m_video_items[mCrtIdx].getVideoNameType();

	export_info.m_video_name = item.getRuleName().toStdString();
	export_info.m_header_type = timestamp_type;

	auto processer = device_ptr->getProcessor();
	HscColorCorrectInfo color_correct_info = item.getProperty(VideoItem::PropType::ColorCorrectInfo).value<HscColorCorrectInfo>();
	processer->setWhiteBalanceMode(color_correct_info.awb_mode_);
	if (color_correct_info.awb_mode_ == HscWhiteBalanceMode::HSC_WB_MANUAL_GAIN)
	{
		processer->setManualGain(color_correct_info.r_gain_, color_correct_info.g_gain_, color_correct_info.b_gain_);
	}
	processer->setWbEnv(color_correct_info.awb_env_);

	//界面显示导出状态
	int max = item.getEndFrameIndex() + 1;
	int min = item.getBeginFrameIndex() + 1;
	if (nFrameStep > 1)
	{
		count = 1 + (count - 1) / nFrameStep;
	}
	ui->progressBar->setRange(1, count);
	//QString progress_str = tr("Exporting %1/%2 frames.").arg(QString::number(ui->progressBar->minimum() + 1)).arg(QString::number(ui->progressBar->maximum() + 1));
	QString progress_str = tr("Exporting %1/%2 frames.").arg(QString::number(1)).arg(QString::number(count));
	ui->label_progress->setText(progress_str);
	if (m_video_items.size() > 1)
	{
		QString strMoreInfo;
		strMoreInfo = tr("Exporting(%1/%2)... ").arg(m_nExortIdx + 1).arg(m_video_items.size());
		ui->label_exporting->setText(strMoreInfo);
		ui->tbv_videolist->selectRow(m_nExortIdx);
	}

	if (device_ptr)
	{
		DeviceHandle dev_handle = device_ptr->device_handle_;
		HscResult res = Export(dev_handle, progressCallback, this, export_info);
		if (res != HSC_OK)
		{
			slotProgressChanged(0, res);
		}
	}
}


void CSDlgDeviceVideoExport::stopExport()
{
	b_exporting = false;
	updateWidgetsState(m_videoOpts[m_nExortIdx]);

	auto item = m_video_items[m_nExortIdx].second;
	auto device_ptr = DeviceManager::instance().getDevice(VideoUtils::parseDeviceIp(item.getId()));
	//auto device_ptr = DeviceManager::instance().getDevice(VideoUtils::parseDeviceIp(m_video_items[mCrtIdx].second.getId()));
	if (!device_ptr.isNull())
	{
		DeviceHandle dev_handle = device_ptr->device_handle_;
		StopExport(dev_handle);
	}
}

void CSDlgDeviceVideoExport::paintWatermark1(cv::Mat &mat, uint8_t * timestamp, uint32_t frame_no, uint32_t width, uint32_t height, uint32_t channel)
{
	if (m_nExortIdx < m_video_items.size() && m_export_cache_frame) {
		int vid = VideoUtils::parseVideoSegmentId(m_video_items[m_nExortIdx].second.getId());

		int8_t water_mark_enable = (int8_t)m_video_items[m_nExortIdx].second.getProperty(VideoItem::PropType::OsdVisible).toInt();
		QString timestamp_str;
		if (m_header_type_== HEAD_TYPE::eMType) {
			timestamp_str = DeviceUtils::formatNewTimestamp(timestamp);
		}
		else if (m_header_type_ == HEAD_TYPE::eGTypeNs ) {
			timestamp_str = DeviceUtils::formatNewTimestampG(timestamp);
		}
		else if (m_header_type_ == eS1315) {
			timestamp_str = DeviceUtils::formatNewTimestampNs(timestamp);
		}
		else {
			timestamp_str = DeviceUtils::formatTimestamp(timestamp);
		}

		if ((water_mark_enable & e_default_watermark) == e_default_watermark) m_export_cache_frame->paintDefWatermark(mat, timestamp_str, frame_no, width, height, channel, m_video_items[m_nExortIdx].second);
		if ((water_mark_enable & e_selfdef_watermark) == e_selfdef_watermark) m_export_cache_frame->paintSelfDefWatermark(vid, mat);
	}
}

void CSDlgDeviceVideoExport::paintWatermark(uint8_t * image_data, uint8_t * timestamp, uint32_t frame_no, uint32_t width, uint32_t height, uint32_t channel, int type)
{
	if (!image_data || !timestamp || width == 0 || height == 0 || channel == 0)
	{
		return;
	}
	
	int frame_size = width * height * channel;
	//for new paintWaterMark
	if (mCrtIdx >= m_video_items.size()) return;
	auto video_item = m_video_items[mCrtIdx];

	cv::Mat mat(height, width, type, image_data);
	paintWatermark1(mat, timestamp, frame_no, width, height, channel);
	return;

	QString frame_no_str = QString::number(frame_no + 1);
	//int frame_size = width * height * channel;
	QImage image;
	if (3 == channel)
	{
		frame_no_str = QString("%1").arg(frame_no + 1);
		image = QImage(image_data, width, height, width * channel, QImage::Format::Format_RGB888);
		image = image.rgbSwapped();
	}
	else if (4 == channel)
	{
		frame_no_str = QString("%1").arg(frame_no + 1);
		image = QImage(image_data, width, height, width * channel, QImage::Format::Format_RGBA8888);
		image = image.rgbSwapped();
	}
	else if (1 == channel)
	{
		//QImage::Format单通道设置为Format_Indexed8时不能在QImage上绘制图形
		image = QImage(image_data, width, height, width * channel, QImage::Format_Grayscale8);
	}

	if (image.isNull())
		return;

	QString text;
	//老采集软件水印格式
	//帧编号/总帧数 宽x高@帧率fps 曝光时间us
	//时间戳
	auto item = m_video_items[mCrtIdx].second;
	auto device_ptr = m_device_ptr.lock();
	if (device_ptr)
	{	
		agile_device::capability::Units nUnit = device_ptr->GetRealExposureTimeUnit();
		if (agile_device::capability::Units::kUnit100ns == nUnit)
		{
			float dbexposure_time = (float)item.getExposureTime() / 10.0f;
			text.append(QString("%1/%2 %3@%4fps %5us<br/>")
				.arg(frame_no_str).arg(item.getTotalFrame()).arg(item.getResolution()).arg(item.getFrameRate()).arg(QString::asprintf("%.2f", dbexposure_time)));
		}
		else if (agile_device::capability::Units::kUnit10ns == nUnit)
		{
			float dbexposure_time = (float)item.getExposureTime() / 100.0f;
			text.append(QString("%1/%2 %3@%4fps %5us<br/>")
				.arg(frame_no_str).arg(item.getTotalFrame()).arg(item.getResolution()).arg(item.getFrameRate()).arg(QString::asprintf("%.2f", dbexposure_time)));
		}
		else 
		{
			text.append(QString("%1/%2 %3@%4fps %5us<br/>")
				.arg(frame_no_str).arg(item.getTotalFrame()).arg(item.getResolution()).arg(item.getFrameRate()).arg(item.getExposureTime()));
		}
	}
	else
	{
		text.append(QString("%1/%2 %3@%4fps %5us<br/>")
			.arg(frame_no_str).arg(item.getTotalFrame()).arg(item.getResolution()).arg(item.getFrameRate()).arg(item.getExposureTime()));
	}
// 	text.append(QString("%1/%2 %3@%4fps %5us<br/>")
// 		.arg(frame_no_str).arg(m_video_item.getTotalFrame()).arg(m_video_item.getResolution()).arg(m_video_item.getFrameRate()).arg(m_video_item.getExposureTime()));
	text.append(QString("%1").arg(DeviceUtils::formatTimestamp(timestamp)));
	//帧编号和时间戳格式
// 	text = QString("<font color = #FF0000>%1</font>").arg(text);
	ExportUtils::paintWatermark2Image(image, text, 0, ExportUtils::WATERMARK_BOTTOMLEFT);

	if (1 != channel)
	{
		image = image.rgbSwapped();
		memcpy(image_data, image.constBits(), frame_size);
	}
}

void progressCallback(DeviceHandle handle, uint32_t value, uint32_t state, void * user_ptr)
{
	if ( user_ptr)
	{
		CSDlgDeviceVideoExport * widget = static_cast<CSDlgDeviceVideoExport*>(user_ptr);
		if (widget)
		{
			widget->progressChanged(handle, value, state);
		}
	}
}

QString CSDlgDeviceVideoExport::FormatStringReplace(QString strText, bool bImage)
{
	QString strNewText = strText;
	if (strNewText.isNull())
	{
		return strNewText;
	}
	if (strNewText.isEmpty())
	{
		return strNewText;
	}
	int nPos = strNewText.indexOf(conststrVideoName, 0, Qt::CaseInsensitive);
	if (nPos > -1)
	{
		//视频名称
		QString strName = m_video_items[mCrtIdx].second.getName();
		strNewText.replace(conststrVideoName, m_video_items[mCrtIdx].second.getName(), Qt::CaseInsensitive);
	}

	nPos = strNewText.indexOf(conststrIP, 0, Qt::CaseInsensitive);
	if (nPos > -1)
	{
		strNewText.replace(conststrIP, VideoUtils::parseDeviceIp(m_video_items[mCrtIdx].second.getId()), Qt::CaseInsensitive);
	}
	
	nPos = strNewText.indexOf(conststrRecordtime, 0, Qt::CaseInsensitive);
	if (nPos > -1)
	{
		QString strcapture_time = m_video_items[mCrtIdx].second.getTimeStamp();
		QDateTime time = QDateTime::fromString(strcapture_time, "yyyy-MM-dd hh:mm:ss");
		strcapture_time = time.toString("yyyyMMdd-HHmmss");
		strNewText.replace(conststrRecordtime, strcapture_time, Qt::CaseInsensitive);
	}

	nPos = strNewText.indexOf(conststrFramerate, 0, Qt::CaseInsensitive);
	if (nPos > -1)
	{
		//帧率
		strNewText.replace(conststrFramerate, QString::number(m_video_items[mCrtIdx].second.getFPS()), Qt::CaseInsensitive);
	}

	nPos = strNewText.indexOf(conststrResolutionratio, 0, Qt::CaseInsensitive);
	if (nPos > -1)
	{
		//帧率
		strNewText.replace(conststrResolutionratio, m_video_items[mCrtIdx].second.getResolution(), Qt::CaseInsensitive);
	}
	nPos = strNewText.indexOf(conststrTimestamp, 0, Qt::CaseInsensitive);
	if (nPos > -1)
	{
		strNewText.replace(conststrTimestamp, m_video_items[mCrtIdx].second.getOriginalTimeStamp(), Qt::CaseInsensitive);
	}
	if (bImage)
	{
		nPos = strNewText.indexOf(conststrFramenumber, 0, Qt::CaseInsensitive);
		if (nPos > -1)
		{
			strNewText.replace(conststrFramenumber, QString::number(m_video_items[mCrtIdx].second.getBeginFrameIndex()), Qt::CaseInsensitive);
		}
		nPos = strNewText.indexOf(conststrFramesubnumber, 0, Qt::CaseInsensitive);
		if (nPos > -1)
		{
			strNewText.replace(conststrFramesubnumber, QString::number(0), Qt::CaseInsensitive);
		}
	}


	nPos = strNewText.indexOf(conststrRecordDate, 0, Qt::CaseInsensitive);
	if (nPos > -1)
	{
		QString strcapture_time = m_video_items[mCrtIdx].second.getRecordDate();
		strNewText.replace(conststrRecordDate, strcapture_time, Qt::CaseInsensitive);
	}

	nPos = strNewText.indexOf(conststrFormat, 0, Qt::CaseInsensitive);
	if (nPos > -1)
	{
		QString strFormat = conststrFormat;
		VideoFormat ft = (VideoFormat)m_video_items[mCrtIdx].second.getVideoFormat();
		switch (ft)
		{
		case VIDEO_RHVD:
			strFormat = "rhvd";
			break;
		case VIDEO_AVI:
			strFormat = "avi";
			break;
		case VIDEO_BMP:
			strFormat = "bmp";
			break;
		case VIDEO_JPG:
			strFormat = "jpg";
			break;
		case VIDEO_PNG:
			strFormat = "png";
			break;
		case VIDEO_MP4:
			strFormat = "mp4";
			break;
		case VIDEO_TIF:
			strFormat = "tif";
			break;
		case VIDEO_RAW:
			strFormat = "raw";
			break;
		default:
			break;
		}
		strNewText.replace(conststrFormat, strFormat, Qt::CaseInsensitive);
	}
	return strNewText;
}


void CSDlgDeviceVideoExport::slotSetTransparentLineEditValue(int value)
{
	int pos = ui->slider_transparent->value();
	int nOpacity = 100 - pos;
	if (nOpacity < 0)
	{
		nOpacity = 0;
	}
	else if (nOpacity > 100)
	{
		nOpacity = 100;
	}
	if (nOpacity >= constPercentValueMin && nOpacity <= constPercentValueMax)
	{
		ui->spinBox_transparent->setValue(100-nOpacity);
	}
	if (m_pSelectedWaterItem)
	{
		m_pSelectedWaterItem->setOpacity(1.0*nOpacity / 100);
		for (auto &tmp : m_videoOpts[mCrtIdx].waterMarks)
		{
			if (tmp.pItem == m_pSelectedWaterItem)
			{
				tmp.nTransparent = 100 - nOpacity;
			}
		}
	}
}

void CSDlgDeviceVideoExport::slotSetRotateLineEditValue(int value)
{
	int pos = ui->slider_rotate->value();
	if (pos >= constAngleValueMin && pos <= constAngleValueMax)
	{
		ui->spinBox_rotate->setValue(pos);
	}
	if (m_pSelectedWaterItem)
	{
		m_pSelectedWaterItem->setRotation(pos);
		for (auto &tmp : m_videoOpts[mCrtIdx].waterMarks)
		{
			if (tmp.pItem == m_pSelectedWaterItem)
			{
				tmp.nRotate = pos;
			}
		}
	}
}

void CSDlgDeviceVideoExport::slotSetWatermarkLineEditValue(int value)
{
	int pos = ui->slider_watermark->value();
	if (pos >= constZoomValueMin && pos <= constZoomValueMax)
	{
		ui->spinBox_watermark->setValue(pos);
	}
	qreal qvalue = pos / 100.0;
	if (qvalue == 0)
	{
		qvalue = 0.01;
	}
	if (m_pSelectedWaterItem)
	{
		m_pSelectedWaterItem->setScale(qvalue);
		for (auto &tmp : m_videoOpts[mCrtIdx].waterMarks)
		{
			if (tmp.pItem == m_pSelectedWaterItem)
			{
				tmp.nWaterSize = pos;
			}
		}
	}
}

void CSDlgDeviceVideoExport::slotTabViewWatermarkClicked(QModelIndex index)
{
	if (!index.isValid())
	{
		return;
	}

	int nColumn = index.column();
	int nRow = index.row();
	if (0 == nColumn || 1 == nColumn)
	{
		QString strName = m_watermark_table_model_ptr->data(m_watermark_table_model_ptr->index(nRow, 1)).toString();
		for (auto &tmp:m_watermark_list)
		{
			if (tmp.strFileName == strName)
			{
				if (m_pSelectedWaterItem)
				{
					m_pSelectedWaterItem->setSelected(false);
				}
				m_pSelectedWaterItem = tmp.pItem;
				if (m_pSelectedWaterItem)
				{
					tmp.nTransparent = 100 - m_pSelectedWaterItem->opacity() * 100;
					tmp.nRotate = m_pSelectedWaterItem->rotation();
					tmp.nWaterSize = m_pSelectedWaterItem->scale() * 100;
				}
				ui->slider_transparent->setValue(tmp.nTransparent);
				ui->slider_rotate->setValue(tmp.nRotate);
				ui->slider_watermark->setValue(tmp.nWaterSize);
				if (m_pSelectedWaterItem)
				{
					m_pSelectedWaterItem->setSelected(true);
					SetWatermarkerCtrEnable(m_pSelectedWaterItem->isVisible());
				}
				break;
			}
		}
	}
	else if (2 == nColumn)
	{
		QString strName = m_watermark_table_model_ptr->data(m_watermark_table_model_ptr->index(nRow, 1)).toString();
		m_watermark_table_model_ptr->removeRow(nRow);

		int nSize = m_watermark_list.size();
		for (int i = 0; i < nSize; i++)
		{
			if (m_watermark_list[i].strFileName == strName)
			{
				if (m_RectItem)
				{
					if (m_watermark_list[i].pItem)
					{
						if (scene)
						{
							scene->removeItem(m_watermark_list[i].pItem);
						}
					}
				}
				m_watermark_list.removeAt(i);
				//移除每个视频内的此水印信息（该信息用于记录水印是否显示）
				for (auto &optIter : m_videoOpts)
				{
					for (int j = 0; j < optIter.waterMarks.size(); j++)
					{
						if (optIter.waterMarks[j].strFileName == strName)
						{
							optIter.waterMarks.removeAt(j);
						}
					}
				}
				break;
			}
		}
		m_pSelectedWaterItem = nullptr;
		InitWatermarkerCtr();
		SetWatermarkerCtrEnable(false);
	}
}

void CSDlgDeviceVideoExport::slotTabViewCurrentSelectChanged(const QModelIndex &current, const QModelIndex &previous)
{
	
	if (!m_bInitShow)
	{
		m_bInitShow = true;
		ui->tbv_videolist->selectRow(0);
	}
	int nIndex = current.row();
	if (nIndex < m_videoOpts.size())
	{
		if (mCrtIdx == nIndex && mCrtIdx == 0)
		{
		}
		else
		{
			UpdateVideoSaveInfo(mCrtIdx, m_videoOpts[mCrtIdx]);
		}
		mCrtIdx = nIndex;
		UpdateVideoUI(m_videoOpts[nIndex]);
		UpdateVideoImage(nIndex, m_videoOpts[nIndex]);
		UpdateWatermarkerListInfo(m_videoOpts[nIndex]);
		updateWidgetsState(m_videoOpts[nIndex]);
	}
}

void CSDlgDeviceVideoExport::slotExportRangeDataChanged(VideoInfoRecordList info)
{
	if (info.nIndex == mCrtIdx)
	{
		m_videoOpts[mCrtIdx].uiStart = info.videoInfo.uiStartFrame;
		m_videoOpts[mCrtIdx].uiEnd = info.videoInfo.uiEndFrame;
		ui->doubleSpinBox_jump->setRange(0, getRange(m_videoOpts[mCrtIdx]));
	}
}

bool CSDlgDeviceVideoExport::disableAVICompress(VideoSaveOpt opt)
{
	if (((StreamType)opt.stream_type == TYPE_H264) || ((VideoFormat)opt.videoformat != VIDEO_AVI)) return true;
	return false;
}


int32_t CSDlgDeviceVideoExport::getVideoNameType()
{
	int32_t type = 0x00;
	return type;
}

void CSDlgDeviceVideoExport::SetWatermarkerCtrEnable(bool bEnable)
{
	if (ui->slider_transparent)
	{
		ui->slider_transparent->setEnabled(bEnable);
	}
	if (ui->slider_rotate)
	{
		ui->slider_rotate->setEnabled(bEnable);
	}
	if (ui->slider_watermark)
	{
		ui->slider_watermark->setEnabled(bEnable);
	}
	if (ui->spinBox_transparent)
	{
		ui->spinBox_transparent->setEnabled(bEnable);
	}
	if (ui->spinBox_rotate)
	{
		ui->spinBox_rotate->setEnabled(bEnable);
	}
	if (ui->spinBox_watermark)
	{
		ui->spinBox_watermark->setEnabled(bEnable);
	}
}

double CSDlgDeviceVideoExport::getDiskFreeSizeGB(QString path)
{
    //提前创建文件夹
    QDir dir;
    dir.mkpath(path);

    // 剩余磁盘空间
    QStorageInfo storage_info = QStorageInfo(path);
    for (int i = 0; i < 10; i++)//等待磁盘信息
    {
        if (storage_info.isValid() && storage_info.isReady())
        {
            break;
        }
        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
    }
    return double(storage_info.bytesAvailable()) / 1024 / 1024 / 1024;
}

int8_t CSDlgDeviceVideoExport::getWaterMarkValue(int index)
{
	if (index >= m_video_items.size()) return 0;
	auto &item = m_video_items[index];
	int vid = VideoUtils::parseVideoSegmentId(item.second.getId());
	int8_t mark = 0;
	bool flag = item.second.isOsdVisible();
	if (flag) mark |= e_default_watermark;

	flag = false;
	if (m_export_cache_frame) flag = m_export_cache_frame->hasWaterMark(vid);
	if (flag) mark |= e_selfdef_watermark;
	return mark;
}

void CSDlgDeviceVideoExport::InitWatermarkerCtr()
{
	if (ui->slider_transparent)
	{
		ui->slider_transparent->setValue(0);
	}
	if (ui->slider_rotate)
	{
		ui->slider_rotate->setValue(0);
	}
	if (ui->slider_watermark)
	{
		ui->slider_watermark->setValue(100);
	}
}

void CSDlgDeviceVideoExport::UpdateShowFormatString(VideoSaveOpt opt)
{
	if (mCrtIdx >= m_video_items.size())
	{
		return;
	}
	if (!ui->comboBox_add_file_name)
	{
		return;
	}
	if (ui->comboBox_add_file_name->count() < 8)
	{
		return;
	}
	QListView *view = qobject_cast<QListView *>(ui->comboBox_add_file_name->view());
	if (view)
	{
		VideoFormat vf = (VideoFormat)opt.videoformat;
		bool bHidden = true;
		if (isSaveImage(opt.videoformat))
		{
			bHidden = false;
		}
		// 6、7为帧编号及帧子编号，视频格式时隐藏此选项
		view->setRowHidden(8, bHidden);
		view->setRowHidden(9, bHidden);
	}
}

void CSDlgDeviceVideoExport::UpdateEnableWatermarker(VideoSaveOpt opt)
{
	bool bEnable = GetWaterMarkOptionEnable(opt);

	ui->pushButton_addWatermark->setEnabled(!b_exporting && bEnable);
	ui->tableViewWatermark->setEnabled(!b_exporting && bEnable);
	if (!bEnable)
	{
		for (auto &temp : m_watermark_list)
		{
			if (temp.pItem)
			{
				temp.pItem->setVisible(bEnable);
			}
		}
		SetWatermarkerCtrEnable(false);
	}
	else
	{
		for (auto &temp : m_watermark_list)
		{
			if (temp.pItem)
			{
				temp.pItem->setVisible(temp.bVisible);
			}
		}
		SetWatermarkerCtrEnable(false);
	}
}

void CSDlgDeviceVideoExport::UpdateFileName(VideoSaveOpt opt)
{
	QString strText;
	bool bImage = isSaveImage(opt.videoformat);
	if (bImage)
	{
		strText = opt.ImageRuleName;
	}
	else
	{
		strText = opt.videoRuleName;
	}
	m_strVideName = strText;
	ui->lineEdit_video_name->setText(strText);
	ui->lineEdit_browse_video_name->setText(FormatStringReplace(strText, bImage));
	QString strPath;
	if (FormatStringToFilePath(strText, ui->lineEdit_save_path->text(), opt, strPath))
	{
		ui->lineEdit_browse_video_name->setToolTip(strPath);
	}
}

void CSDlgDeviceVideoExport::slotfocusItemChanged(QGraphicsItem *newFocus, QGraphicsItem *oldFocus, Qt::FocusReason reason)
{
	if (reason != Qt::MouseFocusReason)
	{
		return;
	}
	if (newFocus == m_pSelectedWaterItem)
	{
		return;
	}
	for (auto item : m_watermark_list)
	{
		if (item.pItem == newFocus)
		{
			m_pSelectedWaterItem = item.pItem;
			if (m_watermark_table_model_ptr)
			{
				int nCount = m_watermark_table_model_ptr->rowCount();
				for (int i = 0; i < nCount; i++)
				{
					QStandardItem *subitem = m_watermark_table_model_ptr->item(i,1);
					if (subitem)
					{
						QString strName = m_watermark_table_model_ptr->data(m_watermark_table_model_ptr->indexFromItem(subitem)).toString();
						if (strName == item.strFileName)
						{
							ui->tableViewWatermark->selectRow(m_watermark_table_model_ptr->indexFromItem(subitem).row());
							break;
						}
					}
				}
			}

			int nTransparent = item.nTransparent;
			int nRotate = item.nRotate;
			int nWaterSize = item.nWaterSize;
			if (m_pSelectedWaterItem)
			{
				nTransparent = 100 - m_pSelectedWaterItem->opacity() * 100;
				nRotate = m_pSelectedWaterItem->rotation();
				nWaterSize = m_pSelectedWaterItem->scale() * 100;
			}
			ui->slider_transparent->setValue(nTransparent);
			ui->slider_rotate->setValue(nRotate);
			ui->slider_watermark->setValue(nWaterSize);
			if (m_pSelectedWaterItem)
			{
				SetWatermarkerCtrEnable(m_pSelectedWaterItem->isVisible());
			}
		}
	}
}

void CSDlgDeviceVideoExport::slotWatermarkItemCheck(const QModelIndex &index,bool bChecked)
{
	int nRow = index.row();
	QString strName = m_watermark_table_model_ptr->data(m_watermark_table_model_ptr->index(nRow, 1)).toString();
	int nWaterIndex = 0;
	for (auto iter = m_watermark_list.begin(); iter!= m_watermark_list.end();iter++)
	{
		if (iter->strFileName == strName)
		{
			iter->pItem->setVisible(bChecked);
			m_watermark_list[nWaterIndex].bVisible = bChecked;
			for (auto &tmp : m_videoOpts[mCrtIdx].waterMarks)
			{
				if (tmp.strFileName == strName)
				{
					tmp.bVisible = bChecked;
				}
			}
		}
		nWaterIndex++;
	}

	if (m_pSelectedWaterItem)
	{
		SetWatermarkerCtrEnable(m_pSelectedWaterItem->isVisible());
	}
}

ErrorFileNameValid CSDlgDeviceVideoExport::isPathFileValid(QString strFileName, QString strPath, VideoSaveOpt opt)
{
	if (strFileName.isEmpty())
	{
		return EFNV_File_Name_Empty;
	}
	QRegExp reQRegExp("^[^\:\*\?\"\<\>\|(\\\\)]+$");
	if (!reQRegExp.exactMatch(strFileName))
	{
		return EFNV_SpecialCharacters;
	}
	QString strText = strFileName;
	// 查找帧编号在命名中使用的次数，帧编号的增加会改变文件名称长度
	QStringList vctFrameNOList = strText.split(conststrTimestamp);
	int nMoreCharNum = 0;
	int nFrameNoSize = vctFrameNOList.size();
	if (nFrameNoSize > 0)
	{
		qint64 uiNum = opt.uiEnd;
		QString strNum = QString::number(uiNum);
		nMoreCharNum = nFrameNoSize*(strNum.length() - 1);
	}
	QStringList vctList = strText.split("/");
	if (vctList.size() > 2)
	{
		return EFNV_More_Dir;
	}
	else if (vctList.size() == 2)
	{
		QString strSubText = vctList[0];
		if (strSubText.isEmpty())
		{
			return EFNV_Dir_Invalid;
		}
		int nPos = strSubText.indexOf(conststrTimestamp, 0, Qt::CaseInsensitive);
		if (nPos > -1)
		{
			return EFNV_Dir_Change;
		}
		nPos = strSubText.indexOf(conststrFramenumber, 0, Qt::CaseInsensitive);
		if (nPos > -1)
		{
			return EFNV_Dir_Change;
		}
		nPos = strSubText.indexOf(conststrFramesubnumber, 0, Qt::CaseInsensitive);
		if (nPos > -1)
		{
			return EFNV_Dir_Change;
		}
	}
	strText = FormatStringReplace(strText, isSaveImage(opt.videoformat));

	QString strPathText = strPath;
	strPathText += "/";
	strPathText += strText;
	strPathText += ".";
	strPathText += ExportUtils::formatToStr(opt.videoformat);
	// 判断文件路径名称+（导出最大帧号扩增的长度）是否大于文件最大路径长度
	if (strPathText.length() + nMoreCharNum > m_max_path_name)
	{
		return EFNV_More_length;
	}
	std::string path = strPathText.toUtf8().toStdString();
	if (path.length() + nMoreCharNum > m_max_path_name)
	{
		return EFNV_More_length;
	}
	return EFNV_Success;
}

bool CSDlgDeviceVideoExport::isSaveImage(VideoFormat videoformat)
{
	if (videoformat == VIDEO_BMP || videoformat == VIDEO_JPG || videoformat == VIDEO_TIF || videoformat == VIDEO_PNG)
	{
		return true;
	}
	return false;
}

bool CSDlgDeviceVideoExport::FormatStringToFilePath(QString strFileName, QString strPath, VideoSaveOpt opt, QString& strFilePath)
{
	QString strText = strFileName;
	// 查找帧编号在命名中使用的次数，帧编号的增加会改变文件名称长度
	QStringList vctFrameNOList = strText.split(conststrTimestamp);
	int nMoreCharNum = 0;
	int nFrameNoSize = vctFrameNOList.size();
	if (nFrameNoSize > 0)
	{
		qint64 uiNum = opt.uiEnd;
		QString strNum = QString::number(uiNum);
		nMoreCharNum = nFrameNoSize*(strNum.length() - 1);
	}
	QStringList vctList = strText.split("/");
	if (vctList.size() > 2)
	{
		return false;
	}
	else if (vctList.size() == 2)
	{
		if (!isSaveImage(opt.videoformat))
		{
			return false;
		}
		QString strSubText = vctList[0];
		int nPos = strSubText.indexOf(conststrTimestamp, 0, Qt::CaseInsensitive);
		if (nPos > -1)
		{
			return false;
		}
		nPos = strSubText.indexOf(conststrFramenumber, 0, Qt::CaseInsensitive);
		if (nPos > -1)
		{
			return false;
		}
		nPos = strSubText.indexOf(conststrFramesubnumber, 0, Qt::CaseInsensitive);
		if (nPos > -1)
		{
			return false;
		}
	}
	strText = FormatStringReplace(strText, isSaveImage(opt.videoformat));

	strFilePath = strPath;
	strFilePath += "/";
	strFilePath += strText;
	strFilePath += ".";
	strFilePath += ExportUtils::formatToStr(opt.videoformat);
	// 判断文件路径名称+（导出最大帧号扩增的长度）是否大于文件最大路径长度
	if (strFilePath.length() + nMoreCharNum > m_max_path_name)
	{
		return false;
	}
	return true;
}

QString CSDlgDeviceVideoExport::getFileNameErrorString(ErrorFileNameValid nError)
{
	QString strErr;
	switch (nError)
	{
	case EFNV_Success:
		break;
	case EFNV_More_Dir:
		strErr = tr("File directory can only have one level!");
		break;
	case EFNV_Dir_Change:
		strErr = tr("File directory cannot have changed wildcards!");
		break;
	case EFNV_SpecialCharacters:
		strErr = tr("File name cannot have special characters!");
		break;
	case EFNV_More_length:
		strErr = tr("File name and path cannot exceed %1 characters!").arg(m_max_path_name);
		break;
	case EFNV_File_Name_Empty:
		strErr = tr("File name cannot be empty!");
		break;
	case EFNV_Dir_Invalid:
		strErr = tr("File directory cannot be empty!");
		break;
	default:
		break;
	}
	return qMove(strErr);
}

void CSDlgDeviceVideoExport::CreateVideoDir(QString strPath, QString strFileContainDir)
{
	QStringList vctList = strFileContainDir.split("/");
	int nCount = vctList.size();
	if (nCount > 1)
	{
		QString strPPath = strPath;
		for (int i = 0; i < nCount - 1; i++)
		{
			if (vctList[i].isEmpty())
			{
				continue;
			}
			strPPath += QDir::separator();
			strPPath += vctList[i];
			QDir dir(strPPath);
			if (dir.exists())
			{
				break;
			}
			dir.mkdir(strPPath);
		}
	}
}

void CSDlgDeviceVideoExport::UpdateVideoImage(int idx, VideoSaveOpt opt)
{
	if (m_RectItem)
	{
		scene->removeItem(m_RectItem);
		delete m_RectItem;
		m_RectItem = nullptr;
	}
	m_pSelectedWaterItem = nullptr;

	qreal qWidth = opt.nRoiWidth, qHeight = opt.nRoiHeight;
	QImage image;
	if (getImage(idx, image))
	{
		QPixmap pPixmap = QPixmap::fromImage(image);
		m_RectItem = new QGraphicsPixmapItem(pPixmap);
	}
	else
	{
		QPixmap pPixmap = QPixmap::fromImage(QImage(qWidth, qHeight, QImage::Format_RGB888));
		m_RectItem = new QGraphicsPixmapItem(pPixmap);
	}
	m_RectItem->setFlag(QGraphicsItem::ItemClipsChildrenToShape);
	//m_RectItem->setFlag(QGraphicsItem::ItemDoesntPropagateOpacityToChildren);
	qWidth = image.width();
	qHeight = image.height();
	scene->addItem(m_RectItem);
	scene->setSceneRect(QRectF(0, 0, qWidth, qHeight));
	ui->waterframe->AutoAdjust();
}

void CSDlgDeviceVideoExport::UpdateWatermarkerListInfo(VideoSaveOpt &opt)
{
	SetWatermarkerCtrEnable(false);
	if (m_watermark_table_model_ptr)
	{
		m_watermark_table_model_ptr->clear();
	}
	bool bAdd = false;
	if (m_RectItem)
	{
		for (auto &temp : m_watermark_list)
		{
			QFile file(temp.strFileID);
			if (!file.exists())
			{
				continue;
			}

			QImageReader image_reader;
			image_reader.setDecideFormatFromContent(true);
			image_reader.setFileName(temp.strFileID);
			QImage img = image_reader.read();

			QPixmap pImage = QPixmap::fromImage(img);
			CSImageGraphiceItem *pSubItem = new CSImageGraphiceItem(pImage);
			pSubItem->setParentItem(m_RectItem);
			pSubItem->setPos(temp.X, temp.Y);
			pSubItem->setOffset(-temp.nWidth, -temp.nHeight);
			pSubItem->setFlag(QGraphicsItem::ItemIsMovable);
			pSubItem->setFlag(QGraphicsItem::ItemClipsToShape);
			pSubItem->setFlag(QGraphicsItem::ItemIsFocusable);
			pSubItem->setFlag(QGraphicsItem::ItemIsSelectable);
			pSubItem->setFlag(QGraphicsItem::ItemIgnoresParentOpacity);
			pSubItem->setAcceptHoverEvents(true);
			pSubItem->setOpacity((100 - temp.nTransparent) / 100.0);
			pSubItem->setRotation(temp.nRotate);
			pSubItem->setScale(temp.nWaterSize / 100.0);

			QStandardItem* button = new QStandardItem(QIcon(tr(":/images/window_control_icon_close.png")), tr(""));
			button->setToolTip(tr("close"));
			QStandardItem * name = new QStandardItem();
			name->setText(temp.strFileName);
			name->setToolTip(temp.strFileID);
			name->setData(temp.strFileName);
			QStandardItem *item = new QStandardItem();
			QList<QStandardItem *> waterInfo;
			waterInfo << item << name << button;
			m_watermark_table_model_ptr->appendRow(waterInfo);
			
			temp.pItem = pSubItem;
			temp.bVisible = true;
			bool bNoExist = true;
			for (auto optIter : opt.waterMarks)
			{
				if (optIter.strFileName == temp.strFileName)
				{
					temp.bVisible = optIter.bVisible;
					temp.pItem->setVisible(temp.bVisible);					
					bNoExist = false;		
				}
			}
			if (bNoExist) opt.waterMarks.append(temp);		
			emit SignalChangeCheckBoxState(m_watermark_table_model_ptr, item->index(), temp.bVisible);
			bAdd = true;
		}
		if (bAdd)
		{
			if (m_pSelectedWaterItem)
			{
				m_pSelectedWaterItem->setSelected(false);
			}
			m_pSelectedWaterItem = m_watermark_list.at(0).pItem;
			if (m_pSelectedWaterItem)
			{
				int nTransparent = 100 - m_pSelectedWaterItem->opacity() * 100;
				int nRotate = m_pSelectedWaterItem->rotation();
				int nWaterSize = m_pSelectedWaterItem->scale() * 100;
				ui->slider_transparent->setValue(nTransparent);
				ui->slider_rotate->setValue(nRotate);
				ui->slider_watermark->setValue(nWaterSize);
				m_pSelectedWaterItem->setSelected(true);
				SetWatermarkerCtrEnable(m_pSelectedWaterItem->isVisible());
			}
			ui->tableViewWatermark->selectRow(0);
			ui->tableViewWatermark->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
			ui->tableViewWatermark->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
			ui->tableViewWatermark->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
			ui->tableViewWatermark->horizontalHeader()->resizeSection(0, 20);
			ui->tableViewWatermark->horizontalHeader()->resizeSection(2, 20);		
		}
	}
	UpdateEnableWatermarker(opt);
}

void CSDlgDeviceVideoExport::UpdateWaterMarkOption(VideoSaveOpt opt)
{
	bool bWaterEnable = GetWaterMarkOptionEnable(opt);
	if (bWaterEnable)
	{
		ui->checkBox_watermark->setEnabled(true && !b_exporting);
		ui->checkBox_watermark->setChecked(opt.enableVideoInfo);
	}
	else
	{
		ui->checkBox_watermark->setEnabled(false);
		ui->checkBox_watermark->setChecked(false);
	}
}

void CSDlgDeviceVideoExport::updateAviCompressOption(VideoSaveOpt opt)
{
	if (((StreamType)opt.stream_type == TYPE_H264) || ((VideoFormat)opt.videoformat != VIDEO_AVI)) {
		ui->checkBox_enableAVIcompress->setEnabled(false);
	}
	else {
		ui->checkBox_enableAVIcompress->setEnabled(true && !b_exporting);
	}

	//AVI压缩
	ui->checkBox_enableAVIcompress->setChecked(opt.enableAVIcompress);

	if (disableAVICompress(opt))
	{
		ui->checkBox_enableAVIcompress->setChecked(false);
	}
}

void CSDlgDeviceVideoExport::updateDepthHint(VideoSaveOpt opt)
{
	uint8_t export_depth = 8;
	if (opt.stream_type == TYPE_RAW8 && (opt.videoformat == VIDEO_TIF || opt.videoformat == VIDEO_RHVD))
	{
		export_depth = opt.nBpp;
	}
	QString hint = tr("Data Depth: %1bit, Export Depth: %2bit").arg(opt.nBpp).arg(export_depth);
	ui->label_depth_hint->setText(hint);
}

void CSDlgDeviceVideoExport::updateWidgetsState(VideoSaveOpt opt)
{
	bool b_enable = !b_exporting;//正在导出时部分控件禁用
	bool b_visible = b_exporting;//只有在导出时才可以显示

	bool bWaterEnable = GetWaterMarkOptionEnable(opt);
	bool bEnableExport = GetEnableExport();
	bool bRateEnabled = !isSaveImage(opt.videoformat);

	ui->comboBox_color_mode->setEnabled(b_enable);//色彩模式
	ui->comboBox_save_format->setEnabled(b_enable);//保存格式
	ui->pushButton_browse->setEnabled(b_enable);//预览按钮
	ui->pushButton_open->setEnabled(b_enable);//打开按钮
	ui->pushButton_default->setEnabled(b_enable); //保存默认配置按钮
	ui->pushButton_start->setEnabled(b_enable && bEnableExport);//开始导出按钮
	ui->lineEdit_video_name->setEnabled(b_enable);//视频名称
	//ui->spinBox_display_rate->setEnabled(b_enable && bRateEnabled);//播放帧率
	ui->comboBox_display_rate->setEnabled(b_enable && bRateEnabled);//播放帧率
	ui->doubleSpinBox_jump->setEnabled(b_enable);//抽帧导出 
	ui->comboBox_save_unit->setEnabled(b_enable);//抽帧导出单位 

	if (!b_enable)
	{
		ui->comboBox_add_file_name->setEnabled(b_enable);
		ui->slider_rotate->setEnabled(b_enable);
		ui->slider_transparent->setEnabled(b_enable);
		ui->slider_watermark->setEnabled(b_enable);
		ui->spinBox_transparent->setEnabled(b_enable);
		ui->spinBox_rotate->setEnabled(b_enable);
		ui->spinBox_watermark->setEnabled(b_enable);
		ui->pushButton_addWatermark->setEnabled(b_enable);
		ui->tableViewWatermark->setEnabled(b_enable);
		ui->waterframe->setEnabled(b_enable);
		ui->checkBox_watermark->setEnabled(b_enable);//视频信息
		ui->checkBox_enableAVIcompress->setEnabled(b_enable);
	}
	
	//根据当前状态控制窗口大小
	ui->groupBox_export_progress->setVisible(b_visible);
	int widget_height = 0;
	if (b_visible)
	{
		//widget_height = m_widget_height_exporting;
		widget_height = m_widget_height_normal +
			ui->groupBox_export_progress->height() +
			ui->verticalLayout_2->spacing();
	}
	else
	{
		widget_height = m_widget_height_normal;
	}
	this->setMaximumHeight(widget_height);
	this->setMinimumHeight(widget_height);
}

void CSDlgDeviceVideoExport::UpdateExportButtonStatus()
{
	if (b_exporting)
	{
		return;
	}
	ErrorFileNameValid nRet = EFNV_Success;
	int nIndex = 0;
	int nCount = m_videoOpts.size();
	for (int i = 0; i < nCount; i++)
	{
		if (isSaveImage(m_videoOpts[i].videoformat))
		{
			nRet = isPathFileValid(m_videoOpts[i].ImageRuleName, m_videoOpts[i].export_path, m_videoOpts[i]);
		}
		else
		{
			nRet = isPathFileValid(m_videoOpts[i].videoRuleName, m_videoOpts[i].export_path, m_videoOpts[i]);
		}
		if (nRet != EFNV_Success)
		{
			nIndex = i;
			break;
		}
	}
	QString strTips;
	if (nRet != EFNV_Success && nIndex < nCount)
	{
		strTips = getFileNameErrorString(nRet);
		strTips = tr("%1/%2 %3").arg(nIndex+1).arg(nCount).arg(getFileNameErrorString(nRet));
		ui->pushButton_start->setEnabled(false);
		ui->pushButton_start->setToolTip(strTips);
		m_bPathIsValid = false;
	}
	else
	{
		if (!m_bPathIsValid)
		{
			m_bPathIsValid = true;
		}
		ui->pushButton_start->setEnabled(true);
		ui->pushButton_start->setToolTip(strTips);
	}
}

void CSDlgDeviceVideoExport::InitUI()
{
	/*for (size_t i = 0; i < m_video_items.size(); i++)
	{
	int vid = VideoUtils::parseVideoSegmentId(m_video_items[i].second.getId());
	if (m_export_cache_frame) m_export_cache_frame->doExportAsync(vid, m_video_items[i].second.getBeginFrameIndex(), m_video_items[i].second.getEndFrameIndex(), 2, 1);
	}*/
	scene = new QGraphicsScene(this);
	scene->setBackgroundBrush(QBrush(Qt::gray));
	ui->waterframe->view()->setScene(scene);
	ui->waterframe->view()->setDragMode(QGraphicsView::ScrollHandDrag);
	ui->waterframe->view()->setInteractive(true);
	connect(scene, &QGraphicsScene::focusItemChanged, this, &CSDlgDeviceVideoExport::slotfocusItemChanged);
	connect(ui->tbv_videolist, &QTableView::entered, this, [=](const QModelIndex& index) {
		if (index.isValid()) {
			if (m_video_items.size() > index.row()) {
				auto item = m_video_items[index.row()].second;
				VideoListThumbnail::ThumbnailInfo info;
				QString device_desc;
				auto device_ptr = m_device_ptr.lock();
				if (device_ptr)
				{
					device_desc = device_ptr->getDescription();
					info.thumbnail_mat = device_ptr->getVideoClipScale(item.getId(),&item);
				}
				else
				{
					device_desc = VideoUtils::parseDeviceIp(item.getId());;
				}
				info.camera_name = device_desc;
				QString qstrTime = item.getTimeStamp();
				QDateTime dateTime = QDateTime::fromString(qstrTime, "yyyy-MM-dd hh:mm:ss");
				QString capture_time = QString::number(dateTime.date().year()) + "/" + QString::number(dateTime.date().month()) + \
					"/" + QString::number(dateTime.date().day()) + " " + dateTime.toString("hh:mm:ss");
				info.record_time = capture_time;
				double size_in_mb = item.getProperty(VideoItem::PropType::VideoSize).toLongLong()*1.0 / 1024 / 1024;
				info.video_size = QString::number(size_in_mb, 'f', 3);
			}
		}
	});

	connect(ui->tbv_videolist, &CSVideoTableView::tableviewMoveEvent, this, [=](const QModelIndex& index) {
		if (index.isValid()) {
			m_old_model_index = index;
			if (m_video_items.size() > index.row()) {
				auto item = m_video_items[index.row()].second;
				VideoListThumbnail::ThumbnailInfo info;
				QString device_desc;
				auto device_ptr = m_device_ptr.lock();
				if (device_ptr)
				{
					device_desc = device_ptr->getDescription();
					info.thumbnail_mat = device_ptr->getVideoClipScale(item.getId(),&item);
				}
				else
				{
					device_desc = VideoUtils::parseDeviceIp(item.getId());;
				}
				info.camera_name = device_desc;
				QString qstrTime = item.getTimeStamp();
				QDateTime dateTime = QDateTime::fromString(qstrTime, "yyyy-MM-dd hh:mm:ss");
				QString capture_time = QString::number(dateTime.date().year()) + "/" + QString::number(dateTime.date().month()) + \
					"/" + QString::number(dateTime.date().day()) + " " + dateTime.toString("hh:mm:ss");
				info.record_time = capture_time;
				double size_in_mb = item.getProperty(VideoItem::PropType::VideoSize).toLongLong()*1.0 / 1024 / 1024;
				info.video_size = QString::number(size_in_mb, 'f', 3);

				m_thumbnail_dlg.setThumbnailInfo(QCursor::pos(), info);
				if (m_thumbnail_dlg.isHidden()) {
					m_thumbnail_dlg.show();
				}
			}
		}
		else {
			m_thumbnail_dlg.hide();
			m_old_model_index = QModelIndex{};
		}
	});

	connect(ui->tbv_videolist, &CSVideoTableView::tableviewLeaveEvent, this, [=]() {
		m_thumbnail_dlg.hide();
	});

	// 插件暂未做跳帧
	ui->doubleSpinBox_jump->setEnabled(true);
	ui->doubleSpinBox_jump->setDecimals(0);

	// 跳帧
	m_listPlaySpeed.clear();
	m_listPlaySpeed << 1 << 2 << 3 << 4 << 5 << 10 << 15 << 20 << 25 << 30 << 60 << 80 << 100;
	for (auto value : m_listPlaySpeed)
	{
		//QString strValue = tr("%1%2").arg(QString::number(value)).arg("fps");
		QString strValue = tr("%1").arg(QString::number(value));
		ui->comboBox_display_rate->addItem(strValue);
	}
	int nIndex = m_listPlaySpeed.indexOf(25);
	if (nIndex > -1 && ui->comboBox_display_rate->count() > nIndex)
	{
		ui->comboBox_display_rate->blockSignals(true);
		ui->comboBox_display_rate->setCurrentIndex(nIndex);
		ui->comboBox_display_rate->blockSignals(false);
	}

	QRegExp regx("(^[0-9]+\.?[0-9]?)|([0-9]?)$");
	QRegExpValidator* validator = new QRegExpValidator(regx);
	ui->doubleSpinBox_jump->findChild<QLineEdit*>()->setValidator(validator);

	// 去除问号按钮
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	//初始化进度条状态
	int min = 1, max = 0;
	max = m_video_items[mCrtIdx].second.getEndFrameIndex() - m_video_items[mCrtIdx].second.getBeginFrameIndex() + 1;

	ui->progressBar->setRange(min, max);
	ui->progressBar->setValue(min);


	//初始化保存位置信息
	//初始化本地工作目录
	QString work_dir = SystemSettingsManager::instance().getWorkingDirectory();//默认为工作目录
	if (!m_video_items[mCrtIdx].second.getExportPath().isEmpty())
	{
		work_dir = m_video_items[mCrtIdx].second.getExportPath();//有导出目录则使用导出目录
	}
	ui->lineEdit_save_path->setText(work_dir);
	ui->lineEdit_save_path->setToolTip(work_dir);

	InitVideoListTableHeader();
	InitVideoListTableInfo();
	InitWidgetInfo();

	if (m_video_items.size() == 0) return;
	if (mCrtIdx >= m_video_items.size()) mCrtIdx = 0;

	ui->label_remain_space->setText(tr("Disk Space Remaining:") + " " + QString::number(getDiskFreeSizeGB(work_dir), 'f', 1) + QString("GB"));

	//视频名称限制
	//ui->lineEdit_video_name->setText(m_cur_edit_video_item->getName());
	ui->lineEdit_video_name->setMaxLength(m_max_path_name);
	m_video_name_validator = new QRegExpValidator(QRegExp("^[^\:\*\?\"\<\>\|(\\\\)]+$"));//文件名不可包含\/:*?"<>|
																						 //ui->lineEdit_video_name->setValidator(m_video_name_validator);
	ui->lineEdit_video_name->setStyleSheet("QLineEdit{border:1px solid black}");

	//ui->spinBox_display_rate->setRange(constPlayRateMin, constPlayRateMax);

	QIntValidator* IntValidator = new QIntValidator;
	IntValidator->setRange(1, 1000);
	//安装过滤事件
	ui->comboBox_add_file_name->installEventFilter(this);
	// 透明度滑动条编辑框
	//ui->lineEdit_jump_frame->setValidator(IntValidator);

	//连接进度信号和槽
	if (mCrtIdx == 0)
	{
		connect(this, &CSDlgDeviceVideoExport::signalProgressChanged, this, &CSDlgDeviceVideoExport::slotProgressChanged, Qt::QueuedConnection);;
	}

	QList<QPair<int32_t, VideoItem>> items;
	for (auto item : m_video_items)
	{
		items.append(qMakePair(item.first, item.second));
	}
	m_videoOpts = loadVideoInfo(items);
	//读取默认配置
	ReadSettings();
	//刷新控件显示状态
	updateWidgetsState(m_videoOpts[mCrtIdx]);

	if (m_enable_auto_export)//自动导出模式
	{
		QTimer::singleShot(200, this, &CSDlgDeviceVideoExport::on_pushButton_start_clicked);
	}
}

void CSDlgDeviceVideoExport::InitVideoListTableHeader()
{
	if (ui->tbv_videolist)
	{
		m_pModeVideoList = new VideoListTableModel(this);
		m_pModeVideoList->SetListColNum(10);
		QStringList labels;
		labels << tr("Device Name") << tr("Record Time") << tr("Frame rate") << tr("Resolution") << tr("Total Frames")
			<< tr("%1/%2").arg(tr("Total Times")).arg(tr("ms")) << tr("%1/%2").arg(tr("Export range")).arg(tr("Frame")) 
			<< tr("%1/%2").arg(tr("Export range")).arg(tr("ms")) << tr("%1/%2").arg(tr("Export total")).arg(tr("Frame"))
			<< tr("%1/%2").arg(tr("Export total")).arg(tr("ms"));
		m_pModeVideoList->SetHeadText(labels);

		ui->tbv_videolist->horizontalHeader()->setVisible(true);
		ui->tbv_videolist->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
		ui->tbv_videolist->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
		ui->tbv_videolist->horizontalHeader()->setStyleSheet("border: 0px solid ; border-bottom: 1px solid #d8d8d8;");
		ui->tbv_videolist->verticalHeader()->setDefaultSectionSize(25);
		ui->tbv_videolist->setSelectionMode(QAbstractItemView::SingleSelection);
		ui->tbv_videolist->setSelectionBehavior(QAbstractItemView::SelectRows);
		ui->tbv_videolist->setModel(m_pModeVideoList);
		connect(ui->tbv_videolist->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &CSDlgDeviceVideoExport::slotTabViewCurrentSelectChanged);
		connect(m_pModeVideoList, &VideoListTableModel::SignalDataChanged, ui->tbv_videolist, &QTableView::reset);
		connect(m_pModeVideoList, &VideoListTableModel::SignalExportRangeDataChanged, this, &CSDlgDeviceVideoExport::slotExportRangeDataChanged);
	}
}

void CSDlgDeviceVideoExport::InitVideoListTableInfo()
{
	//相机名
	QString device_desc;
	auto device_ptr = m_device_ptr.lock();
	if (device_ptr)
	{
		device_desc = device_ptr->getDescription();
	}
	else
	{
		device_desc = VideoUtils::parseDeviceIp(m_video_items[mCrtIdx].second.getId());;
	}
	for (int i = 0; i < m_video_items.size(); i++)
	{
		VideoInfoRecordList record;
		record.nIndex = i;
		record.videoInfo.strName = device_desc;
		QString qstrTime = m_video_items[i].second.getTimeStamp();
		QDateTime dateTime = QDateTime::fromString(qstrTime, "yyyy-MM-dd hh:mm:ss");
		QString capture_time = QString::number(dateTime.date().year()) + "/" + QString::number(dateTime.date().month()) + \
			"/" + QString::number(dateTime.date().day()) + " " + dateTime.toString("hh:mm:ss");
		record.videoInfo.strRecordTime = capture_time;
		record.videoInfo.uiFrames = m_video_items[i].second.getFPS();
		record.videoInfo.strResolution = m_video_items[i].second.getResolution();
		record.videoInfo.uiTotalFrame = m_video_items[i].second.getVideoFrameCount();
		record.videoInfo.uiStartFrame = m_video_items[i].second.getBeginFrameIndex() + 1;
		record.videoInfo.uiEndFrame = m_video_items[i].second.getEndFrameIndex() + 1;

		m_recordList.append(record);
		QString strVideoName = conststrIP;
		strVideoName += "_";
		strVideoName += conststrRecordtime;
		m_video_items[i].second.setRuleName(strVideoName);
	}
	m_pModeVideoList->updateData(m_recordList);
	//ui->tbv_videolist->selectRow(0);
	SetWatermarkerCtrEnable(false);
	InitWatermarkerCtr();
}

void CSDlgDeviceVideoExport::InitWidgetInfo()
{
	if (ui->comboBox_save_unit)
	{
		ui->comboBox_save_unit->setEnabled(true);
	}

	if (ui->comboBox_save_unit)
	{
		QStringList vctUnit;
		vctUnit << tr("frames") << "ms" << "s" ;

		ui->comboBox_save_unit->addItems(vctUnit);
	}
	if (ui->comboBox_add_file_name)
	{
		ui->comboBox_add_file_name->addItem(tr("Add file name"));
		ui->comboBox_add_file_name->addItem(tr("Video Name"), conststrVideoName);
		ui->comboBox_add_file_name->addItem(tr("IP"), conststrIP);
		ui->comboBox_add_file_name->addItem(tr("Record date"), conststrRecordDate);
		ui->comboBox_add_file_name->addItem(tr("Record time"), conststrRecordtime);
		ui->comboBox_add_file_name->addItem(tr("Frame rate"), conststrFramerate);
		ui->comboBox_add_file_name->addItem(tr("Resolution ratio"), conststrResolutionratio);
		ui->comboBox_add_file_name->addItem(tr("Time stamp"), conststrTimestamp);
		ui->comboBox_add_file_name->addItem(tr("Frame number"), conststrFramenumber);
		ui->comboBox_add_file_name->addItem(tr("Frame subnumber"), conststrFramesubnumber);
		ui->comboBox_add_file_name->addItem(tr("Default name"), conststrDefaultName);
		//ui->comboBox_add_file_name->addItem(tr("Format"), conststrFormat);
		QListView *view = qobject_cast<QListView *>(ui->comboBox_add_file_name->view());
		if (view)
		{
			view->setRowHidden(0, true);
		}
	}

	// 透明度滑动条
	ui->slider_transparent->setMinimum(constPercentValueMin);    //最小值
	ui->slider_transparent->setMaximum(constPercentValueMax);    //最大值
	ui->slider_transparent->setSingleStep(1);    //步长

	// 透明度滑动条编辑框
	ui->spinBox_transparent->setRange(constPercentValueMin, constPercentValueMax);
	ui->spinBox_transparent->setValue(0);

	// 旋转角度滑动条
	ui->slider_rotate->setMinimum(constAngleValueMin);    //最小值
	ui->slider_rotate->setMaximum(constAngleValueMax);    //最大值
	ui->slider_rotate->setSingleStep(1);    //步长

	// 旋转角度滑动条编辑框
	ui->spinBox_rotate->setRange(constAngleValueMin, constAngleValueMax);
	ui->spinBox_rotate->setValue(0);

	// 水印度滑动条
	ui->slider_watermark->setMinimum(constZoomValueMin);    //最小值
	ui->slider_watermark->setMaximum(constZoomValueMax);    //最大值
	ui->slider_watermark->setSingleStep(1);    //步长

	 // 水印滑动条编辑框
	ui->spinBox_watermark->setRange(constZoomValueMin, constZoomValueMax);
	ui->spinBox_watermark->setValue(100);

	connect(ui->slider_transparent, &QSlider::valueChanged, this, &CSDlgDeviceVideoExport::slotSetTransparentLineEditValue);
	connect(ui->slider_rotate, &QSlider::valueChanged, this, &CSDlgDeviceVideoExport::slotSetRotateLineEditValue);
	connect(ui->slider_watermark, &QSlider::valueChanged, this, &CSDlgDeviceVideoExport::slotSetWatermarkLineEditValue);

	InitWatermarkInfo();
}

void CSDlgDeviceVideoExport::InitWatermarkInfo()
{
	m_watermark_table_model_ptr = new QStandardItemModel();
	ui->tableViewWatermark->setModel(m_watermark_table_model_ptr);
	CheckBoxDelegate *watermarkDelegate = new CheckBoxDelegate(this);
	ui->tableViewWatermark->setItemDelegate(watermarkDelegate);
	
	m_watermark_table_model_ptr->setColumnCount(3);
	ui->tableViewWatermark->horizontalHeader()->setVisible(false);
	ui->tableViewWatermark->verticalHeader()->setVisible(false);
	ui->tableViewWatermark->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui->tableViewWatermark->setHorizontalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
	ui->tableViewWatermark->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->tableViewWatermark->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->tableViewWatermark->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableViewWatermark->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	ui->tableViewWatermark->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
	ui->tableViewWatermark->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
	ui->tableViewWatermark->horizontalHeader()->resizeSection(0, 20);
	ui->tableViewWatermark->horizontalHeader()->resizeSection(2, 20);
	
	connect(ui->tableViewWatermark, &QTableView::clicked, this, &CSDlgDeviceVideoExport::slotTabViewWatermarkClicked);
	connect(watermarkDelegate, &CheckBoxDelegate::signalTableCheck, this, &CSDlgDeviceVideoExport::slotWatermarkItemCheck);
	connect(this, &CSDlgDeviceVideoExport::SignalChangeCheckBoxState, watermarkDelegate, &CheckBoxDelegate::SlotChangeCheckBoxState);
}

double CSDlgDeviceVideoExport::getRange(VideoSaveOpt &opt)
{
	if (opt.skipUnit == eFrame) return opt.uiEnd - opt.uiStart + 1;
	if (opt.skipUnit == eMillonSec) {
		auto framenum = opt.uiEnd - opt.uiStart + 1;
		return (double)framenum / opt.fps * 1000;
	}
	if (opt.skipUnit == eSecond) {
		auto framenum = opt.uiEnd - opt.uiStart + 1;
		return (double)framenum / opt.fps;
	}
	return 0.0;
}

bool CSDlgDeviceVideoExport::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == ui->comboBox_add_file_name)
	{
		if (event->type() == QEvent::HoverEnter)
		{
			m_nVideoNameEditPos = -1;
			if (ui->lineEdit_video_name->hasFocus())
			{
				int nPos = ui->lineEdit_video_name->cursorPosition();
				m_nVideoNameEditPos = nPos;
			}
		}
	}
	return QDialog::eventFilter(obj, event);
}
