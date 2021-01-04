#pragma once
#include "MyWnd.h"
#include "Direct2DWrite.h"
#include "FilerGridViewProperty.h"
#include "ShellFunction.h"
#include "DeadlineTimer.h"
#include "ReactiveProperty.h"
#include "FilerBindGridView.h"
#include "RenameInfo.h"
#include "IDL.h"
#include "ShellFunction.h"
#include "ShellFileFactory.h"
#include "ResourceIDFactory.h"
#include "ShellFile.h"
#include "ThreadPool.h"
#include "D2DWWindow.h"
#include "Button.h"
#include "ReactiveProperty.h"


template<typename... TItems>
class CFileOperationWndBase: public CD2DWWindow
{
private:

protected:
	SolidFill BackgroundFill = SolidFill(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 1.0f);

	std::shared_ptr<CButton> m_spButtonDo;
	std::shared_ptr<CButton> m_spButtonCancel;

	std::shared_ptr<CFilerBindGridView<TItems...>> m_spFilerControl;

	bool m_showDefault = true;
	bool m_showApply = true;
	bool m_isModal = false;

	CIDL m_srcIDL;
	std::vector<CIDL> m_srcChildIDLs;

	std::shared_ptr<ReactiveVectorProperty<std::tuple<TItems...>>> m_spItemsSource;
	//std::vector< std::tuple<std::shared_ptr<CShellFile>, RenameInfo>> m_selectedItems;

	std::tuple<CRectF, CRectF, CRectF> GetRects()
	{
		CRectF rc = GetRectInWnd();
		CRectF rcGrid(rc.left + 5.f, rc.top + 5.f, rc.Width() - 5.f, rc.bottom - 30.f);
		CRectF rcBtnCancel(rc.right - 5.f - 50.f, rc.bottom - 25.f, rc.right - 5.f, rc.bottom - 5.f);
		CRectF rcBtnDo(rcBtnCancel.left - 5.f - 50.f, rc.bottom - 25.f, rcBtnCancel.left - 5.f, rc.bottom - 5.f);

		return { rcGrid, rcBtnDo, rcBtnCancel };
	}


public:
	CFileOperationWndBase(const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
		const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs)
		:CD2DWWindow(),
		m_spButtonDo(std::make_shared<CButton>(this, std::make_shared<ButtonProperty>())),
		m_spButtonCancel(std::make_shared<CButton>(this, std::make_shared<ButtonProperty>())),
		m_srcIDL(srcIDL), m_srcChildIDLs(srcChildIDLs),
		m_spItemsSource(std::make_shared<ReactiveVectorProperty<std::tuple<TItems...>>>())
	{
		m_rca
			.lpszClassName(L"CFileOperationWnd")
			.style(CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS)
			.hCursor(::LoadCursor(NULL, IDC_ARROW))
			.hbrBackground((HBRUSH)GetStockObject(GRAY_BRUSH));

		DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		if (m_isModal)dwStyle |= WS_POPUP;

		m_cwa
			.lpszWindowName(L"FileOperation")
			.lpszClassName(L"CFileOperationWnd")
			.dwStyle(dwStyle);

		m_spButtonCancel->GetCommand().Subscribe([this]()->void
		{
			SendMessage(WM_CLOSE, NULL, NULL);
		});

		m_spButtonCancel->GetContent().set(L"Cancel");
	}
	virtual ~CFileOperationWndBase() = default;

	virtual void OnCreate(const CreateEvt& e) override
	{
		//Modal Window
		if (m_isModal && GetParent()) {
			::EnableWindow(GetParent(), FALSE);
		}

		//Size
		auto [rcGrid, rcBtnDo, rcBtnCancel] = GetRects();
		
		//Create FilerControl
		m_spFilerControl->OnCreate(CreateEvt(this, rcGrid));

		//OK button
		m_spButtonDo->OnCreate(CreateEvt(this, rcBtnDo));

		//Cancel button
		m_spButtonCancel->OnCreate(CreateEvt(this, rcBtnCancel));

		//Focus
		SetFocusedControlPtr(m_spButtonDo);
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
		CD2DWWindow::OnRect(e);

		auto [rcGrid, rcBtnDo, rcBtnCancel] = GetRects();		
		//Create FilerControl
		m_spFilerControl->OnRect(RectEvent(this, rcGrid));
		//OK button
		m_spButtonDo->OnRect(RectEvent(this, rcBtnDo));
		//Cancel button
		m_spButtonCancel->OnRect(RectEvent(this, rcBtnCancel));
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
	CCopyMoveWndBase(const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
						  const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs);
	virtual ~CCopyMoveWndBase() = default;
};

/************/
/* CCopyWnd */
/************/
class CCopyWnd :public CCopyMoveWndBase
{
public:
	CCopyWnd(const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
			  const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs);
	virtual ~CCopyWnd() = default;
	void Copy();
};

/************/
/* CMoveWnd */
/************/
class CMoveWnd :public CCopyMoveWndBase
{
public:
	CMoveWnd(const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
			 const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs);
	virtual ~CMoveWnd() = default;
	void Move();
};

/**************/
/* CDeleteWnd */
/**************/
class CDeleteWnd :public CFileOperationWndBase<std::shared_ptr<CShellFile>>
{
public:
	CDeleteWnd(const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
			 const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs);
	virtual ~CDeleteWnd() = default;
	void Delete();
};