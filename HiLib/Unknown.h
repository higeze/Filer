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
	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject) override
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

template<class T1, class T2> class CUnknown2 : public T1, public T2
{
private:
	LONG m_refCount;
public:
	CUnknown2():m_refCount(1){}
	virtual ~CUnknown2(){}

	template<class U, class... UArgs>
	static void CreateInstance(T1 **pp, UArgs... args) {
		if (pp != NULL) {
			U *p = new U(args...);
			p->QueryInterface(IID_PPV_ARGS(pp));
		}
	}

	template<class U, class... UArgs>
	static void CreateInstance(T2 **pp, UArgs... args) {
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
	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject) override
	{
		*ppvObject = NULL;

		if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, _uuidof(T1))) {
			*ppvObject = static_cast<T1*>(this);
		} else if (IsEqualIID(riid, _uuidof(T2))){
			*ppvObject = static_cast<T2*>(this);
		} else {
			return E_NOINTERFACE;
		}

		AddRef();
		return S_OK;
	}
};