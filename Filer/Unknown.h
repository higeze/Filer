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
};