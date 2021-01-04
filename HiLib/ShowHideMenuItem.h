#pragma once

#include "MenuItem.h"
class CSheet;
class CColumn;

class CCheckableMenuItem:public CMenuItem
{
public:
	CCheckableMenuItem(WORD wID):CMenuItem(wID){}
	virtual ~CCheckableMenuItem(){}
protected:
	LRESULT OnCommand(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)override;
	LRESULT OnInitMenu(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)override;

	virtual void Update() = 0;
};

class CShowHideColumnMenuItem :public CCheckableMenuItem
{
protected:
	CSheet * m_pSheet;
	CColumn* m_pColumn;
	std::wstring m_name;
	void Update();

public:
	CShowHideColumnMenuItem(const WORD wID, const std::wstring& str, CSheet* pSheet, CColumn* pColumn);
	virtual ~CShowHideColumnMenuItem() {}

	virtual bool GetIsChecked() override;
	virtual void SetIsChecked(bool isChecked) override;
};
