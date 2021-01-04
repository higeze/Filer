#include "ShowHideMenuItem.h"
#include "Sheet.h"
#include "Column.h"

LRESULT CCheckableMenuItem::OnCommand(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
{
	if(GetIsChecked()){
		SetIsChecked(false);
	}else{
		SetIsChecked(true);
	}
	Update();
	return 0;	
}

LRESULT CCheckableMenuItem::OnInitMenu(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	HMENU hMenu = (HMENU)wParam;

	if(GetMenuItemInfo(hMenu, GetID(), FALSE, &m_mii)){
		m_mii.fState = GetIsChecked() ? MFS_CHECKED : MFS_UNCHECKED;
		SetMenuItemInfo(hMenu,GetID(),FALSE,&m_mii);
	}
	return 0;
}

CShowHideColumnMenuItem::CShowHideColumnMenuItem(const WORD wID, const std::wstring& str, CSheet* pSheet, CColumn* pColumn)
	:CCheckableMenuItem(wID), m_pSheet(pSheet), m_pColumn(pColumn), m_name(str)
{
	m_mii.fMask = MIIM_ID | MIIM_STATE | MIIM_STRING;
	m_mii.dwTypeData = (LPWSTR)m_name.c_str();
	m_mii.fState = GetIsChecked() ? MFS_CHECKED : MFS_UNCHECKED;
}

bool CShowHideColumnMenuItem::GetIsChecked()
{
	return m_pColumn->GetIsVisible();
}

void CShowHideColumnMenuItem::SetIsChecked(bool isChecked)
{
	m_pColumn->SetIsVisible(isChecked);
	m_mii.fState = isChecked ? MFS_CHECKED : MFS_UNCHECKED;
}

void CShowHideColumnMenuItem::Update()
{
	//PostUpdate is already called in ColumnVisibleChanged
	m_pSheet->SubmitUpdate();
}