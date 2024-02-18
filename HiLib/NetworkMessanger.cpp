#include "NetworkMessanger.h"
#include "Unknown.h"
#include "Debug.h"

UINT CNetworkMessanger::WM_CONNECTIVITYCHANGED = ::RegisterWindowMessageA("WM_CONNECTIVITYCHANGED");
UINT CNetworkMessanger::WM_NETWORKCONNECTIONPROPERTYCHANGED = ::RegisterWindowMessageA("WM_NETWORKCONNECTIONPROPERTYCHANGED");


class CNetworkMessanger::EventSink : public CUnknown<INetworkListManagerEvents>
{
private:
    HWND m_hWnd;
public:
    EventSink(HWND hWnd):m_hWnd(hWnd) {}

    HRESULT STDMETHODCALLTYPE ConnectivityChanged(NLM_CONNECTIVITY connectiveity)
    {
        ::PostMessageW(m_hWnd, WM_CONNECTIVITYCHANGED, static_cast<WPARAM>(connectiveity), NULL);
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE NetworkConnectionPropertyChanged(GUID connectionId, NLM_CONNECTION_PROPERTY_CHANGE propertychange) 
    { 
         ::PostMessage(m_hWnd, WM_NETWORKCONNECTIONPROPERTYCHANGED, static_cast<WPARAM>(propertychange), NULL);
        return S_OK;
    }
};

CNetworkMessanger::CNetworkMessanger(HWND hWnd)
    :m_pEvent(std::make_unique<EventSink>(hWnd))
{
    Start();
}

CNetworkMessanger::~CNetworkMessanger()
{
    Stop();
}

const CComPtr<INetworkListManager>& CNetworkMessanger::GetNetworkListManagerPtr() const
{
    if (!m_pNetworkListManager) {
        FAILED_THROW(m_pNetworkListManager.CoCreateInstance(CLSID_NetworkListManager, nullptr, CLSCTX_ALL));
    }
    return m_pNetworkListManager;
}

const CComPtr<IConnectionPointContainer>& CNetworkMessanger::GetConnectionPointContainerPtr() const
{
    if (!m_pConnectionPointContainer) {
        FAILED_THROW(GetNetworkListManagerPtr()->QueryInterface(IID_PPV_ARGS(&m_pConnectionPointContainer)));
    }
    return m_pConnectionPointContainer;
}

const CComPtr<IConnectionPoint>& CNetworkMessanger::GetConnectionPointPtr() const
{
    if (!m_pConnectionPoint) {
        FAILED_THROW(GetConnectionPointContainerPtr()->FindConnectionPoint(IID_INetworkListManagerEvents, &m_pConnectionPoint));
    }
    return m_pConnectionPoint;
}

const CComPtr<INetworkListManagerEvents>& CNetworkMessanger::GetNetworkListManagerEventsPtr() const
{
    if (!m_pNetworkListManagerEvents) {
        FAILED_THROW(m_pEvent->QueryInterface(IID_PPV_ARGS(&m_pNetworkListManagerEvents)));
    }
    return m_pNetworkListManagerEvents;
}


void CNetworkMessanger::Start()
{
    FAILED_THROW(GetConnectionPointPtr()->Advise(GetNetworkListManagerEventsPtr(), &m_cookie));
}

void CNetworkMessanger::Stop()
{
    FAILED_THROW(GetConnectionPointPtr()->Unadvise(m_cookie));
}


