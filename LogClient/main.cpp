#include <Windows.h>
#include <iostream>

enum em_Log_Level
{
	em_Log_Level_Debug,
	em_Log_Level_Warning,
	em_Log_Level_Custome,
	em_Log_Level_Exception
};

struct LogContent
{
	// Log Level
	em_Log_Level emLogLevel;
	WCHAR		 wszLogText[1024];

	// More Log Msg
	// Method Name
	// File Name
	// Code Line
	// Client Name(difficult Client)
};

BOOL ExistLogServer()
{
	HANDLE hMutex = ::OpenMutexW(MUTEX_ALL_ACCESS, FALSE, L"MUTEX_LOG_EXIST_SERVER");
	if (hMutex == NULL)
		return FALSE;

	::CloseHandle(hMutex);
	hMutex = NULL;
	return TRUE;
}

BOOL PrintLogToServer(_In_ CONST LogContent& LogContent_)
{
	if (!ExistLogServer())
		return FALSE;

	HANDLE hReadyEvent = ::OpenEventW(EVENT_ALL_ACCESS, FALSE, L"EVENT_LOG_READY");
	if (hReadyEvent == NULL)
		return FALSE;

	HANDLE hBufferEvent = ::OpenEventW(EVENT_ALL_ACCESS, FALSE, L"EVENT_LOG_BUFFER");
	if (hBufferEvent == NULL)
		return FALSE;

	HANDLE hFileMap = ::OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, L"FILEMAP_LOG_CONTENT");
	if (hFileMap == NULL)
		return FALSE;

	LogContent* pLogContent = reinterpret_cast<LogContent*>(::MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, NULL, NULL, NULL));
	if (pLogContent == nullptr)
		return FALSE;

	if (::WaitForSingleObject(hReadyEvent, 3000) == WAIT_TIMEOUT)
		return FALSE;

	*pLogContent = LogContent_;
	::SetEvent(hBufferEvent);
	
	::CloseHandle(hReadyEvent);
	hReadyEvent = NULL;

	::CloseHandle(hBufferEvent);
	hBufferEvent = NULL;

	::UnmapViewOfFile(pLogContent);
	pLogContent = nullptr;

	::CloseHandle(hFileMap);
	hFileMap = NULL;
}

BOOL PrintLog(_In_ em_Log_Level LogLevel, _In_ LPCWSTR pwszFormat, ...)
{
	va_list		args;
	WCHAR		szBuff[1024];
	va_start(args, pwszFormat);
	_vsnwprintf_s(szBuff, _countof(szBuff) - 1, _TRUNCATE, pwszFormat, args);
	va_end(args);

	LogContent LogContent_;
	LogContent_.emLogLevel = LogLevel;
	wcscpy_s(LogContent_.wszLogText, _countof(LogContent_.wszLogText) - 1, szBuff);

	return PrintLogToServer(LogContent_);
}

int main()
{
	while (true)
	{
		PrintLog(em_Log_Level::em_Log_Level_Custome, L"i = 3");
		::Sleep(1000);
	}
	return 0;
}