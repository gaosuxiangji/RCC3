#include "framelesswindow.h"
#include "ui_framelesswindow.h"

#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>

#include "Common/UIUtils/uiutils.h"
#include "System/SystemSettings/systemsettingsmanager.h"
#include "Device/devicemanager.h"
#include "Device/device.h"

FramelessWindow::FramelessWindow(QWidget *parent)
	: QWidget(parent),
	ui(new Ui::FramelessWindow),
	m_bMousePressed(false),
	m_bDragTop(false),
	m_bDragLeft(false),
	m_bDragRight(false),
	m_bDragBottom(false)
{
	ui->setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);

	setMouseTracking(true);

	ui->icon->setScaledContents(true);

	QApplication::instance()->installEventFilter(this);
}

FramelessWindow::~FramelessWindow() 
{ 
	delete ui; 
}

void FramelessWindow::setContent(QWidget *w)
{
	ui->windowContent->layout()->addWidget(w);
	setWindowIcon(w->windowIcon());
	setWindowTitle(w->windowTitle());

	adjustSize();
}

void FramelessWindow::setWindowTitle(const QString &text)
{
	ui->titleText->setText(text);
}

void FramelessWindow::setWindowIcon(const QIcon &ico)
{
	ui->icon->setPixmap(ico.pixmap(ui->icon->sizeHint()));
}

void FramelessWindow::on_maximizeButton_clicked()
{
	ui->maximizeButton->setVisible(false);
	this->setWindowState(Qt::WindowMaximized);
	this->showMaximized();
}

void FramelessWindow::on_minimizeButton_clicked()
{
	setWindowState(Qt::WindowMinimized);
}

void FramelessWindow::on_closeButton_clicked()
{
	close(); 
}

void FramelessWindow::on_windowTitlebar_doubleClicked()
{
	if (windowState().testFlag(Qt::WindowNoState))
	{
		on_maximizeButton_clicked();
	}
}

void FramelessWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
	Q_UNUSED(event);
}

