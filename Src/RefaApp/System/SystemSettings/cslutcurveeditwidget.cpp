#include "cslutcurveeditwidget.h"
#include <QPainter>
#include "Device/device.h"
#include "HscImageProcessAPI.h"
#include <QFontMetrics>
#include <QFont>
#include <QVariant>
#include <QMouseEvent>
#include <cmath>
#include "../../AGSDK/AGCommon/Src/RccNS/Util/HscSystemElapsedTimer.h"

#define VERTICAL_SPAN 4
#define HORIZONTAL_SPAN 8
CSLutCurveEditWidget::CSLutCurveEditWidget(QWidget *parent) : QWidget(parent)
{
	m_cal_img_bar = new std::thread(&CSLutCurveEditWidget::calImgBar, this);
	m_ctrlPoints.clear();
	for (int i = 0; i < 4; i++)
	{
		m_ctrlPoints.append(QPoint(0, 0));
	}
	connect(this, &CSLutCurveEditWidget::updateUI, this, [=] {
		MakePaintBuffer();
		update();
	});
}

CSLutCurveEditWidget::~CSLutCurveEditWidget()
{
	m_bQuit = true;
	if (nullptr != m_cal_img_bar)
	{
		if (m_cal_img_bar->joinable())
		{
			m_cal_img_bar->join();
			delete m_cal_img_bar;
			m_cal_img_bar = nullptr;
		}
	}

}

void CSLutCurveEditWidget::setDevice(QSharedPointer<Device> device_ptr)
{
	m_device_ptr = device_ptr;
	connect(m_device_ptr.data(), &Device::updateFrame, this, [=](RccFrameInfo image) {
		std::lock_guard<std::mutex> lockGuard_img(m_mutex_img);
		m_img_queue.append(image.image);
		if (m_img_queue.size() > 1) {
			m_img_queue.dequeue();
		}
	}, Qt::QueuedConnection);

	if (device_ptr)
	{
		if (device_ptr->isSupportHighBitParam())
		{
			m_ymax = Device::ImageBitsChange(device_ptr->GetLUTValueMax(), 16, m_device_ptr->getProperty(Device::PropPixelBitDepth, true).toInt());
		}
		else
		{
			m_ymax = device_ptr->GetLUTValueMax();
		}
		m_isEnabled = device_ptr->isDigitalGainEnable();
		int nBitsPerPixel = device_ptr->getProperty(Device::PropPixelBitDepth,true).toInt();
		m_nXImageMax = std::pow(2, nBitsPerPixel) - 1;
	}

	m_nCurrentPt = -1;
	InitLut();
}

bool CSLutCurveEditWidget::IsEnabled() const
{
	return m_isEnabled;

}

void CSLutCurveEditWidget::Default()
{
	if (m_device_ptr)
	{
		QList<QVariant> point_list;
		m_device_ptr->GetDefaultLUTCtrlPoints(point_list);
		m_ctrlPoints.clear();
		for (QVariant point : point_list)
		{
			m_ctrlPoints << point.toPoint();
		}
		if (m_device_ptr->isSupportHighBitParam())
		{
			//点位数转换
			convertPointList(m_ctrlPoints, false, m_device_ptr->getProperty(Device::PropPixelBitDepth, true).toInt());
		}
		CtrlPointsChanged();

		Apply();
	}
}

void CSLutCurveEditWidget::Apply()
{
	if (m_device_ptr && m_isEnabled)
	{
		if (m_ctrlPoints.at(3).x() == 0 && m_ctrlPoints.at(3).y() == 0)
		{
			return;
		}
		QList<QVariant> point_list;
		auto temp_list = m_ctrlPoints;
		if (m_device_ptr->isSupportHighBitParam())
		{
			//点位数转换
			convertPointList(temp_list, true, m_device_ptr->getProperty(Device::PropPixelBitDepth, true).toInt());
		}
		for (QPoint point : temp_list)
		{
			point_list << point;
		}
		emit valueChanged();
		m_device_ptr->setProperty(Device::PropLut, point_list);
	}
}

