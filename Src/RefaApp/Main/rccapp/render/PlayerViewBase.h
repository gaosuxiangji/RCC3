#pragma once
#include <QWidget>
#include "graphicsview/bqgraphicsscene.h"
#include "graphicsview/bqgraphicsview.h"
#include "graphicsview/bgmeasuremanage.h"
#include "graphicsview/bgrectitem.h"
#include "graphicsview/csmeasuremanage.h"
#include <Device/csframeinfo.h>
#include "graphicsview/bgtextitem.h"
#include "Device/device.h"
#include "PointMeasure/CMeasureLineManage.h"
#include <QMenu>
#include <QLabel>
#include <QMap>
#include <QToolButton>

#define EXPORT_PREVIEW_PLAYER 20
#define INDEX_MANUALWHITEBALANCE_PLAYER 21

class CSViewManager;

namespace cv {
	class Mat;
}

struct RoiInfo {
	Device::RoiTypes roi_type{ Device::kUnknownRoi };
	QRect roi_rect{};
	QColor roi_color{};
};

class CPlayerViewBase : public QWidget
{
	Q_OBJECT
		
public:
	enum ViewShowLabelType
	{
		ViewIpLabel,    //显示ip信息label
		ViewConnectStatusLabel,    //显示连接状态信息label
		ViewTimestampLabel,    //显示时间戳信息label
		ViewFrameLabel,    //显示已导出视频的帧数
		ViewLabel_Max_COUNT,    //数量
	};

	CPlayerViewBase(int index, uint8_t frame_head_type, QWidget *parent = Q_NULLPTR);
	~CPlayerViewBase(){}
public:
	int getId() const { return m_win_index; }
	virtual QString GetDeviceName() const { return m_strDeiveName; };
	virtual void SetDeviceName(const QString& devicename) {
		m_strDeiveName = devicename;
		//initMeasureLineInfo();
	};
	static void cvMat2QImage(const cv::Mat&, QImage&);
	static void QImage2CvMat(QImage& src, cv::Mat& dst, bool bCopy = false);
	void hide() { QWidget::hide(); }
	void show() { QWidget::show(); }
	void ZoomIn();
	void ZoomOut();
	void FitView(bool bg = false);
	void FullView();
	void SetRotateType(ROTATE_TYPE type, bool bclockwise = true);
	void SetViewShowLabelInfo(ViewShowLabelType type, QVariant info);
	void SetExportDisplayBtnStatus(bool bPause);
	void SetTargetScoringInfo(const bool bShow, const QPointF & fallPoint, const float & dis);
	void SetManualSelectCloseBtnVisible(const bool bShow);
	void updateItemLayer(const Device::RoiTypes& type);
	void turn2Display(bool status);
	void resizeScene();
	void resetParam4DeviceRoiForbid();
	void setDisplayStatus(bool acquiring, const QPointF& pt);

	void setRoiConstraintCondition(int nConstraintCondition);
public slots:
	void SlotUpdateImage(RccFrameInfo img);
	/************************
	* @brief: 修改线段类型
	* @param QString: strIP
	* @param TMeasureLineType: Type
	*************************/
	void SlotMeasureT2VUpdateLineType(const QString &ip, const CMeasureLineManage::TMeasureLineType Type);
	/************************
	* @brief: 修改绘制模式
	* @param QString: strIP
	* @param TMeasureModeType: Type
	*************************/
	void SlotMeasureT2VUpdatModeType(const QString &ip, const CMeasureLineManage::TMeasureModeType Type);

	/************************
	* @brief: 添加所有测量线信息
	* @param QString: strIP
	* @param TMeasureLineInfo: info
	*************************/
	void slotAddMeasureLine(const QString strIP, const CMeasureLineManage::TMeasureLineInfo info);

	/************************
	* @brief: 更新单个测量线信息
	* @param QString: strIP
	* @param TMeasureLineInfo: info
	*************************/
	void slotUpdateMeasureLine(const QString strIP, const CMeasureLineManage::TMeasureLineInfo info);

	/************************
	* @brief: 更新所有测量线信息
	* @param QString: strIP
	* @param QList<TMeasureLineInfo>: info
	*************************/
	void slotUpdateMeasureLines(const QString strIP, const QList<CMeasureLineManage::TMeasureLineInfo> info);

	/************************
	* @brief: 删除部分测量线信息
	* @param QString: strIP
	* @param QList<TMeasureLineInfo>: info
	*************************/
	void slotDeleteMeasureLines(const QString strIP, const QList<int> info);

	/************************
	* @brief: 清除测量线信息
	* @param QString: strIP
	*************************/
	void slotClearMeasureLines(const QString strIP);