void FramelessWindow::checkBorderDragging(QMouseEvent *event)
{
	if (isMaximized()) {
		return;
	}

	QPoint globalMousePos = event->globalPos();
	if (m_bMousePressed) {
		QScreen *screen = QGuiApplication::primaryScreen();
		QRect availGeometry = screen->availableGeometry();
		int h = availGeometry.height();
		int w = availGeometry.width();
		QList<QScreen *> screenlist = screen->virtualSiblings();
		if (screenlist.contains(screen)) {
			QSize sz = QApplication::desktop()->size();
			h = sz.height();
			w = sz.width();
		}

		if (m_bDragTop && m_bDragRight) // 右上角
		{
			int diff = globalMousePos.x() - (m_StartGeometry.x() + m_StartGeometry.width());
			int neww = m_StartGeometry.width() + diff;
			diff = globalMousePos.y() - m_StartGeometry.y();
			int newy = m_StartGeometry.y() + diff;
			if (neww > 0 && newy > 0 && newy < h - 50) {
				QRect newg = m_StartGeometry;
				newg.setWidth(neww);
				newg.setX(m_StartGeometry.x());
				newg.setY(newy);
				setGeometry(newg);
			}
		}
		else if (m_bDragTop && m_bDragLeft) // 左上角
		{
			int diff = globalMousePos.y() - m_StartGeometry.y();
			int newy = m_StartGeometry.y() + diff;
			diff = globalMousePos.x() - m_StartGeometry.x();
			int newx = m_StartGeometry.x() + diff;
			if (newy > 0 && newx > 0) {
				QRect newg = m_StartGeometry;
				newg.setY(newy);
				newg.setX(newx);
				setGeometry(newg);
			}
		}
		else if (m_bDragBottom && m_bDragLeft) // 左下角
		{
			int diff = globalMousePos.y() - (m_StartGeometry.y() + m_StartGeometry.height());
			int newh = m_StartGeometry.height() + diff;
			diff = globalMousePos.x() - m_StartGeometry.x();
			int newx = m_StartGeometry.x() + diff;
			if (newh > 0 && newx > 0) {
				QRect newg = m_StartGeometry;
				newg.setX(newx);
				newg.setHeight(newh);
				setGeometry(newg);
			}
		}
		else if (m_bDragBottom && m_bDragRight) // 右下角
		{
			int diff = globalMousePos.x() - (m_StartGeometry.x() + m_StartGeometry.width());
			int neww = m_StartGeometry.width() + diff;
			diff = globalMousePos.y() - (m_StartGeometry.y() + m_StartGeometry.height());
			int newh = m_StartGeometry.height() + diff;
			if (newh > 0 && neww > 0)
			{
				QRect newg = m_StartGeometry;
				newg.setWidth(neww);
				newg.setHeight(newh);
				setGeometry(newg);
			}
		}
		else if (m_bDragTop) {
			int diff = globalMousePos.y() - m_StartGeometry.y();
			int newy = m_StartGeometry.y() + diff;
			if (newy > 0 && newy < h - 50) {
				QRect newg = m_StartGeometry;
				newg.setY(newy);
				setGeometry(newg);
			}
		}
		else if (m_bDragLeft) {
			int diff = globalMousePos.x() - m_StartGeometry.x();
			int newx = m_StartGeometry.x() + diff;
			if (newx > 0 && newx < w - 50) {
				QRect newg = m_StartGeometry;
				newg.setX(newx);
				setGeometry(newg);
			}
		}
		else if (m_bDragRight) {
			int diff =
				globalMousePos.x() - (m_StartGeometry.x() + m_StartGeometry.width());
			int neww = m_StartGeometry.width() + diff;
			if (neww > 0) {
				QRect newg = m_StartGeometry;
				newg.setWidth(neww);
				newg.setX(m_StartGeometry.x());
				setGeometry(newg);
			}
		}
		else if (m_bDragBottom) {
			int diff =
				globalMousePos.y() - (m_StartGeometry.y() + m_StartGeometry.height());
			int newh = m_StartGeometry.height() + diff;
			if (newh > 0) {
				QRect newg = m_StartGeometry;
				newg.setHeight(newh);
				newg.setY(m_StartGeometry.y());
				setGeometry(newg);
			}
		}
	}
	else {
		// no mouse pressed
		if (leftBorderHit(globalMousePos) && topBorderHit(globalMousePos))
		{
			setCursor(Qt::SizeFDiagCursor);
		}
		else if (rightBorderHit(globalMousePos) && topBorderHit(globalMousePos))
		{
			setCursor(Qt::SizeBDiagCursor);
		}
		else if (leftBorderHit(globalMousePos) && bottomBorderHit(globalMousePos))
		{
			setCursor(Qt::SizeBDiagCursor);
		}
		else if (rightBorderHit(globalMousePos) && bottomBorderHit(globalMousePos))
		{
			setCursor(Qt::SizeFDiagCursor);
		}
		else
		{
			if (topBorderHit(globalMousePos))
			{
				setCursor(Qt::SizeVerCursor);
			}
			else if (leftBorderHit(globalMousePos))
			{
				setCursor(Qt::SizeHorCursor);
			}
			else if (rightBorderHit(globalMousePos))
			{
				setCursor(Qt::SizeHorCursor);
			}
			else if (bottomBorderHit(globalMousePos))
			{
				setCursor(Qt::SizeVerCursor);
			}
			else {
				m_bDragTop = false;
				m_bDragLeft = false;
				m_bDragRight = false;
				m_bDragBottom = false;
				setCursor(Qt::ArrowCursor);
			}
		}
	}
}

// pos in global virtual desktop coordinates
bool FramelessWindow::leftBorderHit(const QPoint &pos) {
	const QRect &rect = this->geometry();
	if (pos.x() >= rect.x() && pos.x() <= rect.x() + CONST_DRAG_BORDER_SIZE) {
		return true;
	}
	return false;
}

bool FramelessWindow::rightBorderHit(const QPoint &pos) {
	const QRect &rect = this->geometry();
	int tmp = rect.x() + rect.width();
	if (pos.x() <= tmp && pos.x() >= (tmp - CONST_DRAG_BORDER_SIZE)) {
		return true;
	}
	return false;
}

bool FramelessWindow::topBorderHit(const QPoint &pos) {
	const QRect &rect = this->geometry();
	if (pos.y() >= rect.y() && pos.y() <= rect.y() + CONST_DRAG_BORDER_SIZE) {
		return true;
	}
	return false;
}

bool FramelessWindow::bottomBorderHit(const QPoint &pos) {
	const QRect &rect = this->geometry();
	int tmp = rect.y() + rect.height();
	if (pos.y() <= tmp && pos.y() >= (tmp - CONST_DRAG_BORDER_SIZE)) {
		return true;
	}
	return false;
}

void FramelessWindow::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::WindowStateChange)
	{
		if (windowState().testFlag(Qt::WindowNoState))
		{
			ui->maximizeButton->setVisible(true);
			event->ignore();
		}
		else if (windowState().testFlag(Qt::WindowMaximized))
		{
			ui->maximizeButton->setVisible(false);
			event->ignore();
		}
	}
	event->accept();
}