void CSLutCurveEditWidget::updatePtInfo( QPoint pt1,  QPoint pt2)
{
	QPoint pt_two{}, pt_three{};
	
	pt2.setY(m_ymax);

	//参数修正,保持存在斜率
	if (pt2.x() - pt1.x() < 4)
	{
		if (pt1.x() < 4)
		{
			pt2.setX(pt1.x() + 4);
		}
		if (pt2.x() > m_xmax -4)
		{
			pt1.setX(pt2.x() - 4);
		}
	}

	int x_interval = ceil((pt2.x() - pt1.x()) / 3);
	int y_interval = ceil((pt2.y() - pt1.y()) / 3);

	if (m_ctrlPoints.size() == 4) {


		m_ctrlPoints[0].setX(pt1.x());
		m_ctrlPoints[0].setY(pt1.y());

		m_ctrlPoints[1].setX(pt1.x() + x_interval);
		m_ctrlPoints[1].setY(pt1.y() + y_interval);

		m_ctrlPoints[2].setX(pt1.x() + x_interval * 2);
		m_ctrlPoints[2].setY(pt1.y() + y_interval * 2);

		m_ctrlPoints[3].setX(pt2.x());
		m_ctrlPoints[3].setY(pt2.y());

		Apply();
		CtrlPointsChanged();
	}
}

void CSLutCurveEditWidget::getPtInfo(QPoint& pt1, QPoint& pt2)
{
	pt1 = m_ctrlPoints[0];
	pt2 = m_ctrlPoints[3];
	return;
}

void CSLutCurveEditWidget::paintEvent(QPaintEvent *event)
{
	QWidget::paintEvent(event);

	QPainter painter(this);
	if (isFirstPaint)
	{
		paintBuffer_backup = QPixmap(this->size());
		paintBuffer_backup.fill(this->palette().window().color());
		MakePaintBuffer();
		isFirstPaint = false;
	}

	painter.drawPixmap(0, 0, paintBuffer);
}

void CSLutCurveEditWidget::mousePressEvent(QMouseEvent *event)
{
	if (m_isEnabled)
	{
		QPoint point = event->pos();
		//捕捉可拖动的曲线控制点
		//将point坐标转为控制点坐标系坐标
		int x = (point.x() - m_rectGrid.left()) * m_xmax / m_rectGrid.width();
		int y = (m_rectGrid.bottom() - point.y()) * m_ymax / m_rectGrid.height();

		//判断point是否在某个控制点附近
		bool bPicked = false;
		for (int i = 0; i < 4; i++)
		{
			if (abs(m_ctrlPoints[i].x() - x) < CTRL_POINT_NEAR_DIS && abs(m_ctrlPoints[i].y() - y) < CTRL_POINT_NEAR_DIS * m_ymax / m_xmax)
			{
				m_nCurrentPt = i;
				bPicked = true;
				//限制控制点运动范围
				int min, max;	//该选中点拖动范围X不能超过min和max的限制
				if (m_nCurrentPt == 0)
				{
					min = 0;
				}
				else 
				{
					min = m_ctrlPoints[m_nCurrentPt - 1].x() + 1; 
				}
				if (m_nCurrentPt == 3) 
				{
					max = 1023;
				}
				else
				{
					max = m_ctrlPoints[m_nCurrentPt + 1].x() - 1;
				}

				min = m_rectGrid.left() + min * m_rectGrid.width() / m_xmax;
				max = m_rectGrid.left() + max * m_rectGrid.width() / m_xmax;

				//确定限制拖动区域
				m_restrictArea.setLeft(min);
				m_restrictArea.setRight(max);
				m_restrictArea.setTop(m_rectGrid.top());
				m_restrictArea.setBottom(m_rectGrid.bottom());

				break;
			}
		}
		if (!bPicked)
		{
			m_nCurrentPt = -1;
		}
	}
	QWidget::mousePressEvent(event);
}

