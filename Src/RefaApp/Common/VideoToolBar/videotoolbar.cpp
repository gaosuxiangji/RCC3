#include "videotoolbar.h"
#include "ui_videotoolbar.h"

VideoToolBar::VideoToolBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoToolBar)
{
    ui->setupUi(this);

    response_enable_buttons_ << ui->toolButtonZoomIn;
    response_enable_buttons_ << ui->toolButtonZoomOut;
    response_enable_buttons_ << ui->toolButtonZoomFull;
    response_enable_buttons_ << ui->toolButtonZoomFit;
    response_enable_buttons_ << ui->toolButtonFocusLine;
    response_enable_buttons_ << ui->toolButtonRoiSelection;
    response_enable_buttons_ << ui->toolButtonSnapshot;

    connect(ui->toolButtonZoomIn, &QToolButton::clicked, this, &VideoToolBar::zoomInTriggered);
    connect(ui->toolButtonZoomOut, &QToolButton::clicked, this, &VideoToolBar::zoomOutTriggered);
    connect(ui->toolButtonZoomFull, &QToolButton::clicked, this, &VideoToolBar::zoomToOriginalTriggered);
    connect(ui->toolButtonZoomFit, &QToolButton::clicked, this, &VideoToolBar::zoomToFitTriggered);
    connect(ui->toolButtonFocusLine, &QToolButton::clicked, this, &VideoToolBar::focusLineTriggered);
    //connect(ui->toolButtonFullscreen, &QToolButton::clicked, this, &VideoToolBar::fullScreenTriggered);
    connect(ui->toolButtonRoiSelection, &QToolButton::clicked, this, &VideoToolBar::roiSelectionTriggered);
    connect(ui->toolButtonSnapshot, &QToolButton::clicked, this, &VideoToolBar::snapshotTriggered);

	for (auto e : response_enable_buttons_)
	{
		connect(e, &QToolButton::clicked, this, &VideoToolBar::buttonClicked);
	}
	connect(ui->toolButtonFullscreen, &QToolButton::clicked, this, &VideoToolBar::buttonClicked);
}

VideoToolBar::~VideoToolBar()
{
    delete ui;
}

void VideoToolBar::setEnabled(bool enabled)
{
    for (auto button_ptr : response_enable_buttons_)
    {
        button_ptr->setEnabled(enabled);
    }
}

bool VideoToolBar::isRoiSelectionEnabled() const
{
	return ui->toolButtonRoiSelection->isEnabled();
}

void VideoToolBar::setFocusLineChecked(bool bchecked)
{
	ui->toolButtonFocusLine->setChecked(bchecked);
}

void VideoToolBar::setRoiSelectionEnabled(bool benabled)
{
	ui->toolButtonRoiSelection->setEnabled(benabled);
}

void VideoToolBar::on_toolButtonFullscreen_clicked()
{
    emit fullScreenTriggered(true);
}
