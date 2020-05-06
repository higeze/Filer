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


template<typename... TItems>
class CFileOperationWndBase: public CWnd
{
protected:
	std::shared_ptr<d2dw::CDirect2DWrite> m_pDirect;
	d2dw::SolidFill BackgroundFill = d2dw::SolidFill(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 1.0f);

	std::wstring m_buttonText;
	CButton m_buttonDo;
	CButton m_buttonCancel;

	std::unique_ptr<CFilerBindGridView<TItems...>> m_pFileGrid;

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
		:CWnd(),
		m_buttonText(buttonText), m_srcIDL(srcIDL), m_srcChildIDLs(srcChildIDLs),
		m_spItemsSource(std::make_shared< observable_vector<std::tuple<TItems...>>>())
	{
		m_pFileGrid = std::make_unique<CFilerBindGridView<TItems...>>(
			spFilerGridViewProp,
			m_spItemsSource);
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


		AddMsgHandler(WM_CREATE, &CFileOperationWndBase::OnCreate, this);
		AddMsgHandler(WM_SIZE, &CFileOperationWndBase::OnSize, this);
		AddMsgHandler(WM_CLOSE, &CFileOperationWndBase::OnClose, this);
		AddMsgHandler(WM_PAINT, &CFileOperationWndBase::OnPaint, this);

		AddCmdIDHandler(CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"Do"), &CFileOperationWndBase::OnCommandDo, this);
		AddCmdIDHandler(CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"Cancel"), &CFileOperationWndBase::OnCommandCancel, this);
	}
	virtual ~CFileOperationWndBase() = default;
	virtual void InitializeFileGrid() = 0;
	virtual d2dw::CDirect2DWrite* GetDirectPtr() override { return m_pDirect.get(); }

	virtual LRESULT OnCommandDo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) = 0;
	virtual LRESULT OnCommandCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		{
			SendMessage(WM_CLOSE, NULL, NULL);
			return 0;
		}

	}

	virtual LRESULT OnCreate(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		//Direct2DWrite
		m_pDirect = std::make_shared<d2dw::CDirect2DWrite>(m_hWnd);

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
		//FileGrid
		m_pFileGrid->CreateWindowExArgument()
			.dwStyle(WS_CHILD | WS_VISIBLE | WS_TABSTOP | m_pFileGrid->CreateWindowExArgument().dwStyle());

		//OK button
		m_buttonDo.CreateWindowExArgument()
			.lpszClassName(WC_BUTTON)
			.hMenu((HMENU)CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"Do"))
			.lpszWindowName(m_buttonText.c_str())
			.dwStyle(WS_CHILD | WS_VISIBLE | WS_TABSTOP | BP_PUSHBUTTON);

		m_buttonDo.RegisterClassExArgument().lpszClassName(WC_BUTTON);
		m_buttonDo.AddMsgHandler(WM_KEYDOWN, [this](UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)->LRESULT {
			switch ((UINT)wParam) {
				case VK_RETURN:
					OnCommandDo(0, 0, 0, bHandled);
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
		m_pFileGrid->Create(m_hWnd, rcGrid);
		InitializeFileGrid();
		m_pFileGrid->PostUpdate(Updates::All);
		m_pFileGrid->SubmitUpdate();

		m_buttonDo.SetFocus();

		return 0;


	}
	virtual LRESULT OnClose(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		//Modal Window
		if (m_isModal && GetParent()) {
			::EnableWindow(GetParent(), TRUE);
		}
		//Foreground Owner window
		if (HWND hWnd = GetWindow(m_hWnd, GW_OWNER); (GetWindowLongPtr(GWL_STYLE) & WS_OVERLAPPEDWINDOW) == WS_OVERLAPPEDWINDOW && hWnd != NULL) {
			::SetForegroundWindow(hWnd);
		}
		//Destroy
		m_pFileGrid->DestroyWindow();
		DestroyWindow();

		return 0;
	}
	virtual LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CPaintDC dc(m_hWnd);
		m_pDirect->BeginDraw();

		m_pDirect->ClearSolid(BackgroundFill);

		m_pDirect->EndDraw();
		return 0;
	}

	virtual LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CRect rc = GetClientRect();
		m_pDirect->GetHwndRenderTarget()->Resize(D2D1_SIZE_U{ (UINT)rc.Width(), (UINT)rc.Height() });

		m_buttonDo.MoveWindow(rc.right - 115, rc.bottom - 25, 52, 22, TRUE);
		m_buttonCancel.MoveWindow(rc.right - 60, rc.bottom - 25, 52, 22, TRUE);
		CRect rcGrid(rc.left + 5, rc.top + 5, rc.Width() - 5, rc.bottom - 30);
		m_pFileGrid->MoveWindow(rcGrid, TRUE);

		return 0;
	}

};

class CCopyMoveWndBase :public CFileOperationWndBase<std::shared_ptr<CShellFile>, RenameInfo>
{
protected:
	CIDL m_destIDL;
public:
	CCopyMoveWndBase(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
						  const std::wstring& buttonText,
						  const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs);
	virtual ~CCopyMoveWndBase() = default;
	virtual void InitializeFileGrid();
};

class CCopyWnd :public CCopyMoveWndBase
{
public:
	CCopyWnd(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
			  const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs);
	virtual ~CCopyWnd() = default;
	virtual LRESULT OnCommandDo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) override;
};

class CMoveWnd :public CCopyMoveWndBase
{
public:
	CMoveWnd(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
			 const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs);
	virtual ~CMoveWnd() = default;
	virtual LRESULT OnCommandDo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) override;
};

class CDeleteWnd :public CFileOperationWndBase<std::shared_ptr<CShellFile>>
{
public:
	CDeleteWnd(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
			 const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs);
	virtual ~CDeleteWnd() = default;
	virtual void InitializeFileGrid();
	virtual LRESULT OnCommandDo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) override;
};