void CSLutCurveEditWidget::mouseMoveEvent(QMouseEvent *event)
{
	if (m_isEnabled)
	{
		if (m_nCurrentPt > -1 && m_restrictArea.isValid())
		{
			QPoint point = event->pos();
			//有选中点存在，则随着鼠标移动改变其位置，但要注意不要超过其左右点的x值限制
			if (point.x()<m_restrictArea.left())
			{
				point.setX(m_restrictArea.left());
			}
			if (point.x() > m_restrictArea.right())
			{
				point.setX(m_restrictArea.right());
			}
			if (point.y() < m_restrictArea.top())
			{
				point.setY(m_restrictArea.top());
			}
			if (point.y() > m_restrictArea.bottom())
			{
				point.setY(m_restrictArea.bottom());
			}
			//计算对应lut点值
			int x = (point.x() - m_rectGrid.left()) * m_xmax / m_rectGrid.width();
			int y = (m_rectGrid.bottom() - point.y()) * m_ymax / m_rectGrid.height();

			//合法性检验修正
			correctPoint(x, y);

			m_ctrlPoints[m_nCurrentPt].setX(x);
			m_ctrlPoints[m_nCurrentPt].setY(y);

			CtrlPointsChanged();
		}
	}
	QWidget::mouseMoveEvent(event);
}

void CSLutCurveEditWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if (m_isEnabled)
	{
		Apply();

		m_nCurrentPt = -1;
	}
	//重置拖动点限制区域
	m_restrictArea = QRect(0, 0, 1, 1);
	QWidget::mouseReleaseEvent(event);
}

