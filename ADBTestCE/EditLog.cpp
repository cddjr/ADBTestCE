
#include "EditLog.h"
#include "resource.h"
CADBTestCEDlg *mainDialogPointer;
FILE* logFile;
void logcat(TCHAR* log)
{
	mainDialogPointer->logcat(log);
	logFile = _wfopen(L"\\StorageCard\\log.txt", L"a+");
	fputws(log, logFile);
	fputws(L"\r\n", logFile);
	fclose(logFile);
}

void setMainDialogPointer(CADBTestCEDlg *p) {
	mainDialogPointer = p;

}