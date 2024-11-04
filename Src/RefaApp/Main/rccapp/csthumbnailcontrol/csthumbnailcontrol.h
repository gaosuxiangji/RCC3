#ifndef CSTHUMBNAILCONTROL_H
#define CSTHUMBNAILCONTROL_H

#include <QWidget>
#include <QMenu>
#include "Playback/csplaybackcontroller.h"
#include <QGraphicsDropShadowEffect>
#define  constFloatControlIndex 500

namespace Ui {
class CSThumbnailControl;
}

class CSThumbnailControl : public QWidget
{
    Q_OBJECT

	enum LoadStatus
	{
		Loading,
		LoadFail,
		None1
	};
public:
    explicit CSThumbnailControl(int index, QWidget *parent = 0);
    ~CSThumbnailControl();
public:
	/**************************
	* @brief: 绘制边框线
	* @param:moveIn-鼠标是否移入控件
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	void DrawBorderLine(bool moveIn);

	/**************************
	* @brief: 更新缩略图
	* @param:thumbnail_index 缩略图索引，有效范围：[0,thumbnail_count-1]
	* @param:thumbnail_image 缩略图
	* @return:
	* @author:mpp
	* @date:2022/06/01
	***************************/
	void UpdateThumbnail(int thumbnail_index, const RccFrameInfo & thumbnail_image);

	/**************************
	* @brief: 获取帧号
	* @param:
	* @return:帧号
	* @author:mpp
	* @date:2022/06/02
	***************************/
	qint64  GetFrameNo() const
	{
		return m_imgFrameNo;
	}

	/**************************
	* @brief: 判断当前控件图像是否为空
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/22
	***************************/
	bool IsCurrentImgEmpty()const
	{
		return (m_imgCurrent.isNull()) ? true : false;
	}

	/// \brief 获取缩略图状态
	int GetThumbnailState() const
	{
		return thumbnail_state_;
	}

	bool IsHeightLight()
	{
		return move_in_;
	}
	QImage GetCurrentImage()
	{
		return m_imgCurrent;
	}

	void SetCurrentImage(QImage image)
	{
		m_imgCurrent = image;
		SetThumbnail(m_imgCurrent);
	}
	void setKeyFrame(bool bKey)
	{
		m_bKeyFrame = bKey;
	}

	void setFrameNo(qint64 imgFrameNo)
	{
		m_imgFrameNo = imgFrameNo;
		SetFrameInfo(m_bKeyFrame, m_imgFrameNo);
	}
	
	bool IsKeyFrame()
	{
		return m_bKeyFrame;
	}
	int getThumbnailIndex()
	{
		return m_iThumbnailIndex;
	}
	void setThumbnailIndex(int nIndex)
	{
		m_iThumbnailIndex = nIndex;
	}
	
	int GetCtrlIndex()
	{
		return m_iIndex;
	}
	void updateUI()
	{
		UadateBackground(true);
	}
signals:
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
	* @brief: 图像显示到大屏信号
	* @param:index 帧索引
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	void SignalShowImgInBigScreen(const int index);

	/**************************
	* @brief: 单张缩略图双击信号
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/29
	***************************/
	void SignalSingleThumbnailClicked(const uint64_t& frameNo);

	void SignalUpdateShowHotWidget(int index, bool bShow);
	void SignalMoveOutThumbnailArea();
private:
	/**************************
	* @brief: 初始化
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	void Init();

	/**************************
	* @brief: 初始化UI
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	void InitUI();

	/**************************
	* @brief: 设置帧信息
	* @param:bKey 是否是关键帧
	* @param:frameNo 帧号
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	void SetFrameInfo(bool bKey, const qint64 frameNo);

	/**************************
	* @brief:
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	void SetThumbnail(const QImage& img);

	/**************************
	* @brief: 设置加载信息
	* @param:status状态 0-加载中 1-加载失败
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	void SetLoadInfo(const LoadStatus status);

	/**************************
	* @brief: 点击鼠标右键
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	virtual void contextMenuEvent(QContextMenuEvent *event) override;

	/**************************
	* @brief: 点击鼠标右键
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	void MouseRightClicked();

	/**************************
	* @brief: 
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	void paintEvent(QPaintEvent *event);

	/**************************
	* @brief: 鼠标移入操作
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	void enterEvent(QEvent *event);

	/**************************
	* @brief: 鼠标移出操作
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	void leaveEvent(QEvent *event);

	/**************************
	* @brief: 刷新背景
	* @param:bSucess 图像是否加载成功
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	void UadateBackground(bool bSucess);

	/**************************
	* @brief: 鼠标双击事件
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/29
	***************************/
	void mouseDoubleClickEvent(QMouseEvent *event);

	void resizeEvent(QResizeEvent *event);
private:
	QMenu* m_rightMenu = nullptr;
	qint64 m_imgFrameNo{ 0 };    //图片的帧号
	QImage m_imgCurrent{};    //当前图片
	int m_iIndex;    //控件索引
	int m_iThumbnailIndex{ -1 };    //缩略图索引
	QGraphicsDropShadowEffect* m_Graph;
	bool m_bKeyFrame{ false };    //是否关键帧
	bool m_bStartFrame{ false };    //是否起始帧
	bool m_bEndFrame{ false };    //是否结束帧
	bool m_bEmptyImage{ false };
	PlaybackInfo::ThumbNailState thumbnail_state_{ PlaybackInfo::TN_VOID };

	bool move_in_{ false };
private:
    Ui::CSThumbnailControl *ui;
};

#endif // CSTHUMBNAILCONTROL_H
