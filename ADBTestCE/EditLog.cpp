#include "EditLog.h"
#include "resource.h"
#include "ADBTestCeDlg.h"

CADBTestCEDlg *mainDialogPointer;
FILE* logFile;
void logcat(TCHAR* log)
{
	mainDialogPointer->logcat(log);
	logFile = _wfopen(L"\\CardC\\log.txt", L"a+");
	fputws(log, logFile);
	fputws(L"\r\n", logFile);
	fclose(logFile);
	NKDbgPrintfW(L"%s\r\n", log);
}

void setMainDialogPointer(CADBTestCEDlg *p) {
	mainDialogPointer = p;

}