#ifndef CSTARGERSCORING_H
#define CSTARGERSCORING_H
#include <memory>
#include <mutex>
#include <QObject>
#include <stack>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QTimer>
#include <QPointer>
#include <thread>
#include <atomic>
#include "Playback/CSPlaybackCacher/csplaybackcacher.h"
#include "Video/VideoItem/videoitem.h"
#include "Device/device.h"
#include "Device/devicemanager.h"
#include <QPointF>

//报靶数据控制模块,用于报靶界面的数据处理操作
namespace CSCtrl {
	class CSTargerScoring : public QObject
	{
		Q_OBJECT

	public:
			//报靶结果类型
			enum TargetScoringResult
		{
			TSR_OK,//成功报靶
			TSR_CAMERA_PARAM_NOT_IMPORTED,//无标定文件
            //未检测到炸点
		};

		//会加载图像,较为耗时
		explicit CSTargerScoring(VideoItem video_item, QObject *parent = 0);
		~CSTargerScoring();

		/**
		* @brief	根据设备情况和标定参数情况来判断是否支持报靶相关操作
		* @return	true-支持,false-不支持
		*/
		bool AllowsTargetScoring()const;


		//落点相关接口,落点的位置和帧号设置与读取
		/**
		* @brief	检测落点
		* @return	是否成功
		* @note		如果成功会导致落点位置和落点帧号变动,报靶结果变更
		*/
		bool DetectBurstPoint();

		/**
		* @brief	设置落点位置(像素坐标)
		* @param	[in]burst_point 落点位置(图像上的像素坐标)
		* @note		会导致落点位置变动,报靶结果变更
		*/
		void SetBurstPoint(QPointF burst_point);
		QPointF GetBurstPoint();

		/**
		* @brief	设置落点所在帧号(用于根据帧号判断是否需要绘制落点)
		* @param	frameNo 帧号
		* @note		落点检测也会变更落点帧号
		*/
		void SetBurstFrameIndex(int64_t frameNo);
		int64_t GetBurstFrameIndex();

		//网格相关接口
		/**
		* @brief	绘制网格和落点
		* @param	[in][out]src_img 需要绘制的原图像
		* @param	draw_grid 是否要绘制网格线
		* @param	frame_index 当前图像的帧号
		* @note		(frame_index用于判断是否需要绘制落点,只有帧号与落点所在帧号相同时才会绘制落点)
		*/
		void DrawGridAndBurstPoint(QImage& src_img, bool draw_grid, int64_t frame_index);

		/**
		* @brief	设置网格绘制的中心点(目标点,靶点)(图片像素坐标)
		* @param	center_point 中心点坐标
		* @note		会改变绘制的网格线
		*/
		void SetCenterPoint(QPointF center_point);
		QPointF GetCenterPoint();

		/**
		* @brief	设置网格间距(物理单位,米)
		* @param	interval 网格间距(单位米)
		* @note		会改变绘制的网格线
		*/
		void SetGridInterval(int interval);
		int GetGridInterval();
		/**
		* @brief	获取网格间距范围(物理单位,米)
		* @param	[out]min 网格间距最小值(单位米)
		* @param	[out]max 网格间距最大值(单位米)
		* @note
		*/
		void GetGridIntervalRange(int& min, int& max);

		/**
		* @brief	计算指定像素坐标点与中心点之间在物理世界中的距离
		* @param	[in]pt 需要计算距离的点(像素坐标)
		* @param	[out]dist 与中心点之间的距离(物理坐标,单位米)
		* @return	是否计算成功
		* @note		中心点位置变动后该计算结果也会变更
		*/
		bool CalculateDistance2CenterPoint(QPointF pt, float& dist);

		/**
		* @brief	获取落点位置信息(获取报靶结果)
		* @param	[out]fallPoint 落点位置(物理坐标,单位米)
		* @param	[out]dis 落点距离目标的距离(物理单位,米)
		* @return	TargetScoringResult 报靶结果 : 成功,未检测到炸点,无标定文件
		* @note		结果和输出参数会随着炸点位置,目标位置,标定文件的状态而变更
		*/
		TargetScoringResult GetTargetScoringInfo(QPointF & fallPoint, float & dis) const;

	signals:

		//告知报靶结果有变动,在变更了报靶结果相关的参数之后会触发(炸点位置,目标位置,标定文件等)
		void SignalTargetScoringInfoChanged();

		public slots:

	private:
		void InitData();

		cv::Point2f QPoint2CvPoint(QPointF qpoint) const;
		QPointF CvPoint2QPoint(cv::Point2f cvpoint) const;
	private:

		VideoItem m_video_item;//当前视频项

		QWeakPointer<Device> m_device_ptr;//当前视频项对应的设备项

		std::shared_ptr<CSPlaybackCacher> m_export_cacher_ptr;//视频导出缓存

		PlaybackCacherParams m_cacher_params;//导出缓存相关参数

		std::vector<RccFrameInfo> burst_point_detecting_frames_;
		bool burst_point_detecting_ok_{ false };//检测炸点是否成功

		int m_grid_interval{ 1 };
	};
}
#endif // CSTARGERSCORING_H