void CSLutCurveEditWidget::MakePaintBuffer()
{
	paintBuffer = paintBuffer_backup;

	QPainter painter(&paintBuffer);

	QRect rect = this->rect();

	//绘制操作,绘制网格背景
	painter.fillRect(rect,Qt::black);

	QPen pen(Qt::white,1);
	QVector<qreal>dashes;
	qreal space = 4;
	dashes << 3 << space;
	pen.setDashPattern(dashes);
	painter.setPen(pen);

	//获取标签文字,计算网格绘制矩形
	QFont font;
	//font.setPointSize(9);
	painter.setFont(font);
	QFontMetrics fm(font);
	//获取字体最大宽高
	int label_max_width = fm.width(QString::number(m_xmax));
	int label_max_height = fm.height();
	//确认网格布局
	m_rectGrid.setLeft(rect.left() + label_max_width + CTRL_POINT_RADIUS);
	m_rectGrid.setRight(rect.right() - label_max_width);
	m_rectGrid.setTop(rect.top() + label_max_height);
	m_rectGrid.setBottom(rect.bottom() - label_max_height - CTRL_POINT_RADIUS);

	if (m_rectGrid.isEmpty())
	{
		return;
	}
	//计算每个网格的宽高
	double dx = m_rectGrid.width() * 1.0 / HORIZONTAL_SPAN;
	double dy = m_rectGrid.height() * 1.0 / VERTICAL_SPAN;

	//绘制纵轴坐标文字
	QString label_str;
	for (int i = 0; i < VERTICAL_SPAN + 1; i++)
	{
		label_str = QString::number(m_nXImageMax * i / VERTICAL_SPAN);
		painter.drawText(QPoint(rect.left(), m_rectGrid.bottom()  - i * dy + label_max_height/2), label_str);
	}
	//绘制横坐标轴文字
	for (int i = 0; i < HORIZONTAL_SPAN + 1; i++)
	{
		label_str = QString::number(m_nXImageMax * i / HORIZONTAL_SPAN);
		painter.drawText(QPoint(m_rectGrid.left()/*- label_max_width/2 */+ i*dx, rect.bottom() /*- label_max_height*/), label_str);
	}

	//绘制网格框
	painter.drawRect(m_rectGrid);
	//绘制横向网格
	for (int i = 1; i < VERTICAL_SPAN; i++)
	{
		painter.drawLine(
			QPoint(m_rectGrid.left(), m_rectGrid.top() + i*dy), 
			QPoint(m_rectGrid.right(), m_rectGrid.top() + i*dy));
	}
	//绘制纵向网格
	for (int i = 1; i < HORIZONTAL_SPAN; i++)
	{
		painter.drawLine(
			QPoint(m_rectGrid.left() + i * dx, m_rectGrid.top()),
			QPoint(m_rectGrid.left() + i * dx, m_rectGrid.bottom()));
	}

	if (m_isEnabled)
	{
		//绘制点坐标
		QPen bule_pen(Qt::blue); 
		if (!m_isEnabled) bule_pen = QPen(Qt::gray);
		painter.setPen(bule_pen);
		int x, y;
		for (int i = 0; i < 4 ; i++)
		{
			x = m_rectGrid.left() + m_ctrlPoints.at(i).x() * m_rectGrid.width() / m_xmax;
			y = m_rectGrid.bottom() - m_ctrlPoints.at(i).y() * m_rectGrid.height() / m_ymax;
			painter.drawRect(x - CTRL_POINT_RADIUS, y - CTRL_POINT_RADIUS, 2 * CTRL_POINT_RADIUS, 2 * CTRL_POINT_RADIUS);
		}

		//绘制曲线,
		QPainterPath lut_curve;
		x = m_rectGrid.left();
		y = m_rectGrid.bottom() - m_lut[0] * m_rectGrid.height() / m_ymax;
		lut_curve.moveTo(x, y);
		for (int i = 1; i < 1024; i++)
		{
			x = m_rectGrid.left() + i * m_rectGrid.width() / m_xmax;
			y = m_rectGrid.bottom() - m_lut[i] * m_rectGrid.height() / m_ymax;
			lut_curve.lineTo(x, y);
		}
		painter.drawPath(lut_curve);
	}

	for (int i = 0; i < m_lum_max; i++)
	{
		int x = m_rectGrid.left() + i * m_rectGrid.width() / m_lum_max;
		int y = m_rectGrid.bottom();
		std::lock_guard<std::mutex> lockGuard_histogram(m_mutex_histogram);

		painter.save();
		painter.setBrush(QColor(0,0,255,150));
		painter.setPen(Qt::NoPen);
		QRectF rectangle_b(x, y, 1, -(m_b_histogram[i] * m_rectGrid.height() / m_ymax));
		painter.drawRect(rectangle_b);
		painter.restore();

		painter.save();
		painter.setBrush(QColor(0, 255, 0, 150));
		painter.setPen(Qt::NoPen);
		QRectF rectangle_g(x, y, 1, -(m_g_histogram[i] * m_rectGrid.height() / m_ymax));
		painter.drawRect(rectangle_g);
		painter.restore();

		painter.save();
		painter.setBrush(QColor(255, 0, 0, 150));
		painter.setPen(Qt::NoPen);
		QRectF rectangle_r(x, y, 1, -(m_r_histogram[i] * m_rectGrid.height() / m_ymax));
		painter.drawRect(rectangle_r);
		painter.restore();
	}
	


}

void CSLutCurveEditWidget::CtrlPointsChanged()
{
	unsigned short lutX[4];
	unsigned short lutY[4];
	
	for (int i = 0; i < 4; i++)
	{
		lutX[i] = m_ctrlPoints.at(i).x();
		lutY[i] = m_ctrlPoints.at(i).y();
	}
	HscFillLutArray(lutX, lutY, m_lut);

	MakePaintBuffer();
	repaint();
}

void CSLutCurveEditWidget::InitLut()
{
	if (!m_device_ptr.isNull())
	{
		QList<QVariant>lut_points = m_device_ptr->getProperty(Device::PropLut).toList();
		m_ctrlPoints.clear();
		for (int i = 0; i < 4 ; i++)
		{
			m_ctrlPoints.append(lut_points.at(i).toPoint());
		}
		//若未读到配置，默认设一个初值，防止曲线加载时4个控制点都是0，0
		if (m_ctrlPoints.at(3).x() == 0 && m_ctrlPoints.at(3).y() == 0)
		{
			m_device_ptr->GetDefaultLUTCtrlPoints(lut_points);
			m_ctrlPoints.clear();
			for (int i = 0; i < 4; i++)
			{
				m_ctrlPoints.append(lut_points.at(i).toPoint());
			}
		}
		if (m_device_ptr->isSupportHighBitParam())
		{
			//点位数转换
			convertPointList(m_ctrlPoints, false, m_device_ptr->getProperty(Device::PropPixelBitDepth, true).toInt());
		}
		CtrlPointsChanged();
	}



	MakePaintBuffer();
	repaint();
}

