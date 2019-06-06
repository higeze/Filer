#pragma once
#include "MyWnd.h"
#include "Direct2DWrite.h"
#include "FilerGridViewProperty.h"
#include "ShellFunction.h"

//#define IDC_BUTTON_OK							162
//#define IDC_BUTTON_CANCEL						163

namespace d2dw
{
	class CProgressBar;
}

class CIDL;
class CCheckableFileGrid;

class CIncrementalCopyWnd :public CWnd
{
private:
	std::shared_ptr<d2dw::CDirect2DWrite> m_pDirect;

	d2dw::SolidFill BackgroundFill = d2dw::SolidFill(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 1.0f);

	CButton m_buttonCopy;
	CButton m_buttonClose;

	std::unique_ptr<d2dw::CProgressBar> m_pProgressbar;
	std::unique_ptr<CCheckableFileGrid> m_pFileGrid;

	bool m_showDefault = true;
	bool m_showApply = true;
	bool m_isModal = false;

	CIDL m_srcIDL;
	std::vector<CIDL> m_srcChildIDLs;
	CIDL m_destIDL;

	CIDL m_newIDL;

	std::unordered_map<CIDL, std::vector<CIDL>,
		shell::IdlHash, shell::IdlEqual> m_idlMap;

public:
	static UINT WM_INCREMENTMAX;
	static UINT WM_INCREMENTVALUE;
	static UINT WM_ADDITEM;
	CIncrementalCopyWnd(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
		const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs);
	~CIncrementalCopyWnd();

	void AddItem(const CIDL& destIDL, const CIDL srcIDL);
	std::unique_ptr<d2dw::CProgressBar>& GetProgressBarPtr() { return m_pProgressbar; }

	LRESULT OnCommandCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandClose(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCreate(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnIncrementMax(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnIncrementValue(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnAddItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnFinalMessage(HWND hWnd);
};