	/************************
	* @brief: 更新测量线信息
	* @param QString: strIP
	*************************/
	void slotMeasureModeType(const QString strIP, const CMeasureLineManage::TMeasureModeType info);
	void slotMeasurePointOffset(const QString strIP, const QPoint point);
	/**
	*@brief	响应播放窗口布局响应
	*@param
	*@return
	**/
	void SlotDisplayWidgetAdjust(int type);

	/************************
	* @brief: 设置标定信息
	* @param QString: strIP
	*************************/
	void slotGotoMeasureCal(const QString strIP, double dbValue, int nType);
	void slotMeasureCalMode(const QString strIP, const CMeasureLineManage::TMeasureModeType info);
private slots:
	void updateRoiInfoSlot(const Device::RoiTypes& type);
	void imageRectSlot(const QRect& rect);
	void sendMousePtInSceneSlot(const QPoint& point);
	void updateMeasureItemInfoSlot(const PointInfo& item);
	void updatemultiMeasureItemInfoSlot(const MultiPointInfo& item);

	void slotAddItemForPoint(QList<QPoint> vctPoint);
	void slotMeasureItemModified(const CMeasureLineManage::TMeasureLineInfo& ptInfo);
	void slotItemPointsMove(QList<QPoint> vctPoint);
	void slotCommonSignal(const QString strIP, const CMeasureLineManage::TMeasureCommonSignalType commandType, QVariant info);
	void slotMeasureItemClicked(const int nIndex);
signals:
	void SignalClicked(int index);
	void SignalDoubleClicked(int index);
	void updateRoiInfoSignal(const Device::RoiTypes& type, const QRect& rect);
	void SignalCustomCrossLineCenterMovePoint(const QPoint& centerpoint);
	void SignalUpdateCustomCrossLineCenterPoint(const QPointF& centerpoint);
	void SignalMeasureV2TAddLine(const QString &ip, const CMeasureLineManage::TMeasureLineInfo lineInfo);
	void SignalMeasureV2TUpdateLine(const QString &ip, const CMeasureLineManage::TMeasureLineInfo lineInfo);
	void sigAboutToClose(int view_id);
	void sigAboutToCloseAll();
	void SignalKeyFrame(const int64_t frameNo);
	void SignalBeginFrame(const int64_t frameNo);
	void SignalEndFrame(const int64_t frameNo);
	void SignalMousePressPointF(const QPointF& point);
	void SignalCloseManualSelect();    //to do:报靶按钮关闭时，发送此信号
	void sigCurrentCursorInVideo(const QPoint& pt);
	void sigZoomCoefficientChanged(float coef);
	void SignalUpdateGrayOrRGBValue(const QPoint& pt, const cv::Mat& matImage);
	void measureLineItemSignal(uint32_t index, const PointInfo& item);
	void multiMeasureLineItemSignal(uint32_t index, const MultiPointInfo& item);
	void SignalExitDrawRoi();
public:
	bool GetTrackRoiEnabled();
	void SetAntiAliasing(bool bAntiAliased);
	bool GetAntiAliasingStatus();
	void EnableDrawBorderLine(bool bdraw);
	void SetOverExposureStatus(bool bOver);
	void setBackgroundImage(const QImage& image);
	float GetZoomCoeff() const;
	bool Snapshot();
	void SetCustomCrossLineCenterPoint(const QPointF& centerpoint);
	virtual void EnableDrawCrossLine(bool bdraw);
	void EnableDrawCustomCrossLine(bool bdraw);
	void drawRoiRect(const RoiInfo& roi_info, bool mid = false);
	void setRoiVisible(Device::RoiTypes type, bool bVisible);
	void setFeaturePointVisible(Device::RoiTypes type, bool bVisible);
	/************************
	* @brief: 设置库测量模式类型
	* @param type: 类型
	*************************/
	void setMeasureModeType(CMeasureLineManage::TMeasureModeType type);
	/************************
	* @brief: 设置记录测量模式类型
	* @param type: 类型
	*************************/
	void updateMeasureModeType(CMeasureLineManage::TMeasureModeType type);
	void showBackgroundImage(bool is_white_balance = false);
private:
	virtual void resizeEvent(QResizeEvent *event) override;
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent* e) override;
	virtual void contextMenuEvent(QContextMenuEvent *event) override;
	void wheelEvent(QWheelEvent *event) override;
	void paintEvent(QPaintEvent *event);
	void leaveEvent(QEvent *event)override;
	bool eventFilter(QObject *watched, QEvent *event);