void FramelessWindow::mousePressEvent(QMouseEvent *event) {
	if (isMaximized()) {
		return;
	}

	m_bMousePressed = true;
	m_StartGeometry = this->geometry();

	QPoint globalMousePos = mapToGlobal(QPoint(event->x(), event->y()));

	if (leftBorderHit(globalMousePos) && topBorderHit(globalMousePos))
	{
		m_bDragTop = true;
		m_bDragLeft = true;
		setCursor(Qt::SizeFDiagCursor);
	}
	else if (rightBorderHit(globalMousePos) && topBorderHit(globalMousePos))
	{
		m_bDragRight = true;
		m_bDragTop = true;
		setCursor(Qt::SizeBDiagCursor);
	}
	else if (leftBorderHit(globalMousePos) && bottomBorderHit(globalMousePos))
	{
		m_bDragLeft = true;
		m_bDragBottom = true;
		setCursor(Qt::SizeBDiagCursor);
	}
	else if (rightBorderHit(globalMousePos) && bottomBorderHit(globalMousePos))
	{
		m_bDragRight= true;
		m_bDragBottom = true;
		setCursor(Qt::SizeFDiagCursor);
	}
	else {
		if (topBorderHit(globalMousePos)) {
			m_bDragTop = true;
			setCursor(Qt::SizeVerCursor);
		}
		else if (leftBorderHit(globalMousePos)) {
			m_bDragLeft = true;
			setCursor(Qt::SizeHorCursor);
		}
		else if (rightBorderHit(globalMousePos)) {
			m_bDragRight = true;
			setCursor(Qt::SizeHorCursor);
		}
		else if (bottomBorderHit(globalMousePos)) {
			m_bDragBottom = true;
			setCursor(Qt::SizeVerCursor);
		}
	}
}

void FramelessWindow::mouseReleaseEvent(QMouseEvent *event) {
	Q_UNUSED(event);
	if (isMaximized()) {
		return;
	}

	m_bMousePressed = false;
	bool bSwitchBackCursorNeeded =
		m_bDragTop || m_bDragLeft || m_bDragRight || m_bDragBottom;
	m_bDragTop = false;
	m_bDragLeft = false;
	m_bDragRight = false;
	m_bDragBottom = false;
	if (bSwitchBackCursorNeeded) {
		setCursor(Qt::ArrowCursor);
	}
}

bool FramelessWindow::eventFilter(QObject *obj, QEvent *event) {
	if (isMaximized()) {
		return QWidget::eventFilter(obj, event);
	}

	// check mouse move event when mouse is moved on any object
	if (event->type() == QEvent::MouseMove) {
		QMouseEvent *pMouse = dynamic_cast<QMouseEvent *>(event);
		if (pMouse) {
			checkBorderDragging(pMouse);
		}
	}
	// press is triggered only on frame window
	else if (event->type() == QEvent::MouseButtonPress && obj == this) {
		QMouseEvent *pMouse = dynamic_cast<QMouseEvent *>(event);
		if (pMouse) {
			mousePressEvent(pMouse);
		}
	}
	else if (event->type() == QEvent::MouseButtonRelease) {
		if (m_bMousePressed) {
			QMouseEvent *pMouse = dynamic_cast<QMouseEvent *>(event);
			if (pMouse) {
				mouseReleaseEvent(pMouse);
			}
		}
	}

	return QWidget::eventFilter(obj, event);
}

void FramelessWindow::closeEvent(QCloseEvent *event)
{
	QString line1 = tr("Confirm close?");
	QString line2 = tr("After clicking OK, the system will disconnect all devices and close the software.");
	bool ok = UIUtils::showQuestionMsgBox(this, QString("%1\r\n%2").arg(line1).arg(line2), 1);
	if (ok)
	{
		// 关闭时将已连接相机写入配置，供下次打开时自动连接使用
		QStringList connected_devices;
		QList<QSharedPointer<Device>> add_devices;
		DeviceManager::instance().getAddedDevices(add_devices);
		for (auto var : add_devices)
		{
			connected_devices << var->getIp();
		}
		SystemSettingsManager::instance().setAutoConnectedDevice(connected_devices);

		// 关闭时断开所有设备信号连接，防止软件关闭时程序崩溃
		QList<QSharedPointer<Device>> devices;
		DeviceManager::instance().getDevices(devices);
		for  (auto var : devices)
		{
			var->QObject::disconnect();
		}

		DeviceManager::instance().disconnectAll();

		event->accept();
	}
	else
	{
		event->ignore();
	}
}
