#pragma once
#include <atlcom.h>
#include "getter_macro.h"

#if defined(_DEBUG)
#    pragma comment(lib, "comsuppwd.lib")
#else
#    pragma comment(lib, "comsuppw.lib")
#endif
#include <comdef.h>
#include <comip.h>
#include <netlistmgr.h>


class CNetworkMessanger
{
public:
	static UINT WM_CONNECTIVITYCHANGED;
	static UINT WM_NETWORKCONNECTIONPROPERTYCHANGED;
private:
	class EventSink;
	
	std::unique_ptr<EventSink> m_pEvent;

	DWORD m_cookie{0};
	DECLARE_LAZY_COMPTR_GETTER(INetworkListManager, NetworkListManager);
	DECLARE_LAZY_COMPTR_GETTER(IConnectionPointContainer, ConnectionPointContainer);
	DECLARE_LAZY_COMPTR_GETTER(IConnectionPoint, ConnectionPoint);
	DECLARE_LAZY_COMPTR_GETTER(INetworkListManagerEvents, NetworkListManagerEvents);
public:
	CNetworkMessanger(HWND hWnd);
	~CNetworkMessanger();
	void Start();
	void Stop();


};
