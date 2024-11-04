#include "cstargerscoring.h"
#include "Video/VideoUtils/videoutils.h"
//#include "FallPointMeasure/FallPointMeasure.h"
//#include "Playback/CSPlaybackCacher/Util/HscSystemElapsedTimer.h"
#include "Util/HscSystemElapsedTimer.h"
#include "Main/rccapp/render/PlayerViewBase.h"
#include <boost/thread.hpp>
#include <boost/chrono.hpp>

namespace CSCtrl {

	CSTargerScoring::CSTargerScoring(VideoItem video_item, QObject *parent /*= 0*/)
		: QObject(parent), m_video_item(video_item)
	{
		InitData();
	}

	CSTargerScoring::~CSTargerScoring()
	{
	}


	bool CSTargerScoring::AllowsTargetScoring()const
	{
		auto device_ptr = m_device_ptr.lock();
		if (device_ptr && device_ptr->AllowsEnableTargetScoring())
		{
			return true;
		}
		return false;
	}

	void CSTargerScoring::InitData()
	{
		//获取到当前视频对应的设备
		QString device_ip = VideoUtils::parseDeviceIp(m_video_item.getId());
		int vid = VideoUtils::parseVideoSegmentId(m_video_item.getId());
		m_device_ptr = DeviceManager::instance().getDevice(device_ip);
		Q_ASSERT(m_device_ptr);
		//初始化设备导出缓存
		m_export_cacher_ptr = std::make_shared<CSPlaybackCacher>(DeviceManager::instance().getDevice(device_ip), vid, m_video_item);

		//加载关键帧前后的图像到缓存中
		int cache_frame_count = 3;
		int key_frame_index = m_video_item.getKeyFrameIndex();
		int start_frame_index = m_video_item.getBeginFrameIndex();
		int end_frame_index = m_video_item.getEndFrameIndex();
		do
		{
			if (end_frame_index - start_frame_index + 1 <= cache_frame_count * 2)//没有足够的帧数,全部加载
			{
				break;
			}

			if (start_frame_index > key_frame_index - cache_frame_count)
			{
				end_frame_index = start_frame_index + cache_frame_count * 2;
			}
			else
			{
				if (end_frame_index < key_frame_index + cache_frame_count)
				{
					start_frame_index = end_frame_index - cache_frame_count * 2;
				}
				else
				{
					start_frame_index = key_frame_index - cache_frame_count;
					end_frame_index = key_frame_index + cache_frame_count;
				}
			}

		} while (0);

		m_cacher_params.curFrameNo = start_frame_index;
		m_cacher_params.frameInterval = 1;
		m_cacher_params.startFrameNo = start_frame_index;
		m_cacher_params.endFrameNo = end_frame_index;

		if (AllowsTargetScoring())//不支持使用报靶功能时不加载
		{
			//开始加载数据
			//m_export_cacher_ptr->start(m_cacher_params);
            /* rgq
			FallPointMeasure::GetInstance().SelectCameraParam(device_ip.toStdString());
			FallPointMeasure::GetInstance().setKeyFrameIndex(key_frame_index);
			FallPointMeasure::GetInstance().SetImageSize(cv::Size(m_video_item.getRoi().width(), m_video_item.getRoi().height()));
			FallPointMeasure::GetInstance().setBurstPoint(cv::Point2f(-1, -1));
			m_grid_interval = FallPointMeasure::GetInstance().FindBestLegendInterval();
			if (m_grid_interval == 0)
			{
				m_grid_interval = 1;
			}
			FallPointMeasure::GetInstance().SetLegendInterval(m_grid_interval);//设置初始网格间距
			//DetectBurstPoint();
			std::thread(&CSTargerScoring::DetectBurstPoint, this).detach();
            */
		}

	}

	cv::Point2f CSTargerScoring::QPoint2CvPoint(QPointF qpoint)const
	{
		return cv::Point2f(qpoint.x(), qpoint.y());
	}

	QPointF CSTargerScoring::CvPoint2QPoint(cv::Point2f cvpoint)const
	{
		return QPointF(cvpoint.x, cvpoint.y);
	}

