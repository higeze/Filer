#pragma once
#include <atlbase.h>
#include <mutex>
#include <map>
#include <thread>
#include "ThreadPool.h"

class CShellThread
{
private:
    std::thread m_thread;
    DWORD m_threadId;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::queue<std::function<void()>> m_tasks;
    bool m_stop;
public:
    CShellThread();

    ~CShellThread();

    template<class F, class... Args>
    inline auto Run(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result_t<F, Args...>>
    {
        using return_type = typename std::invoke_result_t<F, Args...>;
        if (::GetCurrentThreadId() == m_threadId) {
            auto bound = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
            std::packaged_task<return_type()> task(std::move(bound));
            std::future<return_type> fut = task.get_future();
            task();
            return fut;
        } else {
            auto bound = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
            std::packaged_task<return_type()> task(std::move(bound));
            return enqueue(std::move(task));
        }
    }

    template<class F, class T, class... Args>
    inline auto RunInterface(F&& f, T* p, Args&&... args)
        -> std::future<typename std::invoke_result_t<F, T*, Args...>>
    {
        using return_type = typename std::invoke_result_t<F, T*, Args...>;
        if (::GetCurrentThreadId() == m_threadId) {
            // 同一スレッドなら同期実行して future を返す
            auto bound = std::bind(std::forward<F>(f), p, std::forward<Args>(args)...);
            std::packaged_task<return_type()> task(std::move(bound));
            std::future<return_type> fut = task.get_future();
            task(); // 同期実行
            return fut;
        } else {
            //auto bound = std::bind(std::forward<F>(f), p, std::forward<Args>(args)...);
            //std::packaged_task<return_type()> task(std::move(bound));
            return enqueue(std::forward<F>(f), p, std::forward<Args>(args)...);
            //IStream* pStream = nullptr;
            //HRESULT hr = ::CoMarshalInterThreadInterfaceInStream(__uuidof(T), p, &pStream);
            //if (SUCCEEDED(hr) && pStream) {

            //    // capture arguments by value in a shared tuple to ensure lifetime
            //    using args_tuple_type = std::tuple<typename std::decay<Args>::type...>;
            //    auto argsPack = std::make_shared<args_tuple_type>(std::forward<Args>(args)...);

            //    // task 内の主要部分（変更点：HRESULT チェック、CComPtr による管理、エラー時に例外)
            //    auto task = [pStream, f, argsPack]() -> return_type {
            //        T* rawIface = nullptr;
            //        HRESULT hr = CoGetInterfaceAndReleaseStream(pStream, __uuidof(T), reinterpret_cast<LPVOID*>(&rawIface));
            //        if (FAILED(hr) || rawIface == nullptr) {
            //            // 明示的に失敗させる（呼び出し側でログ/解析）
            //            throw std::runtime_error("CoGetInterfaceAndReleaseStream failed");
            //        }
            //        CComPtr<T> sp(rawIface); // RAII

            //        auto invoker = [sp, f](auto&&... unpacked) -> return_type {
            //            // f がメンバポインタか関数オブジェクトかにより呼び方が変わる点に注意：
            //            // - メンバ関数ポインタ: (sp.p->*f)(...)
            //            // - 関数オブジェクト: std::invoke(f, sp.p, ...)
            //            if constexpr (std::is_member_function_pointer<typename std::decay<F>::type>::value) {
            //                return (sp.p->*f)(std::forward<decltype(unpacked)>(unpacked)...);
            //            } else {
            //                return std::invoke(f, sp.p, std::forward<decltype(unpacked)>(unpacked)...);
            //            }
            //            };

            //        return std::apply(invoker, *argsPack);
            //        };
            //    return enqueue(std::move(task));
            //}else {
            //    auto a = 1;
            //}
        }
    }

    void ThreadProc()
    {
        // STA 初期化
        HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        // もし失敗しても続行してタスク内でコールを試みるが注意ログ等を入れてください

        // スレッドID を格納
        m_threadId = ::GetCurrentThreadId();

        // 最小限のメッセージループとタスク処理
        MSG msg;
        bool running = true;
        while (running) {
            // まずタスクを実行する
            std::function<void()> task;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (!m_tasks.empty()) {
                    task = std::move(m_tasks.front());
                    m_tasks.pop();
                }
            }
            if (task) {
                try {
                    task();
                } catch (...) {
                    // 例外は呼び出し元 future に伝搬されるためここでは swallow でも良い
                }
                continue; // すぐ次のタスクをチェック
            }

            // タスクが無ければメッセージ待ち（メッセージが来たら DispatchMessage してから再チェック）
            DWORD res = ::MsgWaitForMultipleObjects(0, nullptr, FALSE, INFINITE, QS_ALLINPUT);
            if (res == WAIT_OBJECT_0) {
                // messages available
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                    if (msg.message == WM_QUIT) {
                        running = false;
                        break;
                    }
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            } else {
                // timeout or other - ループを続ける
            }
        }

        CoUninitialize();
    }

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result_t<F, Args...>>
    {
        using return_type = typename std::invoke_result_t<F, Args...>;
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> fut = task->get_future();
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_stop) {
                std::promise<return_type> p;
                p.set_exception(std::make_exception_ptr(std::runtime_error(FILE_LINE_FUNC)));
                return p.get_future();
            }
            m_tasks.emplace([task]() { (*task)(); });
        }
        // wake STA thread
        ::PostThreadMessage(m_threadId, WM_APP, 0, 0);
        return fut;
    }

    //// IShellFolder など COM インターフェイスをマーシャリングして STA スレッドで実行するヘルパ
    //// func: (T*) -> R を実行する関数オブジェクト
    //template<typename TInterface, typename R>
    //std::future<R> InvokeWithMarshaledInterface(TInterface* pInterface, std::function<R(TInterface*)> func)
    //{
    //    if (!pInterface) {
    //        std::promise<R> p;
    //        p.set_exception(std::make_exception_ptr(std::invalid_argument("null interface")));
    //        return p.get_future();
    //    }

    //    // マーシャリングストリームを作る
    //    IStream* pStream = nullptr;
    //    HRESULT hr = CoMarshalInterThreadInterfaceInStream(__uuidof(TInterface), pInterface, &pStream);
    //    if (FAILED(hr) || !pStream) {
    //        std::promise<R> p;
    //        p.set_exception(std::make_exception_ptr(std::runtime_error("CoMarshalInterThreadInterfaceInStream failed")));
    //        return p.get_future();
    //    }

    //    // ストリームを STA スレッドに渡して復元し、func を呼ぶ
    //    auto call = [pStream, func]() -> R {
    //        // STA スレッド側で CoGetInterfaceAndReleaseStream して TInterface* を得る
    //        TInterface* p = nullptr;
    //        HRESULT hr2 = CoGetInterfaceAndReleaseStream(pStream, __uuidof(TInterface), reinterpret_cast<void**>(&p));
    //        if (FAILED(hr2) || !p) {
    //            throw std::runtime_error("CoGetInterfaceAndReleaseStream failed");
    //        }
    //        // CComPtr で安全に管理
    //        CComPtr<TInterface> sp(p);
    //        return func(sp);
    //        };

    //    return Invoke(call);
    //}

    // 停止（デストラクタで呼ばれる）
    void Stop();

    static CShellThread* GetInstance();
};

//class CShellThread
//{
//private:
//    template<typename T>
//	friend class CThreadSafeComPtr;
//    inline static CShellThread Thread = CShellThread();
//
//public:
//    template<class F, class... Args>
//    inline static auto Run(const char* name, int&& priority, F&& f, Args&&... args)
//        -> std::future<typename std::invoke_result_t<F, Args...>>
//    {
//        if(std::this_thread::get_id() == Thread.GetThreadId(0)) {
//            using return_type = typename std::invoke_result_t<F, Args...>;
//
//// 同一スレッドなら同期実行して future を返す
//            auto bound = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
//            std::packaged_task<return_type()> task(std::move(bound));
//            std::future<return_type> fut = task.get_future();
//            task(); // 同期実行
//            return fut;
//        } else {
//            ::OutputDebugStringW(std::to_wstring(Thread.GetRunnningTaskCount()).c_str());
//			return Thread.enqueue(name, std::forward<int>(priority), std::forward<F>(f), std::forward<Args>(args)...);
//        }
//    }
//
//};#pragma once
