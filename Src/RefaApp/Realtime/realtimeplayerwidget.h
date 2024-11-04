#ifndef REALTIMEPLAYERWIDGET_H
#define REALTIMEPLAYERWIDGET_H

#include <RMAImage.h>
#include <QWidget>
#include <QVariant>
#include <QMap>
#include <memory>
#include <QPair>

/************前置声明区***********/
class CWidgetVideoPlaySingle;
class PlayerControllerInterface;
enum PLAYER_WORK_MODE;
/*******************************/

namespace Ui {
class RealtimePlayerWidget;
}

/**
 * @brief 实时图像播放器类
 */
class RealtimePlayerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RealtimePlayerWidget(QWidget *parent = 0);
    ~RealtimePlayerWidget();

	/**
	*@brief 设置播放器工作模式
	*@param [in] : mode : PLAYER_WORK_MODE，工作模式
	*@note 使用时需注意在设置下一个模式前需先将模式设置为idle
	**/
	void setPlayerWorkStatus(PLAYER_WORK_MODE mode);

	/**
	*@brief 获取当前工作模式
	*@return PLAYER_WORK_MODE : 工作模式
	**/
	PLAYER_WORK_MODE getPlayerWorkStatus() const;

public:
	/**
	*@brief 添加视频
	*@param [in] : player_controller_ptr : std::shared_ptr<PlayerControllerInterface>，playercontroller指针
	video_mark : const QVariant&，视频唯一标识符
	*@note video_mark暂时提供默认值，后续完全调通后不再提供默认值
	**/
    void addVideo(std::shared_ptr<PlayerControllerInterface> player_controller_ptr, const QVariant video_mark = QVariant());

	/**
	*@brief 移除视频
	*@param [in] : player_controller_ptr : std::shared_ptr<PlayerControllerInterface>，playercontroller指针
	**/
    void removeVideo(std::shared_ptr<PlayerControllerInterface> player_controller_ptr);

	/**
	*@brief 设置十字线中心
	*@param [in] : pt : const QPoint &，中心点坐标
	**/
	void setFocusPoint(const QPoint &pt);//设置窗口中心线

	/**
	*@brief 设置roi
	*@param [in] : roi : const QRect &，roi
	**/
	void setRoi(const QRect &roi);//设置ROI

    /**
     * @brief 获取当前播放器IP
     * @return IP
     */
    QString getCurrentPlayerIp() const;

public slots:
	/**
	*@brief 放大
	**/
    void zoomIn();
	
	/**
	*@brief 缩小
	**/
    void zoomOut();
	
	/**
	*@brief 1:1显示
	**/
    void zoomToOriginal();
	
	/**
	*@brief 窗口自适应
	**/
    void zoomToFit();
	
	//工具栏信号响应槽函数
	/**
	*@brief 设置十字线是否显示
	*@param [in] : bvisible : bool，true-显示，false-不显示
	**/
    void setFocusPointVisible(bool bvisible);

	/**
	*@brief 设置是否全屏
	*@param [in] : benabled : bool，true-使能，false-不使能
	**/
    void setFullScreen(bool benabled);//设置是否全屏

	/**
	*@brief 开始选择roi
	**/
    void setRoiSelectionStart();

	/**
	*@brief 设置ROI框选是否使能
	*@param [in] : benabled : bool，true-使能，false-不使能
				   player_id : const QVariant &，播放器id
	**/
	void setRoiSelectionEnabled(bool benabled, const QVariant & player_id);

	/**
	*@brief 快照截屏
	**/
    void snapshot();//快照截屏

	/**
	*@brief 全屏时还原按钮响应
	**/
	void on_toolButtonRestoreFullScreen_clicked();//全屏时还原按钮响应

	/**
	*@brief 设置osd信息是否显示
	*@param [in] : bvisible : bool，true-显示，false-不显示
	**/
	void setOSDVisible(bool bvisible);

