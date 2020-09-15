#pragma once
#include "MyWnd.h"
#include "Direct2DWrite.h"
#include "FilerGridViewProperty.h"
#include "ShellFunction.h"
#include "DeadlineTimer.h"
#include "observable.h"
#include "FilerBindGridView.h"
#include "RenameInfo.h"
#include "IDL.h"
#include "ShellFunction.h"
#include "ShellFileFactory.h"
#include "ResourceIDFactory.h"
#include "ShellFile.h"
#include "ThreadPool.h"
#include "D2DWWindow.h"


template<typename... TItems>
class CFileOperationWndBase: public CD2DWWindow
{
protected:
	SolidFill BackgroundFill = SolidFill(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 1.0f);

	std::wstring m_buttonText;
	CButton m_buttonDo;
	CButton m_buttonCancel;

	std::shared_ptr<CFilerBindGridView<TItems...>> m_spFilerControl;

	bool m_showDefault = true;
	bool m_showApply = true;
	bool m_isModal = false;

	CIDL m_srcIDL;
	std::vector<CIDL> m_srcChildIDLs;

	std::shared_ptr<observable_vector<std::tuple<TItems...>>> m_spItemsSource;
	//std::vector< std::tuple<std::shared_ptr<CShellFile>, RenameInfo>> m_selectedItems;

public:
	CFileOperationWndBase(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
									   const std::wstring& buttonText, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs)
		:CD2DWWindow(),
		m_buttonText(buttonText), m_srcIDL(srcIDL), m_srcChildIDLs(srcChildIDLs),
		m_spItemsSource(std::make_shared< observable_vector<std::tuple<TItems...>>>())
	{
		m_rca
			.lpszClassName(L"CFileOperationWnd")
			.style(CS_VREDRAW | CS_HREDRAW)
			.hCursor(::LoadCursor(NULL, IDC_ARROW))
			.hbrBackground((HBRUSH)GetStockObject(GRAY_BRUSH));

		DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		if (m_isModal)dwStyle |= WS_POPUP;

		m_cwa
			.lpszWindowName(L"FileOperation")
			.lpszClassName(L"CFileOperationWnd")
			.dwStyle(dwStyle);


		//AddMsgHandler(WM_CREATE, &CFileOperationWndBase::OnCreate, this);
		//AddMsgHandler(WM_SIZE, &CFileOperationWndBase::OnSize, this);
		//AddMsgHandler(WM_CLOSE, &CFileOperationWndBase::OnClose, this);
		//AddMsgHandler(WM_PAINT, &CFileOperationWndBase::OnPaint, this);

		m_commandMap.emplace(CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"Do"), std::bind(&CFileOperationWndBase::OnCommandDo, this, phs::_1));
		m_commandMap.emplace(CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"Cancel"), std::bind(&CFileOperationWndBase::OnCommandCancel, this, phs::_1));
	}
	virtual ~CFileOperationWndBase() = default;

	virtual void OnCommandDo(const CommandEvent& e) = 0;
	virtual void OnCommandCancel(const CommandEvent& e)
	{
		SendMessage(WM_CLOSE, NULL, NULL);

	}

	virtual void OnCreate(const CreateEvt& e) override
	{
		//Modal Window
		if (m_isModal && GetParent()) {
			::EnableWindow(GetParent(), FALSE);
		}

		//Size
		CRect rc = GetClientRect();
		CRect rcBtnOK, rcBtnCancel;
		rcBtnOK.SetRect(rc.right - 170, rc.bottom - 25, rc.right - 170 + 52, rc.bottom - 25 + 22);
		rcBtnCancel.SetRect(rc.right - 115, rc.bottom - 25, rc.right - 115 + 52, rc.bottom - 25 + 22);

		CRect rcGrid(rc.left + 5, rc.top + 5, rc.Width() - 5, rc.bottom - 30);

		//OK button
		m_buttonDo.CreateWindowExArgument()
			.lpszClassName(WC_BUTTON)
			.hMenu((HMENU)CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"Do"))
			.lpszWindowName(m_buttonText.c_str())
			.dwStyle(WS_CHILD | WS_VISIBLE | WS_TABSTOP | BP_PUSHBUTTON);

		m_buttonDo.RegisterClassExArgument().lpszClassName(WC_BUTTON);
		m_buttonDo.AddMsgHandler(WM_KEYDOWN, [this](UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)->LRESULT
		{
			switch ((UINT)wParam) {
				case VK_RETURN:
					OnCommandDo(CommandEvent(this, 0, 0));
					break;
				default:
					break;
			}
			return 0;
		});

		//Cancel button
		m_buttonCancel.CreateWindowExArgument()
			.lpszClassName(WC_BUTTON)
			.hMenu((HMENU)CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"Cancel"))
			.lpszWindowName(L"Cancel")
			.dwStyle(WS_CHILD | WS_VISIBLE | WS_TABSTOP);

		m_buttonCancel.RegisterClassExArgument().lpszClassName(WC_BUTTON);

		//Create on order of tabstop
		m_buttonDo.Create(m_hWnd, rcBtnOK);
		m_buttonDo.SubclassWindow(m_buttonDo.m_hWnd);
		m_buttonCancel.Create(m_hWnd, rcBtnCancel);
		
		//Create FilerControl
		m_spFilerControl->OnCreate(e);
		AddControlPtr(m_spFilerControl);

		//Focus
		m_buttonDo.SetFocus();
	}
	virtual void OnClose(const CloseEvent& e) override
	{
		CD2DWWindow::OnClose(e);
		//Modal Window
		if (m_isModal && GetParent()) {
			::EnableWindow(GetParent(), TRUE);
		}
		//Foreground Owner window
		if (HWND hWnd = GetWindow(m_hWnd, GW_OWNER); (GetWindowLongPtr(GWL_STYLE) & WS_OVERLAPPEDWINDOW) == WS_OVERLAPPEDWINDOW && hWnd != NULL) {
			::SetForegroundWindow(hWnd);
		}
		DestroyWindow();
	}

	virtual void OnRect(const RectEvent& e) override
	{
		CRect rc = GetDirectPtr()->Dips2Pixels(e.Rect);
		m_buttonDo.MoveWindow(rc.right - 115, rc.bottom - 25, 52, 22, TRUE);
		m_buttonCancel.MoveWindow(rc.right - 60, rc.bottom - 25, 52, 22, TRUE);

		CRectF rcGrid(e.Rect.left + 5, e.Rect.top + 5, e.Rect.Width() - 5, e.Rect.bottom - 30);
		m_spFilerControl->OnRect(RectEvent(this, rcGrid));
	}

};

