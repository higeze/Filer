#pragma once
#include <atlbase.h>
#include <mutex>
#include <map>
#include <thread>
#include "ThreadPool.h"

class CShellThread
{
private:
    template<typename T>
	friend class CThreadSafeComPtr;
    inline static CThreadPool Thread = CThreadPool(COINIT_APARTMENTTHREADED, 1);

public:
    template<class F, class... Args>
    inline static auto Run(const char* name, int&& priority, F&& f, Args&&... args)
        -> std::future<typename std::invoke_result_t<F, Args...>>
    {
        if(std::this_thread::get_id() == Thread.GetThreadId(0)) {
            using return_type = typename std::invoke_result_t<F, Args...>;

            // 同一スレッドなら同期実行して future を返す
            auto bound = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
            std::packaged_task<return_type()> task(std::move(bound));
            std::future<return_type> fut = task.get_future();
            task(); // 同期実行
            return fut;
        } else {
			return Thread.enqueue(name, std::forward<int>(priority), std::forward<F>(f), std::forward<Args>(args)...);
        }
    }

};

template <class T>
class CThreadSafeComPtr
{
private:
    mutable std::recursive_mutex m_mutex;
    T* p;

    //bool IsMarshalled()const
    //{
    //    IUnknown* p1 = p;      // original pointer
    //    IUnknown* p2 = nullptr;

    //    p->QueryInterface(IID_IUnknown, (void**)&p2);

    //    return (p1 != p2);
    //}

public:
    //Constructors
    CThreadSafeComPtr() throw()
		:p(nullptr)
    {
        p = nullptr;
    }
    CThreadSafeComPtr(_Inout_opt_ T* lp) throw()
		:p(nullptr)
    {
        p = lp;
    }
    //Copy constructor
    CThreadSafeComPtr(const CThreadSafeComPtr& other) throw()
		:p(nullptr)
    {
		this->operator=(other);
    }
    //Copy Assignment operator
    CThreadSafeComPtr& operator=(const CThreadSafeComPtr& other) throw()
    {
        if (*this != other) {
            std::scoped_lock lock(m_mutex, other.m_mutex);
            p = other.p;
            this->AddRef();
        }
        return *this;
    }
    // Move constructor
    CThreadSafeComPtr(CThreadSafeComPtr&& other) throw()
		:p(nullptr)
    {
		this->operator=(std::forward<CThreadSafeComPtr>(other));
    }
    // Move assignment
    CThreadSafeComPtr& operator=(CThreadSafeComPtr&& other) throw()
    {
        if (*this != other) {
            std::scoped_lock lock(m_mutex, other.m_mutex);
            this->Attach(other.Detach());
        }
        return *this;
    }
    //Destructor
    ~CThreadSafeComPtr() throw()
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
  //  template<typename _MemFun, typename... _Args>
  //  auto Call(_MemFun&& memfun, _Args&&... args) const
  //  {
  //      if(std::this_thread::get_id() == Thread.GetThreadId(0)) {
  //          return (p->*memfun)(std::forward<_Args>(args)...);
  //      } else {
  //          return Thread.enqueue(FILE_LINE_FUNC, 0, std::forward<_MemFun>(memfun), p, std::forward<_Args>(args)...).get();
		//}
  //  }

    //template <typename Func, typename... Args>
    //auto Call(Func func, Args&&... args)
    //    -> decltype((std::forward<T>(p).*func)(std::forward<Args>(args)...))
    //{
    //    if (std::this_thread::get_id() == Thread.GetThreadId(0)) {
    //        return (std::forward<T>(p).*func)(std::forward<Args>(args)...);
    //    } else {
    //        return Thread.enqueue(FILE_LINE_FUNC, 0, std::forward<Func>(func), p, std::forward<Args>(args)...).get();
    //    }
    //}

