#pragma once
#include "MyWnd.h"
#include "MyFont.h"
class CDrawTextProperty;
class CRect;

//class IInplaceEdit
//{
//public:
//	typedef std::basic_string<TCHAR> std::wstring;
//public:
//
//	virtual HWND Create(HWND hWnd,CRect rcClient)=0;
//	virtual BOOL SetWindowText(LPCTSTR lpszText)=0;
//	virtual void SetFont(HFONT hFont,BOOL bRedraw=TRUE)=0;
//	virtual DWORD SetMargins(UINT uLeft, UINT uRight) = 0;
//	virtual HWND SetFocus()=0;
//	virtual void SetSel(int nStartChar,int nEndChar,BOOL bNoScroll = FALSE)=0;
//	virtual BOOL ShowWindow(int nCmdShow)=0;
//	virtual BOOL MoveWindow(CRect& rc,BOOL bRepaint)=0;
//	virtual CRect GetRect() = 0;
//	virtual void SetRect(const CRect& rc) = 0;
//
//static IInplaceEdit* CreateInstance(
//	std::function<std::wstring()> getter,
//	std::function<void(const std::wstring&)> setter,
//	std::function<void(const std::wstring&)> changed,
//	std::function<void()> final,
//	std::shared_ptr<CDrawTextProperty> spDrawTextProperty);
//};
//
//typedef std::shared_ptr<IInplaceEdit> InplaceEditPtr;

class CInplaceEdit:public CEdit
{
private:
	std::function<std::wstring()> m_getter;
	std::function<void(const std::wstring&)> m_setter;
	std::function<void(const std::wstring&)> m_changed;
	std::function<void()> m_final;
	CFont m_font;
	UINT m_format;
	std::wstring m_strInit;
	bool m_bAlt;
public:
	CInplaceEdit(
		std::function<std::wstring()> getter,
		std::function<void(const std::wstring&)> setter,
		std::function<void(const std::wstring&)> changed,
		std::function<void()> final,
		CFont font,
		UINT dwDTFormat);
	virtual ~CInplaceEdit(){}
	HWND Create(HWND hWnd, RECT& rcClient) override;

private:
	LRESULT OnSysKeyDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	LRESULT OnMenuChar(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	LRESULT OnKeyDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	LRESULT OnKillFocus(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	LRESULT OnCmdEnChange(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);
	void OnFinalMessage(HWND hWnd);
	void EndEdit();

	//BOOL SetWindowText(LPCTSTR lpszText){return CEdit::SetWindowTextW(lpszText);}
	//void SetFont(HFONT hFont,BOOL bRedraw){return CEdit::SetFont(hFont,bRedraw);}
	//DWORD SetMargins(UINT uLeft, UINT uRight){return CEdit::SetMargins(uLeft, uRight);}
	//HWND SetFocus(){return CEdit::SetFocus();}
	//void SetSel(int nStartChar,int nEndChar,BOOL bNoScroll){return CEdit::SetSel(nStartChar,nEndChar,bNoScroll);}
	//BOOL ShowWindow(int nCmdShow){return CEdit::ShowWindow(nCmdShow);}
	//BOOL MoveWindow(CRect& rc,BOOL bRepaint){return CEdit::MoveWindow(rc,bRepaint);}
	//CRect GetRect(){return CEdit::GetRect();};
	//void SetRect(const CRect& rc){return CEdit::SetRect(rc);}

};
