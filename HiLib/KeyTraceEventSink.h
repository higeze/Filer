#pragma once
#include "msctf.h"
#include "Unknown.h"

class CKeyTraceEventSink : public CUnknown<ITfKeyTraceEventSink>
{
public:
	static DWORD TES_INVALID_COOKIE;
	static UINT WM_KEYTRACEDOWN;
	static UINT WM_KEYTRACEUP;
private:
	HWND m_hWnd;
	DWORD m_dwEditCookie;

public:
	CKeyTraceEventSink();
	void SetHwnd(HWND hWnd) { m_hWnd = hWnd; }
	// ITfTextEditSink
	STDMETHODIMP OnKeyTraceDown(WPARAM wParam, LPARAM lParam);
	STDMETHODIMP OnKeyTraceUp(WPARAM wParam, LPARAM lParam);

	HRESULT _Advise();
	HRESULT _Unadvise();
};