/****************/
/* CCopyMoveWnd */
/****************/
class CCopyMoveWndBase :public CFileOperationWndBase<std::shared_ptr<CShellFile>, RenameInfo>
{
protected:
	CIDL m_destIDL;
public:
	CCopyMoveWndBase(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
						  const std::wstring& buttonText,
						  const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs);
	virtual ~CCopyMoveWndBase() = default;
};

/************/
/* CCopyWnd */
/************/
class CCopyWnd :public CCopyMoveWndBase
{
public:
	CCopyWnd(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
			  const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs);
	virtual ~CCopyWnd() = default;
	virtual void OnCommandDo(const CommandEvent& e) override;
};

/************/
/* CMoveWnd */
/************/
class CMoveWnd :public CCopyMoveWndBase
{
public:
	CMoveWnd(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
			 const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs);
	virtual ~CMoveWnd() = default;
	virtual void OnCommandDo(const CommandEvent& e) override;
};

/**************/
/* CDeleteWnd */
/**************/
class CDeleteWnd :public CFileOperationWndBase<std::shared_ptr<CShellFile>>
{
public:
	CDeleteWnd(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
			 const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs);
	virtual ~CDeleteWnd() = default;
	virtual void OnCommandDo(const CommandEvent& e) override;
};

///***********************/
///* CIncrementalCopyWnd */
///***********************/
////class CIncrementalCopyWnd :public CCopyMoveWndBase
////{
////public:
////	static UINT WM_INCREMENTMAX;
////	static UINT WM_INCREMENTVALUE;
////	static UINT WM_ADDITEM;
////
////protected:
////	std::unique_ptr<CProgressBar> m_pProgressbar;
////	CIDL m_newIDL;
////	CDeadlineTimer m_periodicTimer;
////	std::unordered_map<CIDL, std::vector<CIDL>,
////		shell::IdlHash, shell::IdlEqual> m_idlMap;
////
////public:
////	CIncrementalCopyWnd(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
////			   const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs);
////	virtual ~CIncrementalCopyWnd() = default;
////	virtual void InitializeFileGrid();
////	virtual LRESULT OnCommandDo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) override;
////};