signals:
	/**
	*@brief 十字线变化信号
	*@param [in] : ip : const QVariant& ，相机ip
				   pt : const QPoint& ，十字线交点
	**/
    void focusPointChanged(const QString& ip, const QPoint &pt);

	/**
	*@brief 十字线显示状态变化信号
	*@param [in] : ip : const QVariant& ，相机ip
				   bvisible : bool，true-显示，false-不显示
	**/
	void focusPointVisible(const QString &ip, bool bvisible);

	/**
	*@brief roi变化信号
	*@param [in] : ip : const QVariant& ，相机ip
				   roi : const QRect& ，roi区域
	**/
    void roiChanged(const QString& ip, const QRect &roi);

	/**
	*@brief 全屏信号
	*@param [in] : benabled : bool，true-进入全屏，false-退出全屏
	**/
	void fullScreenTriggered(bool benabled);

	/**
	*@brief 选中窗口变化信号
	*@param [in] : ip : const QVariant& ，相机ip
	**/
	void focusPlayerChanged(const QString& ip);

	/**
	*@brief 保存截图信号
	*@param [in] : ip : const QVariant& ，相机ip
				   img : const RMAImage& ，图片
	**/
	void saveSnapshot(const QString& ip, const RMAImage &img);

private:
	/**
	*@brief 界面初始化
	**/
    void initUi();

	/**
	*@brief 初始信号绑定
	**/
    void initBinding();

	/**
	*@brief 添加视频实现
	*@param [in] : *pplayer : CWidgetVideoPlaySingle，播放器指针
				    player_controller_ptr : std::shared_ptr<PlayerControllerInterface>，player_ctrl指针
	**/
	inline void addVideo(CWidgetVideoPlaySingle *pplayer, std::shared_ptr<PlayerControllerInterface> player_ctrl);

	/**
	*@brief 当前选中的播放器
	*@return 当前选中播放器的指针
	**/
    inline CWidgetVideoPlaySingle *getFocusPlayer() const;

	/**
	*@brief 获取播放器的ip
	*@param [in] : *pplayer : CWidgetVideoPlaySingle，播放器指针
	*@return QString : ip
	**/
    inline QString getPlayerIP(CWidgetVideoPlaySingle *p) const;

	/**
	*@brief 获取播放器的ip
	*@param [in] : player_no : int，播放器编号
	*@return QString : ip
	**/
	inline QString getPlayerIP(int player_no) const;

	/**
	*@brief 更新工具栏
	**/
	void updateToolBar();

    /**
     * @brief 更新播放器是否显示
     */
	void updatePlayersVisible();

	/**
	* @brief 获取当前图像
	*/
	RMAImage getCurImage();

	/**
	*@brief 获取是否使能设置roi
	*@return true-使能，false-不使能
	**/
	inline bool isEnabledSelctionRoi() const;

private slots:
	/**
	*@brief 选中播放器变化
	*@param [in] : *pplayer : CWidgetVideoPlaySingle，选中播放器指针
	**/
    void slotSetFocusWidget(CWidgetVideoPlaySingle *player);//设置当前选中播放器

	/**
	*@brief 播放器内拖动roi
	*@param [in] : roi : const QRect &，roi
	**/
    void slotRoiChanged(const QRect &roi);//roi区域发生变化
		
	/**
	*@brief 播放器内拖动十字线
	*@param [in] : pt : const QPoint &，roi
	*@return
	**/
    void slotFocusPointChanged(const QPoint &pt);//十字线中心发生变化

	/**
	*@brief 播放器更新图像响应
	*@param [in] : idx : int64_t，图像帧号
	**/
	void slotUpdateFrame(int64_t idx);

	/**
	*@brief 开启工具栏显示定时器
	**/
	void startToolbarVisibleTimer();

protected:
	/**
	*@brief 鼠标双击事件
	*@param [in] : *event : QMouseEvent，事件指针
	**/
	void mouseDoubleClickEvent(QMouseEvent *event);

	/**
	*@brief 事件过滤器
	*@param [in] : *watched : QObject，对象指针
				   *event : QEvent，事件指针
	**/
	bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::RealtimePlayerWidget *ui;
    enum
    {
        FOCUS_PLAYER_ZERO = 0,
        FOCUS_PLAYER_ONE,
    };
    int current_focus_player_no_{ FOCUS_PLAYER_ONE };

	QMap<int, QPair<QVariant, bool>> player_id_map_;//key：左播放器或右播放器，pair-first：id，pair-second：绘制矩形框是否使能

	QTimer *toolbar_visible_timer_;
	static const int kToolbarVisibleTimer_{ 5000 };//工具栏显示时间
};

#endif // REALTIMEPLAYERWIDGET_H
