#pragma once
#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shlwapi.h>

template<class TRect> class CUnknown : public TRect
{
private:
	LONG m_refCount;
public:
	CUnknown():m_refCount(1){}
	virtual ~CUnknown(){}

	STDMETHODIMP_(ULONG) AddRef() override
	{
		::InterlockedIncrement( &m_refCount);
		return (ULONG)m_refCount;
	}
	STDMETHODIMP_(ULONG) Release() override
	{
		ULONG ret = (ULONG)::InterlockedDecrement(&m_refCount);
		if(ret == 0){
			delete this;
		}
		return (ULONG)m_refCount;
	}
	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject)
	{
		*ppvObject = NULL;

		if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, _uuidof(TRect))){
			*ppvObject = static_cast<TRect*>(this);
		}else{
			return E_NOINTERFACE;
		}

		AddRef();
		return S_OK;
	}
};