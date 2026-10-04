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
	DWORD m_dwThreadId;
	std::mutex m_mtx;
	std::unordered_map<int, std::function<void()>> m_map;
	std::unique_ptr<CUniqueIDFactory> m_pIDFactory;


public:
	CDispatcher();
	~CDispatcher();

	void Init();
	void PostInvoke(std::function<void()>&& fun);
	void SendInvoke(std::function<void()>&& fun);
	LRESULT OnDispatcher(UINT msg, WPARAM wParam, LPARAM lParam);
	
	static CDispatcher* GetInstance()
	{
		static CDispatcher app;
		return &app;
	}

};
