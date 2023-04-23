#pragma once
#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shlwapi.h>

template<class T> class CUnknown : public T
{
private:
	LONG m_refCount;
public:
	CUnknown():m_refCount(1){}
	virtual ~CUnknown(){}

	template<class U, class... UArgs>
	static void CreateInstance(T **pp, UArgs... args) {
		if (pp != NULL) {
			U *p = new U(args...);
			p->QueryInterface(IID_PPV_ARGS(pp));
		}
	}

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

		if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, _uuidof(T))){
			*ppvObject = static_cast<T*>(this);
		}else{
			return E_NOINTERFACE;
		}

		AddRef();
		return S_OK;
	}
};