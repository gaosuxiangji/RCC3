#pragma once

#ifdef WIN32

#include <Windows.h>

#define DumpEnable_

#if defined(DumpEnable_) && !defined(_DEBUG)
#define DumpEnable
#endif

#pragma comment(lib,"DbgHelp.lib")

//dump�ļ�����
extern LONG ApplicationCrashHandler(EXCEPTION_POINTERS *pException);

#endif