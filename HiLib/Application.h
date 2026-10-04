#pragma once
#include "Dispatcher.h"

template<class T>
class CApplication
{
protected:
	HWND m_hDlgModeless = nullptr;
	std::unique_ptr<CDispatcher> m_pDispatcher = nullptr;

public:
	CApplication(){}
	void SetDialog(HWND hDlg) { m_hDlgModeless = hDlg; }
	std::unique_ptr<CDispatcher>& GetDispatcherPtr() { return m_pDispatcher; }
	virtual void Init() = 0;
	virtual int RunInterface()
	{
		BOOL ret;
		MSG msg;
		while ((ret = GetMessage(&msg, NULL, 0, 0)) != 0) {
			if (ret == -1) {
				break;
			} else if(msg.message == CDispatcher::WM_DISPATCHER){
				CDispatcher::GetInstance()->OnDispatcher(msg.message, msg.wParam, msg.lParam);
			} else if ((m_hDlgModeless == (HWND)NULL || !IsDialogMessage(m_hDlgModeless, &msg))) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		return msg.wParam;
	}

	virtual void Term() = 0;

	static T* GetInstance()
	{
		static T app;
		return &app;
	}

};
