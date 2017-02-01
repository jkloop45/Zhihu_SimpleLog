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

BOOL ExistLog(_Out_ LogContent& LogContent_)
{
	//Initialize
	HANDLE hReadyEvent = ::CreateEventW(NULL, FALSE, FALSE, L"EVENT_LOG_READY");
	if (hReadyEvent == NULL)
		return FALSE;

	HANDLE hBufferEvent = ::CreateEventW(NULL, FALSE, FALSE, L"EVENT_LOG_BUFFER");
	if (hBufferEvent == NULL)
		return FALSE;

	HANDLE hFileMap = ::CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_EXECUTE_READWRITE, NULL, sizeof(LogContent), L"FILEMAP_LOG_CONTENT");
	if (hFileMap == NULL)
		return FALSE;

	LogContent* pLogContent = reinterpret_cast<LogContent*>(::MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, NULL, NULL, NULL));
	if (pLogContent == nullptr)
		return FALSE;

	// Notice Client
	::SetEvent(hReadyEvent);

	// Wait for Client Fill Buffer
	if (::WaitForSingleObject(hBufferEvent, 5 * 1000) == WAIT_TIMEOUT)
		return FALSE; // clear handle

	LogContent_ = *pLogContent;
	//CloseHandle() ¡­¡­¡­¡­¡­¡­
	return TRUE;
}

int main()
{
	if (OpenMutexW(MUTEX_ALL_ACCESS,FALSE, L"MUTEX_LOG_EXIST_SERVER"))
	{
		std::wcout << L"Exist Other Log Server" << std::endl;
		return 0;
	}
	::CreateMutexW(NULL, FALSE, L"MUTEX_LOG_EXIST_SERVER");

	LogContent LogContent_;
	while (true)
	{
		if (ExistLog(LogContent_))
		{
			std::wcout << L"LogLevel:" << LogContent_.emLogLevel << L", Content:" << LogContent_.wszLogText << std::endl;
			continue;
		}
		::Sleep(50);
	}

	return 0;
}