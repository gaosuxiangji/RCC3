#include "splashwidget.h"
#include "ui_splashwidget.h"
#include <QFile>
#include <QTimer>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QCoreApplication>
#include <random>
#include <ctime>


SplashWidget::SplashWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SplashWidget)
{
    ui->setupUi(this);

    initUi();
}

SplashWidget::~SplashWidget()
{
    delete ui;
}

void SplashWidget::setProductName(const QString &name)
{
    ui->labelProductName->setText(name);
}

void SplashWidget::setProductVersion(const QString &version)
{
    ui->labelProductVersion->setText(version);
}

void SplashWidget::setCompanyName(const QString &name)
{
    ui->labelCompanyName->setText(name);
}

void SplashWidget::setDuration(int msecs)
{
    duration_ = msecs;
}

void SplashWidget::setSkipEnabled(bool enable)
{
    ui->pushButtonSkip->setEnabled(enable);
}

void SplashWidget::enableSkip()
{
    setSkipEnabled(true);
}

void SplashWidget::changeState(QMediaPlayer::State state)
{
    if (state == QMediaPlayer::State::PlayingState)
    {
		timer_ = new QTimer(this);
        connect(timer_, &QTimer::timeout, this, &SplashWidget::updateCountDown);
		timer_->start(1000);
    }
}

void SplashWidget::mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
//to do:mpp
    //if (status == QMediaPlayer::LoadedMedia)
    {
        player_->play();
    }
}

void SplashWidget::updateCountDown()
{
    duration_ -= 1000;
    if (duration_ <= 0)
    {
        skip();
    }

    ui->pushButtonSkip->setText(tr("Skip(%1s)").arg(duration_ / 1000));
}

void SplashWidget::initUi()
{
    // 无边框置顶
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    // “跳过”默认禁用
    ui->pushButtonSkip->setEnabled(false);

    // 载入样式
    QFile file(":/splash/qss/black.qss");
    if (file.open(QFile::ReadOnly))
    {
        QString style_sheet = QString::fromUtf8(file.readAll());
        file.close();

        setStyleSheet(style_sheet);
    }

    // 播放器
    player_ = new QMediaPlayer(this);
    connect(player_, &QMediaPlayer::stateChanged, this, &SplashWidget::changeState);
    connect(player_, &QMediaPlayer::mediaStatusChanged, this, &SplashWidget::mediaStatusChanged);
	player_->setMuted(true);//静音
    player_->setVideoOutput(ui->widgetVideo);

    QMediaPlaylist *play_list = new QMediaPlaylist;
    play_list->addMedia(QUrl(QString("qrc:/splash/avi/%1").arg(getRandomVideoFileName())));
    player_->setPlaylist(play_list);
}

QString SplashWidget::getRandomVideoFileName() const
{
    std::default_random_engine engine(time(0));
    std::uniform_int_distribution<int> u(1, 5);

    return QString("splash%2.avi").arg(u(engine), 2, 10, QChar('0'));
}

void SplashWidget::skip()
{
	if (timer_)
	{
		timer_->stop();
	}

	bskipped_ = true;
    emit skipped();
}

void SplashWidget::on_pushButtonSkip_clicked()
{
    skip();
}

bool SplashWidget::isSkipped() const
{
	return bskipped_;
}