    //template <typename Func, typename... Args>
    //auto Call(Func func, Args&&... args)
    //    -> decltype((p->*func)(std::forward<Args>(args)...))
    //{
    //    if (std::this_thread::get_id() == Thread.GetThreadId(0)) {
    //        return (p->*func)(std::forward<Args>(args)...);
    //    } else {
    //        return Thread.enqueue(FILE_LINE_FUNC, 0, [this, func](Args&&... args) -> decltype((p->*func)(std::forward<Args>(args)...)) {
    //            return (p->*func)(std::forward<Args>(args)...);
    //        });
    //    }
    //}
    // 安定化のため std::bind で束ねて enqueue に渡す (C++14 互換)
    template <typename Func, typename... Args>
    auto Call(Func func, Args&&... args)
        -> decltype((p->*func)(std::forward<Args>(args)...))
    {
        //if (std::this_thread::get_id() == Thread.GetThreadId(0)) {
        //    return (p->*func)(std::forward<Args>(args)...);
        //} else {
            auto bound = std::bind(func, p, std::forward<Args>(args)...);
            return CShellThread::Run(FILE_LINE_FUNC, 0, std::move(bound)).get();
        //}
    }

    template <typename Func, typename... Args>
    auto Call(Func func, Args&&... args) const
        -> decltype((p->*func)(std::forward<Args>(args)...))
    {
        //if (std::this_thread::get_id() == Thread.GetThreadId(0)) {
        //    return (p->*func)(std::forward<Args>(args)...);
        //} else {
            auto bound = std::bind(func, p, std::forward<Args>(args)...);
            return CShellThread::Run(FILE_LINE_FUNC, 0, std::move(bound)).get();
        //}
    }

    //template <typename Func, typename... Args>
    //HRESULT Call(Func func, Args&&... args) const
    //    //-> decltype((p->*func)(std::forward<Args>(args)...))
    //{
    //    if (std::this_thread::get_id() == Thread.GetThreadId(0)) {
    //        return (p->*func)(std::forward<Args>(args)...);
    //    } else {
    //        return Thread.enqueue(FILE_LINE_FUNC, 0, std::forward<Func>(func), p, std::forward<Args>(args)...).get();
    //    }
    //}

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
    bool operator==(const CThreadSafeComPtr& other) const throw()
    {
        return p == other.p;
    }
    bool operator!=(const CThreadSafeComPtr& other) const throw()
    {
        return !(p == other);
    }
    //Functions
    void Swap(CThreadSafeComPtr& other)
    {
        std::scoped_lock lock(m_mutex, other.m_mutex);
        T* pTemp = p;
        p = other.p;
        other.p = pTemp;
    }
    void Release() throw()
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        T* pTemp = p;
        if (pTemp)
        {
            p = nullptr;
            pTemp->Release();
        }
    }
    void AddRef()
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if (p) {
            p->AddRef();
        }
    }
    void Attach(_In_opt_ T* pOther) throw()
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        this->Release();
        p = pOther;
    }
    T* Detach() throw()
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
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
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
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
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        ATLASSERT(pp != NULL);
        return p->QueryInterface(__uuidof(Q), (void**)pp);
    }

};

