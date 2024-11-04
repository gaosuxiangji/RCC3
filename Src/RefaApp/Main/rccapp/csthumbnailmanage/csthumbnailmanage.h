#ifndef CSTHUMBNAILMANAGE_H
#define CSTHUMBNAILMANAGE_H

#include <QWidget>
#include "../csthumbnailcontrol/csthumbnailcontrol.h"
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QEvent>
#include <QTimer>

#define TRUN_ON_THUMBNAIL_ANIMATION_MODE 1    //开启缩略图动画模式 1-开启 0-关闭
#define THUMBNAIL_NUM 15
#define THUMBNAIL_MAX_NUM 50
#define TIME_INTERVAL 500    //定时器间隔
#define TRUN_ON_FLOAT_THUMBNAIL 0				//开启缩略图动画模式 1-开启 0-关闭

namespace Ui {
class CSThumbnailManage;
}

class CSThumbnailManage : public QWidget
{
    Q_OBJECT

public:
    explicit CSThumbnailManage(QWidget *parent = 0);
    ~CSThumbnailManage();
public slots:
	/**************************
	* @brief: 缩略图更新槽函数
	* @param:thumbnail_index 缩略图索引，有效范围：[0,thumbnail_count-1]
	* @param:thumbnail_image 缩略图
	* @return:
	* @author:mpp
	* @date:2022/06/01
	***************************/
	void SlotThumbnailUpdated(int thumbnail_index, const RccFrameInfo & thumbnail_image);

	/// \brief 高亮缩略图
	/// \param [in] thumbnail_index 缩略图索引
	void SlotHighlightThumbnail(int thumbnail_index);

	/**************************
	* @brief: 左按钮鼠标移入槽函数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/07/06
	***************************/
	void SlotMouseInLeftBtn();

	/**************************
	* @brief: 左按钮鼠标移出槽函数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/07/06
	***************************/
	void SlotMouseOutLeftBtn();

	/**************************
	* @brief: 右按钮鼠标移入槽函数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/07/06
	***************************/
	void SlotMouseInRightBtn();

	/**************************
	* @brief: 右按钮鼠标移出槽函数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/07/06
	***************************/
	void SlotMouseOutRightBtn();
public:
	/**************************
	* @brief: 根据鼠标在进度条上移动位置所在的帧更新缩略图区域
	* @param:frame_no 帧号
	* @return:
	* @author:mpp
	* @date:2022/06/22
	***************************/
	void updateSelectedThumbnail(int frame_no);

	/**************************
	* @brief: 设置图片左右移动
	* @param: bMove  true-图片左移 false-图片右移
	* @return:
	* @author:mpp
	* @date:2022/06/30
	***************************/
	void SetImgMoveDirection(const bool bMove);
private:
	/**************************
	* @brief: 初始化
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/01
	***************************/
	void Init();

	/**************************
	* @brief: 初始化UI
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/01
	***************************/
	void InitUI();

	/// \brief 初始化缩略图控件
	void InitThumbnailCtrls();

	/**************************
	* @brief:  鼠标按下事件响应
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	/*virtual */void mousePressEvent(QMouseEvent *event);

	/**************************
	* @brief: 鼠标释放事件响应
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	virtual void mouseReleaseEvent(QMouseEvent *event);

	/**************************
	* @brief: 鼠标移动事件响应
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/01
	***************************/
	virtual void mouseMoveEvent(QMouseEvent *event);

	/**************************
	* @brief: 计算控件索引
	* @param:xPos 鼠标x点的相对坐标 
	* @return:
	* @author:mpp
	* @date:2022/06/01
	***************************/
	void CalThumbnailIndex(const int xPos);

	/**************************
	* @brief: 重置成员变量值
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/02
	***************************/
	void ResetVariablesValue();
	void leaveEvent(QEvent *event);

	/************************
	* @brief: 重载resizeEvent
	* @param e:QResizeEvent对象指针
	* @author: mpp
	*************************/
	void resizeEvent(QResizeEvent *e);

	/**************************
	* @brief: 计算鼠标所在缩略图的索引
	* @param:鼠标坐标
	* @return:索引值
	* @author:mpp
	* @date:2022/06/22
	***************************/
	int ReturnThumbnailIndex(const QPointF& posPointf) const;

