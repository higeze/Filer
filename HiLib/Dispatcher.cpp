#include "Dispatcher.h"
#include "D2DWWindow.h"
#include "UniqueIDFactory.h"


UINT CDispatcher::WM_DISPATCHER = ::RegisterWindowMessage(L"WM_DISPATCHER");


CDispatcher::CDispatcher(CD2DWWindow* pWnd):m_pControl(pWnd), m_pIDFactory(std::make_unique<CUniqueIDFactory>()){}

CDispatcher::~CDispatcher() = default;

void CDispatcher::PostInvoke(std::function<void()> fun)
{
	int id = 0;
	{
		std::lock_guard<std::mutex> lock(m_mtx);
		id = m_pIDFactory->NewID();
		m_map.emplace(id, fun);
	}
	m_pControl->PostMessage(WM_DISPATCHER, static_cast<WPARAM>(id), 0);
}
void CDispatcher::SendInvoke(std::function<void()> fun)
{
	int id = 0;
	{
		std::lock_guard<std::mutex> lock(m_mtx);
		id = m_pIDFactory->NewID();
		m_map.emplace(id, fun);
	}
	m_pControl->SendMessage(WM_DISPATCHER, static_cast<WPARAM>(id), 0);
}
LRESULT CDispatcher::OnDispatcher(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
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

