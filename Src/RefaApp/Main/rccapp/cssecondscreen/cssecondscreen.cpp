#include "cssecondscreen.h"
#include "ui_cssecondscreen.h"

CSSecondScreen::CSSecondScreen(const RccFrameInfo& img, int frame_head_type, QWidget *parent) :
    QDialog(parent),
	frame_head_type_(frame_head_type),
    ui(new Ui::CSSecondScreen)
{
    ui->setupUi(this);
	Init();
	if (m_player)
	{
		m_player->SlotUpdateImage(img);
	}
}

CSSecondScreen::~CSSecondScreen()
{
	delete ui;
}

void CSSecondScreen::SlotImageUpdate(const RccFrameInfo& image)
{
	if (m_player)
	{
		m_player->SlotUpdateImage(image);
	}
}

void CSSecondScreen::accept()
{
	close();
}

void CSSecondScreen::reject()
{
	close();
}

void CSSecondScreen::closeEvent(QCloseEvent* event)
{
	Q_UNUSED(event)
	emit SecondScreenCloseSignal();
}

void CSSecondScreen::Init()
{
	m_player = new CPlayerViewBase(20, frame_head_type_, ui->second_screen_widget);
	InitUI();
}

void CSSecondScreen::InitUI()
{
	//去除标题框
	setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
	QHBoxLayout* secondScreenLayout = new QHBoxLayout(ui->second_screen_widget);
	secondScreenLayout->setContentsMargins(0, 0, 0, 0);
	secondScreenLayout->setSpacing(0);
	ui->second_screen_widget->setLayout(secondScreenLayout);
	secondScreenLayout->addWidget(m_player);
}

void CSSecondScreen::mousePressEvent(QMouseEvent * event)
{
	if (event->buttons()&Qt::LeftButton)//buttons处理的是长事件，button处理的是短暂的事件
	{
		m_pt = event->globalPos() - this->geometry().topLeft();
	}
}

void CSSecondScreen::mouseMoveEvent(QMouseEvent * event)
{
	if (event->buttons()&Qt::LeftButton)
	{
		this->move(event->globalPos() - m_pt);
	}
}
