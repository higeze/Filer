#pragma once
#include <mutex>
#include <unordered_map>

class CD2DWWindow;
class CUniqueIDFactory;

class CDispatcher
{
public:
	static UINT WM_DISPATCHER;
private:
	CD2DWWindow* m_pControl;
	std::mutex m_mtx;
	std::unordered_map<int, std::function<void()>> m_map;
	std::unique_ptr<CUniqueIDFactory> m_pIDFactory;


public:
	CDispatcher(CD2DWWindow* m_pControl);
	~CDispatcher();

	void PostInvoke(std::function<void()> fun);
	void SendInvoke(std::function<void()> fun);
	LRESULT OnDispatcher(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);


};