void CSLutCurveEditWidget::convertPointList(QList<QPoint> & point_list, bool to_device /*= true*/, uint8_t current_bpp /*= 8*/)
{
	if (to_device)//当前bpp转到16bit
	{

		switch (current_bpp)
		{
		case 8:
		case 10:
		case 12:
		{
			point_list[0].setX(Device::ImageBitsChange(point_list[0].x(), 10, 16));
			point_list[1].setX(Device::ImageBitsChange(point_list[1].x(), 10, 16));
			point_list[2].setX(Device::ImageBitsChange(point_list[2].x(), 10, 16));
			point_list[3].setX(Device::ImageBitsChange(point_list[3].x(), 10, 16));
			point_list[0].setY(Device::ImageBitsChange(point_list[0].y(), current_bpp, 16));
			point_list[1].setY(Device::ImageBitsChange(point_list[1].y(), current_bpp, 16));
			point_list[2].setY(Device::ImageBitsChange(point_list[2].y(), current_bpp, 16));
			point_list[3].setY(Device::ImageBitsChange(point_list[3].y(), current_bpp, 16));
		}
		break;
		default:
			break;
		}
	}
	else
	{

		switch (current_bpp)
		{
		case 8:
		case 10:
		case 12:
		{
			point_list[0].setX(Device::ImageBitsChange(point_list[0].x(), 16, 10));
			point_list[1].setX(Device::ImageBitsChange(point_list[1].x(), 16, 10));
			point_list[2].setX(Device::ImageBitsChange(point_list[2].x(), 16, 10));
			point_list[3].setX(Device::ImageBitsChange(point_list[3].x(), 16, 10));
			point_list[0].setY(Device::ImageBitsChange(point_list[0].y(), 16, current_bpp));
			point_list[1].setY(Device::ImageBitsChange(point_list[1].y(), 16, current_bpp));
			point_list[2].setY(Device::ImageBitsChange(point_list[2].y(), 16, current_bpp));
			point_list[3].setY(Device::ImageBitsChange(point_list[3].y(), 16, current_bpp));
		}
		break;
		default:
			break;
		}
	}
}

void CSLutCurveEditWidget::correctPoint(int & x, int &y)
{
	int min, max;	//该选中点拖动范围X不能超过min和max的限制
	if (m_nCurrentPt == 0){min = 0;}
	else{min = m_ctrlPoints[m_nCurrentPt - 1].x() + 1;}

	if (m_nCurrentPt == 3){max = 1023;}
	else{max = m_ctrlPoints[m_nCurrentPt + 1].x() - 1;}

	if (x < min){x = min;}

	if (x > max){x = max;}

	if (y < 0){y = 0;}

	if (y > m_ymax){y = m_ymax;}
}