	bool CSTargerScoring::DetectBurstPoint()
	{
		burst_point_detecting_ok_ = false;
		//将加载到的关键帧前后图像发送到检测模块中
		HscSystemElapsedTimer elapsedTimer;
		int64_t frameNo = m_cacher_params.startFrameNo;
		int64_t endFrameNo = m_cacher_params.endFrameNo;
		RccFrameInfo cachedFrameInfo;
		elapsedTimer.restart();
		while (frameNo <= endFrameNo)
		{
			if (m_export_cacher_ptr->getFrame(frameNo, cachedFrameInfo,false))
			{
				RccFrameInfo frameInfo;
				frameInfo.image = cachedFrameInfo.image;
				frameInfo.playback_info.frame_no = cachedFrameInfo.playback_info.frame_no;
				burst_point_detecting_frames_.push_back(frameInfo);

				elapsedTimer.restart();

				frameNo++;
			}
			else
			{
				if (elapsedTimer.hasExpired(5000))
				{
					break;
				}
			}
			boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
		}
		std::vector<cv::Mat> matImages;
		for (auto & frameInfo : burst_point_detecting_frames_)
		{
			cv::Mat mat;
			CPlayerViewBase::QImage2CvMat(frameInfo.image, mat);
			matImages.push_back(mat);
		}

		if (burst_point_detecting_frames_.size() == 0)
		{
			burst_point_detecting_ok_ = false;
			emit SignalTargetScoringInfoChanged();
			return false;
		}

        bool res = false;//FallPointMeasure::GetInstance().DetectBurstPoint(matImages, burst_point_detecting_frames_.begin()->playback_info.frame_no); //rgq
		//通知检测结果变更
		if (res)
		{
			burst_point_detecting_ok_ = true;
			emit SignalTargetScoringInfoChanged();
		}
		return res;
	}


	void CSTargerScoring::SetBurstPoint(QPointF burst_point)
	{
		if (burst_point.x() >= 0 && burst_point.y() >= 0)
		{
            //rgq
            //FallPointMeasure::GetInstance().setBurstPoint(QPoint2CvPoint(burst_point));
            //burst_point_detecting_ok_ = true;
            //SignalTargetScoringInfoChanged();
		}
	}

	QPointF CSTargerScoring::GetBurstPoint()
	{
        //rgq
        //return CvPoint2QPoint( FallPointMeasure::GetInstance().getBurstPoint());
        return QPoint{};
	}

	void CSTargerScoring::SetBurstFrameIndex(int64_t frameNo)
	{
        //rgq
        //FallPointMeasure::GetInstance().setKeyFrameIndex(frameNo);
	}

	int64_t CSTargerScoring::GetBurstFrameIndex()
	{
        //rgq
        //return 	FallPointMeasure::GetInstance().GetBurstPointFrameIndex();
        return 0;
	}

	void CSTargerScoring::DrawGridAndBurstPoint(QImage& src_img, bool draw_grid, int64_t frame_index)
	{
		cv::Mat mat;
		CPlayerViewBase::QImage2CvMat(src_img, mat);
        //rgq
        //FallPointMeasure::GetInstance().DrawLegends(mat, draw_grid, frame_index);
		CPlayerViewBase::cvMat2QImage(mat, src_img);
	}

	void CSTargerScoring::SetCenterPoint(QPointF center_point)
	{
        //rgq
        //FallPointMeasure::GetInstance().setCenterPos(QPoint2CvPoint(center_point));
	}

	QPointF CSTargerScoring::GetCenterPoint()
	{
		cv::Point2f center_point(-1, -1);
        //rgq
        //FallPointMeasure::GetInstance().getCenterPos(center_point);
		return CvPoint2QPoint(center_point);
	}

	void CSTargerScoring::SetGridInterval(int interval)
	{
		int min_interval = 0;
		int max_interval = 0;
		GetGridIntervalRange(min_interval, max_interval);
		if (interval < min_interval)
		{
			interval = min_interval;
		}
		if (interval > max_interval)
		{
			interval = max_interval;
		}
		m_grid_interval = interval;
        //rgq
        //FallPointMeasure::GetInstance().SetLegendInterval(interval);
	}

	int CSTargerScoring::GetGridInterval()
	{
		return m_grid_interval;
	}

	void CSTargerScoring::GetGridIntervalRange(int& min, int& max)
	{
		//目前为固定范围
		min = 1;
		max = 50;
	}

	bool CSTargerScoring::CalculateDistance2CenterPoint(QPointF pt, float& dist)
	{
        //rgq
        //return FallPointMeasure::GetInstance().CalcDistToCenter(QPoint2CvPoint(pt), dist);
        return false;
	}

	CSTargerScoring::TargetScoringResult CSTargerScoring::GetTargetScoringInfo(QPointF &fallPoint, float & dis) const
	{
		if (!AllowsTargetScoring())
		{
			return TSR_CAMERA_PARAM_NOT_IMPORTED;//不支持使用标定功能
		}

        //rgq
        //if (!burst_point_detecting_ok_)
        //{
        //	return TSR_BURST_POINT_NOT_DETECTED;//没有炸点
        //}
        //cv::Point2f cvpoint = QPoint2CvPoint(fallPoint);
        //FallPointMeasure::GetInstance().CalcFallPoint(FallPointMeasure::GetInstance().getBurstPoint(), cvpoint);
        //fallPoint = CvPoint2QPoint(cvpoint);
        //FallPointMeasure::GetInstance().CalcDistToCenter(FallPointMeasure::GetInstance().getBurstPoint(), dis);

		return TSR_OK;
	}

}
