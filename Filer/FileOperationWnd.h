#pragma once
#include "MyWnd.h"
#include "Direct2DWrite.h"
#include "FilerGridViewProperty.h"
#include "ShellFunction.h"
#include "DeadlineTimer.h"
#include "observable.h"
#include "FilerBindGridView.h"
#include "RenameInfo.h"

class CIDL;
class CCopyFileGrid;

class CFileOperationWndBase :public CWnd
{
protected:
	std::shared_ptr<d2dw::CDirect2DWrite> m_pDirect;
	d2dw::SolidFill BackgroundFill = d2dw::SolidFill(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 1.0f);

	std::wstring m_buttonText;
	CButton m_buttonDo;
	CButton m_buttonCancel;

	std::unique_ptr<CFilerBindGridView<std::shared_ptr<CShellFile>, RenameInfo>> m_pFileGrid;

	bool m_showDefault = true;
	bool m_showApply = true;
	bool m_isModal = false;

	CIDL m_srcIDL;
	std::vector<CIDL> m_srcChildIDLs;
	CIDL m_destIDL;

	std::shared_ptr<observable_vector<std::tuple<std::shared_ptr<CShellFile>, RenameInfo>>> m_spItemsSource;
	//std::vector< std::tuple<std::shared_ptr<CShellFile>, RenameInfo>> m_selectedItems;

public:
	CFileOperationWndBase(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
						  const std::wstring& buttonText,
						  const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs);
	virtual ~CFileOperationWndBase();
	virtual d2dw::CDirect2DWrite* GetDirectPtr() override { return m_pDirect.get(); }

	virtual LRESULT OnCommandDo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) = 0;
	virtual LRESULT OnCommandCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	virtual LRESULT OnCreate(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnClose(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnDestroy(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

class CCopyWnd :public CFileOperationWndBase
{
public:
	CCopyWnd(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
			  const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs);
	virtual ~CCopyWnd() = default;
	virtual LRESULT OnCommandDo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) override;
};

class CMoveWnd :public CFileOperationWndBase
{
public:
	CMoveWnd(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
			 const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs);
	virtual ~CMoveWnd() = default;
	virtual LRESULT OnCommandDo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) override;
};