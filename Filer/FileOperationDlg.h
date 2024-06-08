#pragma once
#include "D2DWWindow.h"
#include "D2DWDialog.h"
#include "D2DWControl.h"
#include "ShellFunction.h"
#include "DeadlineTimer.h"
#include "FilerBindGridView.h"
#include "RenameInfo.h"
#include "IDL.h"
#include "ShellFunction.h"
#include "ShellFileFactory.h"
#include "ResourceIDFactory.h"
#include "ShellFile.h"
#include "D2DWWindow.h"
#include "Button.h"
#include "Textbox.h"
#include "ExeExtensionProperty.h"
#include "Dispatcher.h"
#include "reactive_vector.h"

template<typename T>
class CFileOperationGridView :public CFilerBindGridView<T>
{
	using CFilerBindGridView<T>::CFilerBindGridView;

	virtual bool DeleteSelectedFiles() override
	{
		auto indexes = this->GetSelectedIndexes();
		auto frozen_count = this->GetFrozenCount<RowTag>();
		for (auto i : indexes) {
			this->GetItemsSource().erase(this->GetItemsSource()->cbegin() + (i - frozen_count));
		}
		return true;
	}
};

/*************************/
/* CFileOperationDlgBase */
/*************************/
template<typename T>
class CFileOperationDlgBase: public CD2DWDialog
{
private:

protected:
	SolidFill BackgroundFill = SolidFill(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 1.0f);

	std::shared_ptr<int> Dummy;
	std::shared_ptr<CButton> m_spButtonDo;
	std::shared_ptr<CButton> m_spButtonCancel;

	std::shared_ptr<CFileOperationGridView<T>> m_spFilerControl;

	bool m_showDefault = true;
	bool m_showApply = true;
	bool m_isModal = false;

	CIDL m_srcIDL;
	std::vector<CIDL> m_srcChildIDLs;

	std::future<void> m_future;

public:
	reactive_vector_ptr<T> ItemsSource;

public:
	CFileOperationDlgBase(
		CD2DWControl* pParentControl,
		const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs)
		:CD2DWDialog(pParentControl),
		m_spButtonDo(std::make_shared<CButton>(this)),
		m_spButtonCancel(std::make_shared<CButton>(this)),
		m_srcIDL(srcIDL), m_srcChildIDLs(srcChildIDLs), Dummy(std::make_shared<int>(0))
	{
		m_spButtonCancel->Command.subscribe([this]()->void{
			GetWndPtr()->GetDispatcherPtr()->PostInvoke([this]() { OnClose(CloseEvent(GetWndPtr(), NULL, NULL)); });
		}, Dummy);

		m_spButtonCancel->Content.set(L"Cancel");
	}
	virtual ~CFileOperationDlgBase() = default;

	virtual void OnClose(const CloseEvent& e) override
	{
		CD2DWDialog::OnClose(e);
	}
};

/*******************************/
/* CSimpleFileOperationDlgBase */
/*******************************/
template<typename T>
class CSimpleFileOperationDlgBase :public CFileOperationDlgBase<T>
{
private:
	std::tuple<CRectF, CRectF, CRectF> GetRects()
	{
		CRectF rc = this->GetRectInWnd();
		CRectF rcTitle = this->GetTitleRect();
		rc.top = rcTitle.bottom;
		CRectF rcGrid(rc.left + 5.f, rc.top + 5.f, rc.right - 5.f, rc.bottom - 30.f);
		CRectF rcBtnCancel(rc.right - 5.f - 50.f, rc.bottom - 25.f, rc.right - 5.f, rc.bottom - 5.f);
		CRectF rcBtnDo(rcBtnCancel.left - 5.f - 50.f, rc.bottom - 25.f, rcBtnCancel.left - 5.f, rc.bottom - 5.f);

		return { rcGrid, rcBtnDo, rcBtnCancel };
	}

public:
	using CFileOperationDlgBase<T>::CFileOperationDlgBase;
	virtual ~CSimpleFileOperationDlgBase() = default;

	virtual void OnCreate(const CreateEvt& e) override
	{
		//Create
		CFileOperationDlgBase<T>::OnCreate(e);
		//Size
		auto [rcGrid, rcBtnDo, rcBtnCancel] = GetRects();
		
		//Create FilerControl
		this->m_spFilerControl->OnCreate(CreateEvt(this->GetWndPtr(), this, rcGrid));

		//OK button
		this->m_spButtonDo->OnCreate(CreateEvt(this->GetWndPtr(), this, rcBtnDo));

		//Cancel button
		this->m_spButtonCancel->OnCreate(CreateEvt(this->GetWndPtr(), this, rcBtnCancel));

		//Focus
		this->GetWndPtr()->SetFocusToControl(this->m_spButtonDo);
	}