//// Template wrapper for thread-safe CComPtr
//template <typename T>
//class CThreadSafeComPtr
//{
//public:
//    CThreadSafeComPtr() = default;
//
//    // Copy constructor
//    CThreadSafeComPtr(const CThreadSafeComPtr& other)
//    {
//        std::lock_guard<std::mutex> lock(other.m_mutex);
//        m_ptr = other.m_ptr; // CComPtr copy (AddRef)
//    }
//
//    // Move constructor
//    CThreadSafeComPtr(CThreadSafeComPtr&& other) noexcept
//    {
//        std::lock_guard<std::mutex> lock(other.m_mutex);
//        m_ptr.Attach(other.m_ptr.Detach());
//    }
//
//    // Assignment operator
//    CThreadSafeComPtr& operator=(const CThreadSafeComPtr& other)
//    {
//        if (this != &other) {
//            std::scoped_lock lock(m_mutex, other.m_mutex);
//            m_ptr = other.m_ptr;
//        }
//        return *this;
//    }
//
//    // Move assignment
//    CThreadSafeComPtr& operator=(CThreadSafeComPtr&& other) noexcept
//    {
//        if (this != &other) {
//            std::scoped_lock lock(m_mutex, other.m_mutex);
//            m_ptr.Attach(other.m_ptr.Detach());
//        }
//        return *this;
//    }
//
//    // Assign from raw pointer
//    CThreadSafeComPtr& operator=(T* ptr)
//    {
//        std::lock_guard<std::mutex> lock(m_mutex);
//        m_ptr = ptr;
//        return *this;
//    }
//
//    // Thread-safe getter (returns a copy with AddRef)
//    CComPtr<T> Get() const
//    {
//        std::lock_guard<std::mutex> lock(m_mutex);
//        return m_ptr;
//    }
//
//    // Thread-safe setter
//    void Set(T* ptr)
//    {
//        std::lock_guard<std::mutex> lock(m_mutex);
//        m_ptr = ptr;
//    }
//
//    // Thread-safe release
//    void Release()
//    {
//        std::lock_guard<std::mutex> lock(m_mutex);
//        m_ptr.Release();
//    }
//
//    // Check if pointer is not null
//    bool IsValid() const
//    {
//        std::lock_guard<std::mutex> lock(m_mutex);
//        return m_ptr != nullptr;
//    }
//
//private:
//    mutable std::mutex m_mutex;
//    CComPtr<T> m_ptr;
//};


