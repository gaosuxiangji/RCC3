#include "recoveringdatawidget.h"
#include "ui_recoveringdatawidget.h"

#include <QCloseEvent>

#include "Common/UIExplorer/uiexplorer.h"
#include "Common/UIUtils/uiutils.h"

recoveringdatawidget::recoveringdatawidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::recoveringdatawidget)
{
    ui->setupUi(this);

	initUI();
}

recoveringdatawidget::~recoveringdatawidget()
{
    delete ui;
}

int recoveringdatawidget::ShowProgressDlg()
{
	cur_progress_ = 0;
	ui->progressBar->setRange(0, 100);
	ui->progressBar->setValue(0);
	ui->labelProgress->setText(QString("%1%").arg(cur_progress_));
	ui->labelTip->setText(tr("Data recovery in progress, please wait..."));

	timer->start(50);

	return exec();
}

void recoveringdatawidget::closeEvent(QCloseEvent *event)
{
	setResult(m_recover_success);
	timer->stop();
	event->accept();
}

void recoveringdatawidget::onProgressChanged(int progress)
{
	cur_progress_ = progress;
}

void recoveringdatawidget::onSearchFinished()
{
	m_recover_success = true;
	close();
}

void recoveringdatawidget::onTimer()
{
	//手动刷新进度条
	if (cur_progress_ < 100 && cur_progress_ >= 0)
	{
		ui->progressBar->setValue(cur_progress_);
		ui->labelProgress->setText(QString("%1%").arg(cur_progress_));
	}
	else {
		onSearchFinished();
	}
}

void recoveringdatawidget::initUI()
{
	// 设置标题
	setWindowTitle(UIExplorer::instance().getProductName());

	// 去除问号按钮
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	//进度条刷新
	timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &recoveringdatawidget::onTimer);
}