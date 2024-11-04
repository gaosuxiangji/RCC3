#ifndef PLAYERTITLEBAR_H
#define PLAYERTITLEBAR_H

#include <QWidget>

namespace Ui {
class PlayerTitleBar;
}

/**
*@brief 播放器标题栏
*@attention 仅在全屏时显示 
**/
class PlayerTitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit PlayerTitleBar(QWidget *parent = 0);
    ~PlayerTitleBar();

public slots:
	/**
	*@brief 设置标题名称
	*@param [in] : text : const QString &，标题
	**/
	void setTitle(const QString &text);

	/**
	*@brief 设置图标
	*@param [in] : ico : const QIcon &，图标
	**/
	void setIcon(const QIcon &ico);

	/**
	*@brief 设置进入全屏显示状态槽函数
	*@param [in] : benabled : bool，true-使能，false-不使能
	**/
	void setFullScreen(bool benabled);

private slots:
	/**
	*@brief 还原按钮相应槽函数
	**/
	void on_restoreButton_clicked();

signals:
	/**
	*@brief 全屏信号
	*@param [in] : benabled : bool，true-进入全屏，false-退出全屏
	**/
	void fullScreen(bool benable);

private:
    Ui::PlayerTitleBar *ui;
};

#endif // PLAYERTITLEBAR_H
