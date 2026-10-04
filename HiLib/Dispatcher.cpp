#include "Dispatcher.h"
#include "D2DWWindow.h"
#include "UniqueIDFactory.h"


UINT CDispatcher::WM_DISPATCHER = ::RegisterWindowMessage(L"WM_DISPATCHER");


CDispatcher::CDispatcher()
	:m_pIDFactory(std::make_unique<CUniqueIDFactory>()){}

CDispatcher::~CDispatcher() = default;

void CDispatcher::Init()
{
	m_dwThreadId = ::GetCurrentThreadId();
}

void CDispatcher::PostInvoke(std::function<void()>&& fun)
{
	int id = 0;
	{
		std::lock_guard<std::mutex> lock(m_mtx);
		id = m_pIDFactory->NewID();
		m_map.emplace(id, std::move(fun));
	}
	//m_pControl->PostMessage(WM_DISPATCHER, static_cast<WPARAM>(id), 0);
	::PostThreadMessage(m_dwThreadId, WM_DISPATCHER, static_cast<WPARAM>(id), 0);
}
void CDispatcher::SendInvoke(std::function<void()>&& fun)
{
	if (GetCurrentThreadId() == m_dwThreadId)
	{
		return fun();
	}

	// std::promise はコピー不可なので shared_ptr で包んで
	// ラムダをコピー可能にする
	auto p = std::make_shared<std::promise<void>>();
	auto future = p->get_future();

	PostInvoke(
		[fun = std::move(fun), p]() mutable -> void {
			try {
				fun();
				p->set_value();
			}
			catch (...) {
				// 例外を呼び出し元へ伝搬
				try { p->set_exception(std::current_exception()); } catch (...) {}
			}
		});

	return future.get();
}
//{
//	int id = 0;
//	{
//		std::lock_guard<std::mutex> lock(m_mtx);
//		id = m_pIDFactory->NewID();
//		m_map.emplace(id, fun);
//	}
//	//m_pControl->SendMessage(WM_DISPATCHER, static_cast<WPARAM>(id), 0);
//	::SendThreadMessage(m_dwThreadId, WM_DISPATCHER, static_cast<WPARAM>(id), 0);
//}
LRESULT CDispatcher::OnDispatcher(UINT msg, WPARAM wParam, LPARAM lParam)
{
	std::function<void()> fun;
	{
		auto id = static_cast<int>(wParam);
		auto iter = m_map.find(id);
		if (iter != m_map.end()) {
			fun = iter->second;
			m_map.erase(iter);
		}
		std::lock_guard<std::mutex> lock(m_mtx);
		m_pIDFactory->DeleteID(id);
	}
	fun();
	return 0;
}

//class CDispatcher
//{
//public:
//
//    template<typename F>
//    auto Invoke(F&& func)
//        -> std::invoke_result_t<F>
//    {
//        using Result = std::invoke_result_t<F>;
//
//        // 自スレッドなら直接実行
//        if (GetCurrentThreadId() == m_uiThreadId)
//        {
//            return func();
//        }
//
//        std::promise<Result> promise;
//        auto future = promise.get_future();
//
//        BeginInvoke(
//            [f = std::forward<F>(func),
//            p = std::move(promise)]() mutable
//            {
//                try
//                {
//                    if constexpr (std::is_void_v<Result>)
//                    {
//                        f();
//                        p.set_value();
//                    } else
//                    {
//                        p.set_value(f());
//                    }
//                } catch (...)
//                {
//                    p.set_exception(
//                        std::current_exception());
//                }
//            });
//
//        return future.get();
//    }
//
//    template<typename F>
//    void BeginInvoke(F&& func)
//    {
//        {
//            std::lock_guard lock(m_lock);
//
//            m_queue.push(
//                std::function<void()>(
//                    std::forward<F>(func)));
//        }
//
//        PostMessage(
//            m_hwnd,
//            WM_DISPATCHER,
//            0,
//            0);
//    }
//
//    void ProcessQueue()
//    {
//        std::queue<std::function<void()>> queue;
//
//        {
//            std::lock_guard lock(m_lock);
//            queue.swap(m_queue);
//        }
//
//        while (!queue.empty())
//        {
//            queue.front()();
//            queue.pop();
//        }
//    }
//
//private:
//
//    DWORD m_uiThreadId{};
//    HWND  m_hwnd{};
//
//    std::mutex m_lock;
//    std::queue<std::function<void()>> m_queue;
//};

