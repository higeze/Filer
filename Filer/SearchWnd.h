#pragma once
#include "MyWnd.h"
#include "Direct2DWrite.h"
#include "FilerGridViewProperty.h"
#include "ShellFunction.h"


namespace d2dw
{
	class CProgressBar;
}

class CIDL;
class CCheckableFileGrid;

class CSearchWnd :public CWnd
{
private:
	std::shared_ptr<d2dw::CDirect2DWrite> m_pDirect;

	d2dw::SolidFill BackgroundFill = d2dw::SolidFill(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 1.0f);

	CButton m_buttonSearch;
	CButton m_buttonCancel;
	CButton m_buttonClose;
	CEdit m_edit;
	CFont m_font;

	std::unique_ptr<d2dw::CProgressBar> m_pProgressbar;
	std::unique_ptr<CCheckableFileGrid> m_pFileGrid;

	CIDL m_srcIDL;
	CIDL m_newIDL;

	CRect GetEditRect(const CRect& rcClient)const;
	CRect GetProgressBarRect(const CRect& rcClient)const;
	CRect GetFileGridRect(const CRect& rcClient)const;
	CRect GetSearchBtnRect(const CRect& rcClient)const;
	CRect GetCancelBtnRect(const CRect& rcClient)const;
	CRect GetCloseBtnRect(const CRect& rcClient)const;

public:
	static UINT WM_INCREMENTMAX;
	static UINT WM_INCREMENTVALUE;
	static UINT WM_ADDITEM;

	CSearchWnd(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp, const CIDL& srcIDL);
	~CSearchWnd();
	virtual d2dw::CDirect2DWrite* GetDirectPtr()const override { return m_pDirect.get(); }

	std::unique_ptr<d2dw::CProgressBar>& GetProgressBarPtr() { return m_pProgressbar; }

	LRESULT OnCreate(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnIncrementMax(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnIncrementValue(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnAddItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnCommandSearch(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandClose(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};