	/**************************
	* @brief: 获取缩略图起始帧号
	* @param: iStart 起始帧号
	* @param: iEnd 结束帧号
	* @return:
	* @author:mpp
	* @date:2022/06/30
	***************************/
	void GetThumbnailRange(uint64_t& iStart, uint64_t& iEnd);
signals:
	/**************************
	* @brief: 图像显示到大屏信号
	* @param:index 帧索引
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	void SignalShowImgInBigScreen(const int index);

	/**************************
	* @brief: 设置关键帧信号
	* @param:frameNo 关键帧编号
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	void SignalKeyFrame(const int64_t frameNo);

	/**************************
	* @brief: 设置起始帧信号
	* @param:frameNo 起始帧编号
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	void SignalBeginFrame(const int64_t frameNo);

	/**************************
	* @brief: 设置结束帧信号
	* @param:frameNo 结束帧编号
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	void SignalEndFrame(const int64_t frameNo);

	/**************************
	* @brief: 更新“起始-结束”帧范围信号
	* @param:iStart-起始帧号 iEnd-结束帧号
	* @return:
	* @author:mpp
	* @date:2022/06/02
	***************************/
	void SignalUapdateFrameRange(const qint64 iStart, const qint64 iEnd);

	/**************************
	* @brief: 移出缩略图区域信号
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/20
	***************************/
	void SignalMoveOutThumbnailArea();

	/**************************
	* @brief: 单张缩略图点击信号
	* @param:frameNo 帧号
	* @return:
	* @author:mpp
	* @date:2022/06/28
	***************************/
	void SignalSingleThumbnailClicked(const uint64_t& frameNo);

	/**************************
	* @brief: 左右按钮是否启用信号
	* @param:type 1-左按钮  2-右按钮
	* @param:enable true-启用 false-禁用
	* @return:
	* @author:mpp
	* @date:2022/06/30
	***************************/
	void SignalFrameStepStatus(int type, bool enable);

	/**************************
	* @brief: 缩略图区间，帧号范围
	* @param:iStart 起始帧号
	* @param:iEnd 结束帧号
	* @return:
	* @author:mpp
	* @date:2022/06/30
	***************************/
	void SignalThumbnailRange(const uint64_t& iStart, const uint64_t& iEnd);
	private slots:
	void SlotLeftBtnTimer();
	void SlotRightBtnTimer();

	void SlotUpdateShowHotWidget(int nIndex, bool bShow);
	public slots:
	void SlotVideoRangeChanged(int64_t start_frame_index, int64_t end_frame_index);
private:
	/// \brief 获取缩略图控件
	/// \param [in] index 索引
	/// \return 缩略图控件指针
	CSThumbnailControl* getThumbnailCtrl(int index) const;

	/// \brief 设置缩略图控件
	/// \param [in] index 索引
	/// \param [in] ctrl_ptr 控件指针
	void setThumbnailCtrl(int index, CSThumbnailControl* ctrl_ptr);

	/// \brief 获取缩略图控件数量
	int getThumbnailCtrlCount() const;

	/// \brief 设置缩略图高亮状态
	/// \param [in] index 索引
	/// \param [in] highlighted 是否高亮
	void setThumbnailHighlighted(int index, bool highlighted);

private:
	QVector<CSThumbnailControl*> thumbnail_ctrls_;
	QHBoxLayout* m_hThumbnailLayout;
	int m_indexNewest{ -1 };
	int m_pressPos{ -1 };
	int m_iDragIndexStart{ -1 };
	int m_iDragIndexEnd{ -1 };
	bool m_bLeftBtnPressed{ false };
	int m_indexMax{ 0 };    //鼠标移动的最大值
	int thumbnail_ctrl_count_{ 0 };


	uint64_t start_frame_no_{ 0 };
	uint64_t end_frame_no_{ 0 };
	int thumbnail_interval_{ 20 };
	uint64_t thumbnail_end_frame_no_{ 0 };

	int m_iNewestFrameNo{ 0 };     //最新的帧号

	QTimer* m_leftBtnTimer;    //左按钮定时器
	QTimer* m_rightBtnTimer;    //右按钮定时器

	CSThumbnailControl * m_pThumbnailCtrl{nullptr};
	int m_nThumbnailCtrl{ -1 };
private:
    Ui::CSThumbnailManage *ui;

	int highlight_thumbnail_index_{ -1 };
};

#endif // CSTHUMBNAILMANAGE_H