	virtual void OnRect(const RectEvent& e) override
	{
		CD2DWControl::OnRect(e);

		auto [rcGrid, rcBtnDo, rcBtnCancel] = GetRects();		
		//Create FilerControl
		this->m_spFilerControl->OnRect(RectEvent(this->GetWndPtr(), rcGrid));
		//OK button
		this->m_spButtonDo->OnRect(RectEvent(this->GetWndPtr(), rcBtnDo));
		//Cancel button
		this->m_spButtonCancel->OnRect(RectEvent(this->GetWndPtr(), rcBtnCancel));
	}
};

/****************/
/* CCopyMoveDlg */
/****************/
class CCopyMoveDlgBase :public CSimpleFileOperationDlgBase<std::tuple<std::shared_ptr<CShellFile>, RenameInfo>>
{
protected:
	CIDL m_destIDL;
public:
	CCopyMoveDlgBase(CD2DWControl* pParentControl,
		const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs);
	virtual ~CCopyMoveDlgBase() = default;
};

/************/
/* CCopyDlg */
/************/
class CCopyDlg :public CCopyMoveDlgBase
{
public:
	CCopyDlg(CD2DWControl* pParentControl,
		const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs);
	virtual ~CCopyDlg() = default;
	void Copy();
};

/************/
/* CMoveDlg */
/************/
class CMoveDlg :public CCopyMoveDlgBase
{
public:
	CMoveDlg(CD2DWControl* pParentControl,
		const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs);
	virtual ~CMoveDlg() = default;
	void Move();
};

/**************/
/* CDeleteDlg */
/**************/
class CDeleteDlg :public CSimpleFileOperationDlgBase<std::shared_ptr<CShellFile>>
{
public:
	CDeleteDlg(CD2DWControl* pParentControl,
		const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs);
	virtual ~CDeleteDlg() = default;
	void Delete();
};

/***********/
/* CExeWnd */
/***********/

class CExeExtensionDlg: public CFileOperationDlgBase<std::shared_ptr<CShellFile>>
{
private:

protected:
	std::shared_ptr<CTextBox> m_spTextPath;
	std::shared_ptr<CTextBox> m_spTextParam;
	ExeExtension m_exeExtension;

	std::tuple<CRectF, CRectF, CRectF, CRectF, CRectF> GetRects();



public:
	CExeExtensionDlg(CD2DWControl* pParentControl,
		const std::shared_ptr<CShellFolder>& folder,
		const std::vector<std::shared_ptr<CShellFile>>& files,
		const ExeExtension& exeExtension);

	virtual ~CExeExtensionDlg() = default;

	void Execute();

	virtual void OnCreate(const CreateEvt& e) override;
	virtual void OnRect(const RectEvent& e) override;
};

class CPDFOperationDlgBase: public CFileOperationDlgBase<std::shared_ptr<CShellFile>>
{
private:
protected:
	std::tuple<CRectF, CRectF, CRectF, CRectF> GetRects();
	std::shared_ptr<CTextBox> m_spParameter;

public:
	CPDFOperationDlgBase(CD2DWControl* pParentControl,
		const std::shared_ptr<CShellFolder>& folder,
		const std::vector<std::shared_ptr<CShellFile>>& files);
	virtual ~CPDFOperationDlgBase() = default;

	virtual void OnCreate(const CreateEvt& e) override;
	virtual void OnRect(const RectEvent& e) override;
};

class CPDFSplitDlg : public CPDFOperationDlgBase
{
public:
	CPDFSplitDlg(CD2DWControl* pParentControl,
		const std::shared_ptr<CShellFolder>& folder,
		const std::vector<std::shared_ptr<CShellFile>>& files);
	virtual ~CPDFSplitDlg() = default;
};

class CPDFMergeDlg : public CPDFOperationDlgBase
{
public:
	CPDFMergeDlg(CD2DWControl* pParentControl,
		const std::shared_ptr<CShellFolder>& folder,
		const std::vector<std::shared_ptr<CShellFile>>& files);
	virtual ~CPDFMergeDlg() = default;
};

class CPDFExtractDlg : public CPDFOperationDlgBase
{
public:
	CPDFExtractDlg(CD2DWControl* pParentControl,
		const std::shared_ptr<CShellFolder>& folder,
		const std::vector<std::shared_ptr<CShellFile>>& files);
	virtual ~CPDFExtractDlg() = default;
};

class CPDFUnlockDlg : public CPDFOperationDlgBase
{
public:
	CPDFUnlockDlg(CD2DWControl* pParentControl,
		const std::shared_ptr<CShellFolder>& folder,
		const std::vector<std::shared_ptr<CShellFile>>& files);
	virtual ~CPDFUnlockDlg() = default;
};

