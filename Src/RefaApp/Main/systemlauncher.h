#ifndef SPLASHCONTROLLER_H
#define SPLASHCONTROLLER_H

#include "Main/rccapp/csglobaldefine.h"

#include <QSharedPointer>

class SplashWidget;
class RefaMainWindow;
class FramelessWindow;
class MainInitThread;
class CSRccApp;

/**
 * @brief 系统启动类
 * @details 启动页显示->主初始化线程启动->主初始化线程完成->主界面初始化->启动页跳过使能->跳过/超时->主界面显示->启动页隐藏
 */
class SystemLauncher : public QObject
{
    Q_OBJECT

public:
    SystemLauncher();
	~SystemLauncher();

private slots:
    /**
     * @brief 初始化主界面
     */
	void initMainWindow();

    /**
     * @brief 显示主界面
     */
    void showMainWindow();

private:
	void initSDK();

	QSharedPointer<SplashWidget> splash_widget_ptr_;
#ifdef CSRCCAPP
	CSRccApp *main_window_ptr_{ nullptr };//主界面，由于会将其父窗口设为frameless_main_window_ptr_，所以不能使用智能指针
#else
	RefaMainWindow *main_window_ptr_{ nullptr };//主界面，由于会将其父窗口设为frameless_main_window_ptr_，所以不能使用智能指
#endif // CSRCCAPP

	
	QSharedPointer<FramelessWindow> frameless_main_window_ptr_;//无边框主界面
	QSharedPointer<MainInitThread> main_init_thread_ptr_;
	bool is_main_window_shown = false;
};

#endif // SPLASHCONTROLLER_H