void CSLutCurveEditWidget::calImgBar()
{
	while (!m_bQuit) {
		if (m_img_queue.size() > 0) {
			QImage src{};
			{
				std::lock_guard<std::mutex> lockGuard_img(m_mutex_img);
				src = m_img_queue.last();
			}
			
			std::vector<cv::Mat> bgr_planes;
			cv::Mat dst{};
			switch (src.format())
			{
			case QImage::Format_Grayscale8:
				dst = cv::Mat(src.height(), src.width(), CV_8UC1, (void*)src.bits(), (size_t)src.bytesPerLine());
				break;
			case QImage::Format_ARGB32:
			case QImage::Format_RGB32:
			case QImage::Format_ARGB32_Premultiplied:
				dst = cv::Mat{ src.height(),src.width(),CV_8UC4,(void*)src.bits(),(size_t)src.bytesPerLine() };
				break;
			case QImage::Format_RGB888:
				dst = cv::Mat{ src.height(),src.width(),CV_8UC3,(void*)src.bits(),(size_t)src.bytesPerLine() };
				cv::cvtColor(dst, dst, cv::COLOR_BGR2RGB);
				//注意: 该行为会变更src的rgb为bgr
				break;
			case QImage::Format_Indexed8:
				dst = cv::Mat{ src.height(),src.width(),CV_8UC1,(void*)src.bits(),(size_t)src.bytesPerLine() };
				break;
			}

			split(dst, bgr_planes);

			int histSize = m_lum_max;
			float range[] = { 0,256 };
			const float *ranges[1] = { range };
			cv::Mat b_hist, g_hist, r_hist;

			if (1 == bgr_planes.size()) {
				calcHist(&bgr_planes[0], 1, 0, cv::Mat(), b_hist, 1, &histSize, ranges, true, false);
				normalize(b_hist, b_hist, 0, m_ymax, cv::NORM_MINMAX, -1, cv::Mat());
			}
			
			if (3 == bgr_planes.size()) {
				calcHist(&bgr_planes[0], 1, 0, cv::Mat(), b_hist, 1, &histSize, ranges, true, false);
				normalize(b_hist, b_hist, 0, m_ymax, cv::NORM_MINMAX, -1, cv::Mat());

				calcHist(&bgr_planes[1], 1, 0, cv::Mat(), g_hist, 1, &histSize, ranges, true, false);
				normalize(g_hist, g_hist, 0, m_ymax, cv::NORM_MINMAX, -1, cv::Mat());

				calcHist(&bgr_planes[2], 1, 0, cv::Mat(), r_hist, 1, &histSize, ranges, true, false);
				normalize(r_hist, r_hist, 0, m_ymax, cv::NORM_MINMAX, -1, cv::Mat());
			}

			if (4 == bgr_planes.size()) {
				calcHist(&bgr_planes[2], 1, 0, cv::Mat(), b_hist, 1, &histSize, ranges, true, false);
				normalize(b_hist, b_hist, 0, m_ymax, cv::NORM_MINMAX, -1, cv::Mat());

				calcHist(&bgr_planes[1], 1, 0, cv::Mat(), g_hist, 1, &histSize, ranges, true, false);
				normalize(g_hist, g_hist, 0, m_ymax, cv::NORM_MINMAX, -1, cv::Mat());

				calcHist(&bgr_planes[0], 1, 0, cv::Mat(), r_hist, 1, &histSize, ranges, true, false);
				normalize(r_hist, r_hist, 0, m_ymax, cv::NORM_MINMAX, -1, cv::Mat());
			}

			coutGrayNum(b_hist, g_hist, r_hist);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void CSLutCurveEditWidget::coutGrayNum(const cv::Mat& bc, const cv::Mat& gc, const cv::Mat& rc)
{
	{
		std::lock_guard<std::mutex> lockGuard_histogram(m_mutex_histogram);
		memset(m_b_histogram, 0, sizeof(m_b_histogram));
		for (int i = 0; i < bc.rows; i++) {
			for (int j = 0; j < bc.cols; j++) {
				m_b_histogram[i] = (floor)(bc.at<float>(i, j));
			}
		}

		memset(m_g_histogram, 0, sizeof(m_g_histogram));
		for (int i = 0; i < gc.rows; i++) {
			for (int j = 0; j < gc.cols; j++) {
				m_g_histogram[i] = (floor)(gc.at<float>(i, j));
			}
		}

		memset(m_r_histogram, 0, sizeof(m_r_histogram));
		for (int i = 0; i < rc.rows; i++) {
			for (int j = 0; j < rc.cols; j++) {
				m_r_histogram[i] = (floor)(rc.at<float>(i, j));
			}
		}
	}

	emit updateUI();
}
