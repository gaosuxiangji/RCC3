#ifndef CSLUTCURVEEDITWIDGET_H
#define CSLUTCURVEEDITWIDGET_H

#include <QObject>
#include <QWidget>
#include <QSharedPointer>
#include <QImage>
#include <QQueue>
#include <thread>
#include <atomic>
#include <opencv2/opencv.hpp>
#include <mutex>

class Device;

/**
 * @brief Lut曲线编辑控件
 */
class CSLutCurveEditWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CSLutCurveEditWidget(QWidget *parent = 0);
	~CSLutCurveEditWidget();

	void setDevice(QSharedPointer<Device> device_ptr);

	bool IsEnabled() const;

	void Default();

	void Apply();
    void updatePtInfo( QPoint pt1,  QPoint pt2);
	void getPtInfo(QPoint& pt1, QPoint& pt2);

protected:
	/** @brief 重载绘制事件
	@param [in] : QPaintEvent *event : 事件对象
	*/
	virtual void paintEvent(QPaintEvent *event) override;

	//鼠标按下事件
	virtual void mousePressEvent(QMouseEvent *event) override;
	//鼠标移动事件(未设置mouseTracking时,只有按下鼠标时移动鼠标才能接收到事件响应)
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	//鼠标抬起事件
	virtual void mouseReleaseEvent(QMouseEvent *event) override;
signals:
	void updateUI();

	void valueChanged();
public slots:

private:
	/** @brief 制作绘图缓冲区
	*/
	void MakePaintBuffer();

	void CtrlPointsChanged();

	void InitLut();

	//点序列在实际bit位和16bit之间转换
	void convertPointList(QList<QPoint> & point_list, bool to_device = true, uint8_t current_bpp = 8);

	//设置点之前的合法性检验修正
	void correctPoint(int & x, int &y);
	void calImgBar();
	void coutGrayNum(const cv::Mat& bc, const cv::Mat& gc, const cv::Mat& rc);
private:
	bool isFirstPaint = true;//是否是初次绘制播放控制控件
	QPixmap paintBuffer;//缓冲示意图
	QPixmap paintBuffer_backup;//绘图缓冲区的备份

	const int LABEL_FONT_SIZE = 11;//字体大小
	const int CTRL_POINT_RADIUS = 5;//控制点大小
	const int CTRL_POINT_NEAR_DIS = 20;//控制点邻近距离

	QList<QPoint> m_ctrlPoints; // 4个亮度曲线点，曲线点的X取值范围[0,1023],Y取值范围[0,255]
										//4点之间要求从0-3各点的X值依次增大，不能重复，0和3点的X不一定必须是0和1023
	unsigned short m_lut[1024] = {0};	//连续1024个点的纵坐标值，用于绘制亮度曲线
	int m_nCurrentPt;	//当前选中的控制点序号0-3，无选中时为-1

	QRect m_rectGrid;//网格绘制矩形框范围
	int m_ymax{ 255 };	//Y最大值
	int m_xmax{ 1023 };

	int m_nXImageMax{ 255 };	//x最大值

	QRect m_restrictArea;//拖动控制点的限制区域

	bool m_isEnabled{ true };//界面是否使能

	QSharedPointer<Device> m_device_ptr;
	const int m_lum_max{ 256 };    //亮度范围最大值
	//const int m_lum_num_range{ 1023 };    //将亮度数量归一化到0~1024范围内
	QQueue<QImage> m_img_queue{};
	std::atomic_bool m_bQuit{ false };
	std::thread* m_cal_img_bar{ nullptr };
	std::mutex m_mutex_img;    
	std::mutex m_mutex_histogram;
	int m_b_histogram[256]{};
	int m_g_histogram[256]{};
	int m_r_histogram[256]{};
};

#endif // CSLUTCURVEEDITWIDGET_H