//#pragma once
//#include <atlbase.h>
//#include <mutex>
//#include <map>
//#include <thread>
//#include "ThreadPool.h"
//
////template <class T>
////class CShellComPtr
////{
////public:
////    inline static CThreadPool Thread = CThreadPool(COINIT_APARTMENTTHREADED, 1);
////    T* ptr;
////    std::thread::id ownerThreadId;
////
////    CShellComPtr()
////        : ptr(nullptr)
////        , ownerThreadId(std::this_thread::get_id())
////    {
////    }
////
////    explicit CShellComPtr(T* p)
////        : ptr(p)
////        , ownerThreadId(std::this_thread::get_id())
////    {
////    }
////
////    ~CShellComPtr()
////    {
////        if (ptr) {
////            AssertThread();
////            ptr->Release();
////        }
////    }
////
////    // コピー
////    CShellComPtr(const CShellComPtr& other)
////        : ptr(other.ptr)
////        , ownerThreadId(other.ownerThreadId)
////    {
////        if (ptr) ptr->AddRef();
////    }
////
////    // ムーブ
////    CShellComPtr(CShellComPtr&& other) noexcept
////        : ptr(other.ptr)
////        , ownerThreadId(other.ownerThreadId)
////    {
////        other.ptr = nullptr;
////    }
////
////    // 代入
////    CShellComPtr& operator=(const CShellComPtr& other)
////    {
////        if (this != &other) {
////            if (ptr) {
////                AssertThread();
////                ptr->Release();
////            }
////            ptr = other.ptr;
////            ownerThreadId = other.ownerThreadId;
////            if (ptr) ptr->AddRef();
////        }
////        return *this;
////    }
////
////    CShellComPtr& operator=(CShellComPtr&& other) noexcept
////    {
////        if (this != &other) {
////            if (ptr) {
////                AssertThread();
////                ptr->Release();
////            }
////            ptr = other.ptr;
////            ownerThreadId = other.ownerThreadId;
////            other.ptr = nullptr;
////        }
////        return *this;
////    }
////
////    CShellComPtr& operator=(T* p)
////    {
////        if (ptr != p) {
////            if (ptr) {
////                AssertThread();
////                ptr->Release();
////            }
////            ptr = p;
////            ownerThreadId = GetCurrentThreadId();
////            if (ptr) ptr->AddRef();
////        }
////        return *this;
////    }
////
////    // IUnknown ラップ
////    ULONG AddRef()
////    {
////        AssertThread();
////        return ptr ? ptr->AddRef() : 0;
////    }
////
////    ULONG Release()
////    {
////        AssertThread();
////        ULONG c = 0;
////        if (ptr) {
////            c = ptr->Release();
////            ptr = nullptr;
////        }
////        return c;
////    }
////
////    HRESULT QueryInterface(REFIID riid, void** ppv)
////    {
////        AssertThread();
////        HRESULT hr = ptr ? ptr->QueryInterface(riid, ppv) : E_POINTER;
////
////        if (SUCCEEDED(hr) && ppv && *ppv) {
////            // QueryInterface で返された COM オブジェクトも同じスレッドチェックを適用
////            // ただしここでは CheckedComPtr に包むのは呼び出し側の責任
////        }
////        return hr;
////    }
////
////    // メソッド呼び出しチェック
////    T* operator->() const
////    {
////        AssertThread();
////        return ptr;
////    }
////
////    operator T* () const
////    {
////        AssertThread();
////        return ptr;
////    }
////
////    // operator& : ケース3として ownerThreadId を更新
////    T** operator&()
////    {
////        AssertThread();  // 既存オブジェクトへの & は同一スレッドのみ許可
////        if (ptr) {
////            ptr->Release();
////            ptr = nullptr;
////        }
////        ownerThreadId = GetCurrentThreadId();
////        return &ptr;
////    }
////
////    T* Get() const
////    {
////        return ptr;
////    }
////
////    void Attach(T* p)
////    {
////        if (ptr) {
////            AssertThread();
////            ptr->Release();
////        }
////        ptr = p;
////        ownerThreadId = GetCurrentThreadId();
////    }
////
////    T* Detach()
////    {
////        AssertThread();
////        T* p = ptr;
////        ptr = nullptr;
////        return p;
////    }
////
////    bool IsNull() const
////    {
////        return ptr == nullptr;
////    }
////
////private:
////    void AssertThread() const
////    {
////        if (std::this_thread::get_id() != Thread.GetThreadId(0)) {
////            assert(!"Shell COM accessed from non-shell STA thread!");
////        }
////    }
////};
//
//
//#ifdef defined(_DEBUG) && defined(_TEST)
//#define DEBUG_DECLARE \
//        inline static std::mutex s_mutex = std::mutex(); \
//        inline static std::map<T*, std::pair<std::thread::id, long>> s_map = std::map<T*, std::pair<std::thread::id, long>>();
//#define DEBUG_REGISTER RegisterIfNot();
//#define DEBUG_INCR \
//        if (s_map.find(p) != s_map.end()) { \
//            if (std::this_thread::get_id() != s_map[p].first) { \
//                THROW_FILE_LINE_FUNC; \
//            } else { \
//                s_map[p].second++; \
//            } \
//        } else { \
//			DEBUG_REGISTER \
//        }
//#define DEBUG_DECR \
//        if (s_map.find(p) != s_map.end()) { \
//            if (std::this_thread::get_id() != s_map[p].first) { \
//                THROW_FILE_LINE_FUNC; \
//            } else { \
//                s_map[p].second--; \
//                if (s_map[p].second == 0) { \
//                    s_map.erase(p); \
//                } \
//            } \
//        } else { \
//        }
//
//#else
//#define DEBUG_REGISTER
//#define DEBUG_DECLARE
//#define DEBUG_INCR
//#define DEBUG_DECR
//#endif
//
//template <class T>
//class CThreadSafeComPtr
//{
//private:
//    DEBUG_DECLARE
//        inline static CThreadPool Thread = CThreadPool(COINIT_APARTMENTTHREADED, 1);
//    mutable std::recursive_mutex m_mutex;
//    T* p;
//
//    bool IsMarshalled()const
//    {
//        IUnknown* p1 = p;      // original pointer
//        IUnknown* p2 = nullptr;
//
//        p->QueryInterface(IID_IUnknown, (void**)&p2);
//
//        return (p1 != p2);
//    }
//
//public:
//    //Constructors
//    CThreadSafeComPtr() throw()
//        :p(nullptr)
//    {
//        p = nullptr;
//    }
//    CThreadSafeComPtr(_Inout_opt_ T* lp) throw()
//        :p(nullptr)
//    {
//        p = lp;
//    }
//    //Copy constructor
//    CThreadSafeComPtr(const CThreadSafeComPtr& other) throw()
//        :p(nullptr)
//    {
//        this->operator=(other);
//    }
//    //Copy Assignment operator
//    CThreadSafeComPtr& operator=(const CThreadSafeComPtr& other) throw()
//    {
//        if (*this != other) {
//            std::scoped_lock lock(m_mutex, other.m_mutex);
//            p = other.p;
//            this->AddRef();
//        }
//        return *this;
//    }
//    // Move constructor
//    CThreadSafeComPtr(CThreadSafeComPtr&& other) throw()
//        :p(nullptr)
//    {
//        this->operator=(std::forward<CThreadSafeComPtr>(other));
//    }
//    // Move assignment
//    CThreadSafeComPtr& operator=(CThreadSafeComPtr&& other) throw()
//    {
//        if (*this != other) {
//            std::scoped_lock lock(m_mutex, other.m_mutex);
//            this->Attach(other.Detach());
//        }
//        return *this;
//    }
//    //Destructor
//    ~CThreadSafeComPtr() throw()
//    {
//        Release();
//    }
//    //Operators
//    operator T* () const throw()
//    {
//        DEBUG_REGISTER
//            return p;
//    }
//    T& operator*() const
//    {
//        ATLENSURE(p != NULL);
//        DEBUG_REGISTER
//            return *p;
//    }
//    T** operator&() throw()
//    {
//        ATLASSERT(p == NULL);
//        return &p;
//    }
//    //T* operator->() const throw()
//    //{
//    //    ATLASSERT(p != NULL);
//    //    DEBUG_REGISTER
//    //    if (IsMarshalled()) {
//    //        auto a = 1;
//    //    }
//    //    return p;
//    //}
//  //  template<typename _MemFun, typename... _Args>
//  //  auto Call(_MemFun&& memfun, _Args&&... args) const
//  //  {
//  //      if(std::this_thread::get_id() == Thread.GetThreadId(0)) {
//  //          return (p->*memfun)(std::forward<_Args>(args)...);
//  //      } else {
//  //          return Thread.enqueue(FILE_LINE_FUNC, 0, std::forward<_MemFun>(memfun), p, std::forward<_Args>(args)...).get();
//        //}
//  //  }
//
//    //template <typename Func, typename... Args>
//    //auto Call(Func func, Args&&... args)
//    //    -> decltype((std::forward<T>(p).*func)(std::forward<Args>(args)...))
//    //{
//    //    if (std::this_thread::get_id() == Thread.GetThreadId(0)) {
//    //        return (std::forward<T>(p).*func)(std::forward<Args>(args)...);
//    //    } else {
//    //        return Thread.enqueue(FILE_LINE_FUNC, 0, std::forward<Func>(func), p, std::forward<Args>(args)...).get();
//    //    }
//    //}
//
//    template <typename Func, typename... Args>
//    HRESULT Call(Func func, Args&&... args)
//        //-> decltype((p->*func)(std::forward<Args>(args)...))
//    {
//        if (std::this_thread::get_id() == Thread.GetThreadId(0)) {
//            return (p->*func)(std::forward<Args>(args)...);
//        } else {
//            return Thread.enqueue(FILE_LINE_FUNC, 0, std::forward<Func>(func), p, std::forward<Args>(args)...).get();
//        }
//    }
//
//    template <typename Func, typename... Args>
//    HRESULT Call(Func func, Args&&... args) const
//        //-> decltype((p->*func)(std::forward<Args>(args)...))
//    {
//        if (std::this_thread::get_id() == Thread.GetThreadId(0)) {
//            return (p->*func)(std::forward<Args>(args)...);
//        } else {
//            return Thread.enqueue(FILE_LINE_FUNC, 0, std::forward<Func>(func), p, std::forward<Args>(args)...).get();
//        }
//    }
//
//    bool operator!() const throw()
//    {
//        DEBUG_REGISTER
//            return (p == NULL);
//    }
//    bool operator<(_In_opt_ T* pT) const throw()
//    {
//        DEBUG_REGISTER
//            return p < pT;
//    }
//    bool operator==(_In_opt_ T* pT) const throw()
//    {
//        DEBUG_REGISTER
//            return p == pT;
//    }
//    bool operator==(const CThreadSafeComPtr& other) const throw()
//    {
//        DEBUG_REGISTER
//            return p == other.p;
//    }
//    bool operator!=(const CThreadSafeComPtr& other) const throw()
//    {
//        DEBUG_REGISTER
//            return !(p == other);
//    }
//    //Functions
//    void Swap(CThreadSafeComPtr& other)
//    {
//        std::scoped_lock lock(m_mutex, other.m_mutex);
//        T* pTemp = p;
//        p = other.p;
//        other.p = pTemp;
//    }
//    void Release() throw()
//    {
//        std::lock_guard<std::recursive_mutex> lock(m_mutex);
//        T* pTemp = p;
//        if (pTemp)
//        {
//            p = nullptr;
//            pTemp->Release();
//            DEBUG_DECR
//        }
//    }
//    void AddRef()
//    {
//        std::lock_guard<std::recursive_mutex> lock(m_mutex);
//        if (p) {
//            p->AddRef();
//            DEBUG_INCR
//        }
//    }
//    void Attach(_In_opt_ T* pOther) throw()
//    {
//        std::lock_guard<std::recursive_mutex> lock(m_mutex);
//        this->Release();
//        p = pOther;
//    }
//    T* Detach() throw()
//    {
//        std::lock_guard<std::recursive_mutex> lock(m_mutex);
//        T* pTemp = p;
//        p = nullptr;
//        return pTemp;
//    }
//
//#if defined(_DEBUG) && defined(_TEST)
//    void RegisterIfNot() const
//    {
//
//        std::lock_guard<std::recursive_mutex> lock(m_mutex);
//        if (p) {
//            if (s_map.find(p) == s_map.end()) {
//                s_map.emplace(p, std::make_pair(std::this_thread::get_id(), 1));
//            }
//        }
//    }
//#endif
//
//    //_Check_return_ HRESULT CopyTo(_COM_Outptr_result_maybenull_ T** ppT) throw()
//    //{
//    //    ATLASSERT(ppT != NULL);
//    //    if (ppT == NULL)
//    //        return E_POINTER;
//    //    *ppT = p;
//    //    if (p)
//    //        p->AddRef();
//    //    return S_OK;
//    //}
//    //_Check_return_ HRESULT SetSite(_Inout_opt_ IUnknown* punkParent) throw()
//    //{
//    //    return AtlSetChildSite(p, punkParent);
//    //}
//    //_Check_return_ HRESULT Advise(
//    //    _Inout_ IUnknown* pUnk,
//    //    _In_ const IID& iid,
//    //    _Out_ LPDWORD pdw) throw()
//    //{
//    //    return AtlAdvise(p, pUnk, iid, pdw);
//    //}
//    _Check_return_ HRESULT CoCreateInstance(
//        _In_ REFCLSID rclsid,
//        _Inout_opt_ LPUNKNOWN pUnkOuter = NULL,
//        _In_ DWORD dwClsContext = CLSCTX_ALL) throw()
//    {
//        ATLASSERT(p == NULL);
//        return ::CoCreateInstance(rclsid, pUnkOuter, dwClsContext, __uuidof(T), (void**)&p);
//    }
//    _Check_return_ HRESULT CoCreateInstance(
//        _In_z_ LPCOLESTR szProgID,
//        _Inout_opt_ LPUNKNOWN pUnkOuter = NULL,
//        _In_ DWORD dwClsContext = CLSCTX_ALL) throw()
//    {
//        std::lock_guard<std::recursive_mutex> lock(m_mutex);
//        CLSID clsid;
//        HRESULT hr = CLSIDFromProgID(szProgID, &clsid);
//        ATLASSERT(p == NULL);
//        if (SUCCEEDED(hr))
//            hr = ::CoCreateInstance(clsid, pUnkOuter, dwClsContext, __uuidof(T), (void**)&p);
//        return hr;
//    }
//    template <class Q>
//    _Check_return_ HRESULT QueryInterface(_Outptr_ Q** pp) const throw()
//    {
//        std::lock_guard<std::recursive_mutex> lock(m_mutex);
//        ATLASSERT(pp != NULL);
//        return p->QueryInterface(__uuidof(Q), (void**)pp);
//    }
//
//};
//
////// Template wrapper for thread-safe CComPtr
////template <typename T>
////class CThreadSafeComPtr
////{
////public:
////    CThreadSafeComPtr() = default;
////
////    // Copy constructor
////    CThreadSafeComPtr(const CThreadSafeComPtr& other)
////    {
////        std::lock_guard<std::mutex> lock(other.m_mutex);
////        m_ptr = other.m_ptr; // CComPtr copy (AddRef)
////    }
////
////    // Move constructor
////    CThreadSafeComPtr(CThreadSafeComPtr&& other) noexcept
////    {
////        std::lock_guard<std::mutex> lock(other.m_mutex);
////        m_ptr.Attach(other.m_ptr.Detach());
////    }
////
////    // Assignment operator
////    CThreadSafeComPtr& operator=(const CThreadSafeComPtr& other)
////    {
////        if (this != &other) {
////            std::scoped_lock lock(m_mutex, other.m_mutex);
////            m_ptr = other.m_ptr;
////        }
////        return *this;
////    }
////
////    // Move assignment
////    CThreadSafeComPtr& operator=(CThreadSafeComPtr&& other) noexcept
////    {
////        if (this != &other) {
////            std::scoped_lock lock(m_mutex, other.m_mutex);
////            m_ptr.Attach(other.m_ptr.Detach());
////        }
////        return *this;
////    }
////
////    // Assign from raw pointer
////    CThreadSafeComPtr& operator=(T* ptr)
////    {
////        std::lock_guard<std::mutex> lock(m_mutex);
////        m_ptr = ptr;
////        return *this;
////    }
////
////    // Thread-safe getter (returns a copy with AddRef)
////    CComPtr<T> Get() const
////    {
////        std::lock_guard<std::mutex> lock(m_mutex);
////        return m_ptr;
////    }
////
////    // Thread-safe setter
////    void Set(T* ptr)
////    {
////        std::lock_guard<std::mutex> lock(m_mutex);
////        m_ptr = ptr;
////    }
////
////    // Thread-safe release
////    void Release()
////    {
////        std::lock_guard<std::mutex> lock(m_mutex);
////        m_ptr.Release();
////    }
////
////    // Check if pointer is not null
////    bool IsValid() const
////    {
////        std::lock_guard<std::mutex> lock(m_mutex);
////        return m_ptr != nullptr;
////    }
////
////private:
////    mutable std::mutex m_mutex;
////    CComPtr<T> m_ptr;
////};
