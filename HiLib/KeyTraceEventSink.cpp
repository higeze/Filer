#include "KeyTraceEventSink.h"
#include "TSFManager.h"
#include "Debug.h"

DWORD CKeyTraceEventSink::TES_INVALID_COOKIE = ((DWORD)(-1));
UINT CKeyTraceEventSink::WM_KEYTRACEDOWN = ::RegisterWindowMessage(L"WM_KEYTRACEDOWN");
UINT CKeyTraceEventSink::WM_KEYTRACEUP = ::RegisterWindowMessage(L"WM_KEYTRACEUP");

CKeyTraceEventSink::CKeyTraceEventSink()
    :m_dwEditCookie(TES_INVALID_COOKIE){}

HRESULT CKeyTraceEventSink::_Advise()
{
    CComPtr<ITfSource> pSource;

    FAILED_THROW(CTSFManager::GetInstance()->GetThreadMgrPtr()->QueryInterface(IID_ITfSource, (void **)&pSource))
    FAILED_THROW(pSource->AdviseSink(IID_ITfKeyTraceEventSink, (ITfKeyTraceEventSink *)this, &m_dwEditCookie))

    return S_OK;
}

HRESULT CKeyTraceEventSink::_Unadvise()
{
    CComPtr<ITfSource> pSource;

    FAILED_THROW(CTSFManager::GetInstance()->GetThreadMgrPtr()->QueryInterface(IID_ITfSource, (void**)&pSource));
    FAILED_THROW(pSource->UnadviseSink(m_dwEditCookie));

    return S_OK;
}

STDMETHODIMP CKeyTraceEventSink::OnKeyTraceDown(WPARAM wParam, LPARAM lParam)
{
    ::SendMessage(m_hWnd, WM_KEYTRACEDOWN, wParam, lParam);
	return S_OK;
}

STDMETHODIMP CKeyTraceEventSink::OnKeyTraceUp(WPARAM wParam, LPARAM lParam)
{
    ::SendMessage(m_hWnd, WM_KEYTRACEUP, wParam, lParam);
    return S_OK;
}