#include "keyboardcontrol.h"
#include <QDebug>
#include <QDir>

#ifdef _WIN32
#define ENABLE_USE_WINDOWS_SOFT_KEYBOARD
#endif

#ifdef ENABLE_USE_WINDOWS_SOFT_KEYBOARD
#include <qt_windows.h>
#pragma  comment (lib, "user32.lib")
#endif

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

static const wchar_t kKeyboardClassName[MAX_PATH] = { L"IPTip_Main_Window" };
void KeyBoardControl::OpenKeyBoard()
{
#ifdef ENABLE_USE_WINDOWS_SOFT_KEYBOARD
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	static QString keyboard_ptah;
	static wchar_t keyboard_path_wchar[MAX_PATH]{ 0 };
	if (keyboard_ptah.isEmpty())
	{
		keyboard_ptah = QDir::rootPath() + QString("Program Files\\Common Files\\Microsoft Shared\\ink\\tabtip.exe");
		keyboard_ptah.replace("/", "\\");
		keyboard_ptah.toWCharArray(keyboard_path_wchar);
		
		//static wchar_t osk[MAX_PATH] = { L"C:\\Windows\\System32\\osk.exe" };//系统屏幕键盘，开发电脑需要管理员权限才能正常打开
		SetEnvironmentVariable(L"__compat_layer", L"RunAsInvoker");//必须要设置，否则无法弹出或无法关闭，软件打开后仅需配置1次
	}

	if (!CreateProcess(nullptr, keyboard_path_wchar, nullptr
		, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
	{
		qDebug("OpenKeyBoard failed, error code = %u", GetLastError());
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return;
	}
#endif
}

void KeyBoardControl::CloseKeyBoard()
{
#ifdef ENABLE_USE_WINDOWS_SOFT_KEYBOARD
	HWND appWnd = FindWindow(kKeyboardClassName, nullptr);
	if (appWnd)
	{
		PostMessage(appWnd, WM_CLOSE, 0, 0);
	}
#endif
}

void KeyBoardControl::MoveKeyBoard(const QRect & rc)
{
#ifdef ENABLE_USE_WINDOWS_SOFT_KEYBOARD
	if (rc.isValid())
	{
		HWND appWnd = FindWindow(kKeyboardClassName, nullptr);
		if (appWnd)
		{
			MoveWindow(appWnd, rc.x(), rc.y(), rc.width(), rc.height(), true);
		}
	}
#endif
}
