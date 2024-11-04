#ifdef WIN32

#include "CrashDumpInfo.h"
#include <QString>
#include <QDateTime>
#include <QStandardPaths>
#include <DbgHelp.h>
#include <experimental/filesystem>

LONG ApplicationCrashHandler(EXCEPTION_POINTERS *pException)
{
#ifndef DumpEnable
	return EXCEPTION_EXECUTE_HANDLER;
#endif // DumpEnable

	QString dump_file_path = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/AgileDevice/REFA/Dump";
	std::string dump_file_path_str = dump_file_path.toLocal8Bit().data();
	
	//创建父文件夹
	if (!std::experimental::filesystem::exists(dump_file_path_str))
	{
		if (!std::experimental::filesystem::create_directories(dump_file_path_str))
		{
			return EXCEPTION_EXECUTE_HANDLER;
		}
	}

	//创建Dump文件
	HANDLE hDumpFile = CreateFile((dump_file_path_str.c_str() + QDateTime::currentDateTime().toString(QStringLiteral("/yyyy-MM-dd_HH.mm.ss.zzz")) + ".dmp").toStdWString().c_str(),
		GENERIC_WRITE, 
		0, 
		NULL, 
		CREATE_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL);

	if (INVALID_HANDLE_VALUE==hDumpFile)
	{
		return EXCEPTION_EXECUTE_HANDLER;
	}

	//Dump信息
	MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
	dumpInfo.ExceptionPointers = pException;
	dumpInfo.ThreadId = GetCurrentThreadId();
	dumpInfo.ClientPointers = TRUE;

	//写入Dump文件内容
	if (!MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL))
	{
		CloseHandle(hDumpFile);
		return EXCEPTION_EXECUTE_HANDLER;
	}

	if (!CloseHandle(hDumpFile))
	{
		return EXCEPTION_EXECUTE_HANDLER;
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

#endif