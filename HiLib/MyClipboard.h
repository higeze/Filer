#pragma once
#include <Windows.h>

class CClipboard
{
public:
	CClipboard(){};
	virtual ~CClipboard(){Close();}

	BOOL Open(HWND hWnd = NULL){return ::OpenClipboard(hWnd);}
	BOOL Close(){return ::CloseClipboard();}
	BOOL Empty(){return ::EmptyClipboard();}
	HANDLE SetData(UINT uFormat, HANDLE hMem){return ::SetClipboardData(uFormat, hMem);}
	HANDLE GetData(UINT uFormat){return ::GetClipboardData(uFormat);}
	BOOL IsClipboardFormatAvailable(UINT format){return ::IsClipboardFormatAvailable(format);}

	HANDLE SetDdb(HBITMAP hDDB);
	HANDLE SetImageFromDdb(LPCWSTR mimetype, LPCWSTR format, HBITMAP hDDB);
	HANDLE SetJpegFromDdb(HBITMAP hDDB);
	HANDLE SetPngFromDdb(HBITMAP hDDB);
	HANDLE SetGifFromDdb(HBITMAP hDDB);
	HANDLE SetTiffFromDdb(HBITMAP hDDB);
};