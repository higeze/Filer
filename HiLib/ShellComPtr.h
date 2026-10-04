#pragma once
#include <atlbase.h>
#include <mutex>
#include <map>
#include "ShellThread.h"

template <class T>
class CShellComPtr
{
public:
    T* p;

public:
    //Constructors
    CShellComPtr() throw()
		:p(nullptr)
    {
        p = nullptr;
    }
    CShellComPtr(_Inout_opt_ T* lp) throw()
		:p(nullptr)
    {
        p = lp;
    }
    //Copy constructor
    CShellComPtr(const CShellComPtr& other) throw()
		:p(nullptr)
    {
		this->operator=(other);
    }
    //Copy Assignment operator
    CShellComPtr& operator=(const CShellComPtr& other) throw()
    {
        if (*this != other) {
            p = other.p;
            this->AddRef();
        }
        return *this;
    }
    // Move constructor
    CShellComPtr(CShellComPtr&& other) throw()
		:p(nullptr)
    {
		this->operator=(std::forward<CShellComPtr>(other));
    }
    // Move assignment
    CShellComPtr& operator=(CShellComPtr&& other) throw()
    {
        if (*this != other) {
            this->Attach(other.Detach());
        }
        return *this;
    }
    //Destructor
    ~CShellComPtr() throw()
    {
        Release();
    }
    //Operators
    operator T* () const throw()
    {
        return p;
    }
    T& operator*() const
    {
        ATLENSURE(p != NULL);
        return *p;
    }
    T** operator&() throw()
    {
        ATLASSERT(p == NULL);
        return &p;
    }
    //T* operator->() const throw()
    //{
    //    ATLASSERT(p != NULL);
    //    DEBUG_REGISTER
    //    if (IsMarshalled()) {
    //        auto a = 1;
    //    }
    //    return p;
    //}
public:
    template<typename T>
    T WaitForFutureWithMessagePump(std::future<T>&& fut) const
    {
        using namespace std::chrono_literals;
        while (fut.wait_for(0ms) != std::future_status::ready) {
            MSG msg;
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT) PostQuitMessage((int)msg.wParam);
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            std::this_thread::sleep_for(5ms);
        }
        return fut.get();
    }
 
    template <typename Func, typename... Args>
    auto Call(Func&& func, Args&&... args) const
        -> decltype((p->*func)(std::forward<Args>(args)...))
    {
		return std::move(CallAsync(std::forward<Func>(func), std::forward<Args>(args)...)).get();
    }

    template <typename Func, typename... Args>
    auto CallAsync(Func func, Args&&... args) const
        -> std::future<decltype((p->*func)(std::forward<Args>(args)...))>
    {
        return CShellThread::GetInstance()->RunInterface(std::forward<Func>(func), p, std::forward<Args>(args)...);
    }

    bool operator!() const throw()
    {
        return (p == NULL);
    }
    bool operator<(_In_opt_ T* pT) const throw()
    {
        return p < pT;
    }
    bool operator==(_In_opt_ T* pT) const throw()
    {
        return p == pT;
    }
    bool operator==(const CShellComPtr& other) const throw()
    {
        return p == other.p;
    }
    bool operator!=(const CShellComPtr& other) const throw()
    {
        return !(p == other);
    }

    //Functions
    void Swap(CShellComPtr& other)
    {
        T* pTemp = p;
        p = other.p;
        other.p = pTemp;
    }
    void Release() throw()
    {
        T* pTemp = p;
        if (pTemp)
        {
            p = nullptr;
            pTemp->Release();
        }
    }
    void AddRef()
    {
        if (p) {
            p->AddRef();
        }
    }
    void Attach(_In_opt_ T* pOther) throw()
    {
        this->Release();
        p = pOther;
    }
    T* Detach() throw()
    {
        T* pTemp = p;
        p = nullptr;
        return pTemp;
    }
    //_Check_return_ HRESULT CopyTo(_COM_Outptr_result_maybenull_ T** ppT) throw()
    //{
    //    ATLASSERT(ppT != NULL);
    //    if (ppT == NULL)
    //        return E_POINTER;
    //    *ppT = p;
    //    if (p)
    //        p->AddRef();
    //    return S_OK;
    //}
    //_Check_return_ HRESULT SetSite(_Inout_opt_ IUnknown* punkParent) throw()
    //{
    //    return AtlSetChildSite(p, punkParent);
    //}
    //_Check_return_ HRESULT Advise(
    //    _Inout_ IUnknown* pUnk,
    //    _In_ const IID& iid,
    //    _Out_ LPDWORD pdw) throw()
    //{
    //    return AtlAdvise(p, pUnk, iid, pdw);
    //}
    _Check_return_ HRESULT CoCreateInstance(
        _In_ REFCLSID rclsid,
        _Inout_opt_ LPUNKNOWN pUnkOuter = NULL,
        _In_ DWORD dwClsContext = CLSCTX_ALL) throw()
    {
        ATLASSERT(p == NULL);
        return ::CoCreateInstance(rclsid, pUnkOuter, dwClsContext, __uuidof(T), (void**)&p);
    }
    _Check_return_ HRESULT CoCreateInstance(
        _In_z_ LPCOLESTR szProgID,
        _Inout_opt_ LPUNKNOWN pUnkOuter = NULL,
        _In_ DWORD dwClsContext = CLSCTX_ALL) throw()
    {
        CLSID clsid;
        HRESULT hr = CLSIDFromProgID(szProgID, &clsid);
        ATLASSERT(p == NULL);
        if (SUCCEEDED(hr))
            hr = ::CoCreateInstance(clsid, pUnkOuter, dwClsContext, __uuidof(T), (void**)&p);
        return hr;
    }
    template <class Q>
    _Check_return_ HRESULT QueryInterface(_Outptr_ Q** pp) const throw()
    {
        ATLASSERT(pp != NULL);
        return p->QueryInterface(__uuidof(Q), (void**)pp);
    }
};
