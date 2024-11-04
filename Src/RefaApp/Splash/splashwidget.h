#ifndef SPLASHWIDGET_H
#define SPLASHWIDGET_H

#include <QWidget>
#include <QMediaPlayer>

namespace Ui {
class SplashWidget;
}

class QTimer;

/**
 * @brief 启动页类
 */
class SplashWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SplashWidget(QWidget *parent = 0);
    ~SplashWidget();

    /**
     * @brief 设置产品名称
     * @param name 产品名称
     */
    void setProductName(const QString & name);

    /**
     * @brief 设置产品版本
     * @param version 产品版本
     */
    void setProductVersion(const QString & version);

    /**
     * @brief 设置公司名称
     * @param name 公司名称
     */
    void setCompanyName(const QString & name);

    /**
     * @brief 设置持续时间
     * @param msecs 持续时间，单位：毫秒
     */
    void setDuration(int msecs);

    /**
     * @brief 设置跳过是否使能
     * @param enable true-使能，false-禁用
     */
    void setSkipEnabled(bool enable);

	/**
	* @brief 是否已跳过
	* @return true-使能，false-禁用
	*/
	bool isSkipped() const;

signals:
    /**
     * @brief 已跳过
     */
    void skipped();

public slots:
    /**
     * @brief 跳过使能
     */
    void enableSkip();

private slots:
    void changeState(QMediaPlayer::State state);
    void mediaStatusChanged(QMediaPlayer::MediaStatus status);
    void updateCountDown();

    void on_pushButtonSkip_clicked();

private:
    /**
     * @brief 初始化界面
     */
    void initUi();

    /**
     * @brief 获取随机视频文件名
     * @return
     */
    QString getRandomVideoFileName() const;

    /**
     * @brief 跳过
     */
    void skip();

private:
    Ui::SplashWidget *ui;

	QMediaPlayer* player_{ nullptr };
	QTimer* timer_{ nullptr };

    int duration_{ 5000 }; // 持续时间，默认5秒

	bool bskipped_{ false };//是否已跳过
};

#endif // SPLASHWIDGET_H