private:
	void bindSignalAndSlot();
	void init();
	void reCalRoiRect(QRect& rect);
	void moveRoi2Center(QRect& rect);
	void initMeasureLineInfo();
	void MouseRightClicked();
	void UpdateImgPixelColor(QImage &img, QImage& overExploreImg);

	/************************
	* @brief: 移动label位置
	* @param size: 播放控件大小
	* @author: mpp
	*************************/
	void AdjustViewBaseLabel(const QSize& size);

	/************************
	* @brief: 更新QLabel样式
	* @param type: label类型
	* @param label: 播放控件QLabel
	* @param fontsize: 字体大小
	* @author: mpp
	*************************/
	void UpdateQLabelStyle(const ViewShowLabelType& type, QLabel* label, int fontsize);

	/************************
	* @brief: 移动label位置
	* @param size: 播放控件resize后的大小
	* @author: mpp
	*************************/
	int GetLabelFontSize(const QSize& size);

	/************************
	* @brief: 初始化播放控件的QLable
	* @author: mpp
	*************************/
	void InitViewLabel();
	void updateUI();
	bool isDrawCenterRect(const QPointF& pt, const QRectF& rt);
	bool isDrawCenterHLine(const QPointF& pt, const QRectF& rt);
	bool isDrawCenterVLine(const QPointF& pt, const QRectF& rt);

	/**************************
	* @brief: 初始化“叠加至图像”相关的UI
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/10
	***************************/
	void InitOverlayUI();
#if 0
	void updateLayout();
#endif // 0
	void updateViewSceneRect();

	void initMeasureItem();
	void updateMeaureInfo(const CMeasureLineManage::TMeasureLineInfo& info);

	void clearMeasureItem();
protected:
	virtual void changeEvent(QEvent *event) override;
private:
	friend class CSViewManager;

	BQGraphicsScene* m_graphics_scene{nullptr};
	BQGraphicsView* m_graphics_view{nullptr};

	RectangleGraphicsItem m_device_roi{};
	RectangleGraphicsItem m_intelligent_roi{};
	RectangleGraphicsItem m_auto_exposure_roi{};
	RectangleGraphicsItem m_white_balance_roi{};

	int m_win_index{};
	QString m_strDeiveName{};
	QPointF m_mouse_press_point{ 0,0 };
	bool m_bPauseBtnClicked{ false };    //导出预览  是否点击暂停按钮
	bool m_bAnti{ true };    //反走样
	QRect m_original_roi_rect{};
	bool m_roi_drawing{ false };  //绘制roi使能
	RccFrameInfo m_current_image_backup{};
	QImage m_background_img{};
	bool m_over_exposure{ false };    //过曝提示是否开启
	QImage m_exposure_img{};

	CMeasureLineManage::TMeasureModeType m_tMeasureModeTye{ CMeasureLineManage::MMT_Normal };	 // 测量模式
	CMeasureLineManage::TMeasureLineType m_tMeasureLineTye{ CMeasureLineManage::MLT_MORE_POINT };// 线段类型
	QList<CMeasureLineManage::TMeasureLineInfo> m_vctMeasureLine{};			 // 测量信息
	CMeasureLineManage::TMeasureAddLineInfo m_tAddMeasurePoint{};				// 记录编辑点 有两种结束方式，1：点超出总数，2：右键结束 
	QPoint m_PointOffset{ QPoint(0, 0) };									// 记录偏移点
	CMeasureLineManage::TMeasureModeType m_tMeasureCalMode{ CMeasureLineManage::MMT_Normal };

	QMenu * m_rightMenu;
	QAction* act_close;
	QAction* act_close_all;

	QMap<ViewShowLabelType, QLabel*> m_mapViewLabel;    //播放控件上显示的label<label类型，QLabel*>
	QMap<ViewShowLabelType, QString> m_mapViewText;    //播放控件上显示的文字<label类型，QString>
	QMap<ViewShowLabelType, QColor> m_mapViewColor;    //播放控件上显示的颜色<label类型，QString>

	QLabel* m_overlayImageLabel{};    //叠加图像label
	QLabel* m_fallPosNameLabel{};    //落点坐标名称
	QLabel* m_fallPosValueLabel{};    //落点坐标值
	QLabel* m_distanceNameLabel{};    //落点距离名称
	QLabel* m_distanceValueLabel{};    //落点距离值

	QToolButton* m_targetBtnClose{};    //报靶设置-关闭按钮
	bool m_bManualSelected{ false };    //进入手动选择状态

	const QSize m_viewBaseOriginalSize{ QSize(1176, 898) };    //播放控件原始尺寸大小，用于计算缩放因子
	const int m_locationOffset{ 10 };    //位置偏移量
	bool m_is_show_background_img{ false };
	bool m_is_turn_to_display{ false };
	bool m_is_draw_border{ false };
	bool m_cross_line_visible{ false };
	bool m_center_line_visible{ false };
	uint8_t frame_header_type_ = 1;
	bool m_is_allow_shrink{ true };    //是否允许缩小
	bool m_is_allow_magnify{ true };    //是否允许放大
	//qreal m_min_coefficient{};
	QRectF m_view_rect_in_scene{};
	bool m_is_acquring{ false };
	bool m_is_size_changed{ false };
	int m_split_type{ 1 };
	bool m_measure_btn_clicked{ false };

	CSMeasureItemManage m_measureItemManage{};
};
