#pragma once

template<class TRect>
class CApplication
{
protected:
	HWND m_hDlgModeless = NULL;

public:
	void SetDialog(HWND hDlg) { m_hDlgModeless = hDlg; }
	virtual void Init() = 0;
	virtual int Run()
	{
		BOOL ret;
		MSG msg;
		while ((ret = GetMessage(&msg, NULL, 0, 0)) != 0) {
			if (ret == -1) {
				break;
			} else if ((m_hDlgModeless == (HWND)NULL || !IsDialogMessage(m_hDlgModeless, &msg))) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		return msg.wParam;
	}

	virtual void Term() = 0;

	static TRect* GetInstance()
	{
		static TRect app;
		return &app;
	}

};
