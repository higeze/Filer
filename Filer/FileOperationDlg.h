#pragma once
#include "D2DWWindow.h"
#include "D2DWDialog.h"
#include "D2DWControl.h"
#include "FilerGridViewProperty.h"
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

class CFileOperationGridView :public CFilerBindGridView
{
	using CFilerBindGridView::CFilerBindGridView;

	virtual bool DeleteSelectedFiles() override
	{
		auto indexes = this->GetSelectedIndexes();
		auto frozen_count = this->GetFrozenCount<RowTag>();
		for (auto i : indexes) {
			this->ItemsSource.erase(this->ItemsSource->cbegin() + (i - frozen_count));
		}
		return true;
	}
};

/*************************/
/* CFileOperationDlgBase */
/*************************/
class CFileOperationDlgBase: public CD2DWDialog
{
private:

protected:
	SolidFill BackgroundFill = SolidFill(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 1.0f);

	std::shared_ptr<int> Dummy;
	std::shared_ptr<CButton> m_spButtonDo;
	std::shared_ptr<CButton> m_spButtonCancel;

	std::shared_ptr<CFileOperationGridView> m_spFilerControl;

	bool m_showDefault = true;
	bool m_showApply = true;
	bool m_isModal = false;

	CIDL m_srcIDL;
	std::vector<CIDL> m_srcChildIDLs;

	std::future<void> m_future;

public:
	reactive_vector_ptr<any_tuple> ItemsSource;

public:
	CFileOperationDlgBase(
		CD2DWControl* pParentControl,
		const std::shared_ptr<DialogProperty>& spDialogProp,
		const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
		const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs);
	virtual ~CFileOperationDlgBase() = default;
};

/*******************************/
/* CSimpleFileOperationDlgBase */
/*******************************/
class CSimpleFileOperationDlgBase :public CFileOperationDlgBase
{
private:
	std::tuple<CRectF, CRectF, CRectF> GetRects();
public:
	using CFileOperationDlgBase::CFileOperationDlgBase;
	virtual ~CSimpleFileOperationDlgBase() = default;

	virtual void OnCreate(const CreateEvt& e) override;
	virtual void OnRect(const RectEvent& e) override;
};

/****************/
/* CCopyMoveDlg */
/****************/
class CCopyMoveDlgBase :public CSimpleFileOperationDlgBase
{
protected:
	CIDL m_destIDL;
public:
	CCopyMoveDlgBase(CD2DWControl* pParentControl,
		const std::shared_ptr<DialogProperty>& spDialogProp,
		const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
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
		const std::shared_ptr<DialogProperty>& spDialogProp,
		const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
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
		const std::shared_ptr<DialogProperty>& spDialogProp,
		const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
		const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs);
	virtual ~CMoveDlg() = default;
	void Move();
};

/**************/
/* CDeleteDlg */
/**************/
class CDeleteDlg :public CSimpleFileOperationDlgBase
{
public:
	CDeleteDlg(CD2DWControl* pParentControl,
		const std::shared_ptr<DialogProperty>& spDialogProp,
		const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
		const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs);
	virtual ~CDeleteDlg() = default;
	void Delete();
};

/***********/
/* CExeWnd */
/***********/

class CExeExtensionDlg: public CFileOperationDlgBase
{
private:

protected:
	std::shared_ptr<CTextBox> m_spTextPath;
	std::shared_ptr<CTextBox> m_spTextParam;
	ExeExtension m_exeExtension;

	std::tuple<CRectF, CRectF, CRectF, CRectF, CRectF> GetRects();



public:
	CExeExtensionDlg(CD2DWControl* pParentControl,
		const std::shared_ptr<DialogProperty>& spDialogProp,
		const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
		const std::shared_ptr<TextBoxProperty>& spTextBoxProp,
		const std::shared_ptr<CShellFolder>& folder,
		const std::vector<std::shared_ptr<CShellFile>>& files,
		const ExeExtension& exeExtension);

	virtual ~CExeExtensionDlg() = default;

	void Execute();

	virtual void OnCreate(const CreateEvt& e) override;
	virtual void OnRect(const RectEvent& e) override;
};

class CPDFOperationDlgBase: public CFileOperationDlgBase
{
private:
protected:
	std::tuple<CRectF, CRectF, CRectF, CRectF> GetRects();
	std::shared_ptr<CTextBox> m_spParameter;

public:
	CPDFOperationDlgBase(CD2DWControl* pParentControl,
		const std::shared_ptr<DialogProperty>& spDialogProp,
		const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
		const std::shared_ptr<TextBoxProperty>& spTextBoxProp,
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
		const std::shared_ptr<DialogProperty>& spDialogProp,
		const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
		const std::shared_ptr<TextBoxProperty>& spTextBoxProp,
		const std::shared_ptr<CShellFolder>& folder,
		const std::vector<std::shared_ptr<CShellFile>>& files);
	virtual ~CPDFSplitDlg() = default;
};

class CPDFMergeDlg : public CPDFOperationDlgBase
{
public:
	CPDFMergeDlg(CD2DWControl* pParentControl,
		const std::shared_ptr<DialogProperty>& spDialogProp,
		const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
		const std::shared_ptr<TextBoxProperty>& spTextBoxProp,
		const std::shared_ptr<CShellFolder>& folder,
		const std::vector<std::shared_ptr<CShellFile>>& files);
	virtual ~CPDFMergeDlg() = default;
};

class CPDFExtractDlg : public CPDFOperationDlgBase
{
public:
	CPDFExtractDlg(CD2DWControl* pParentControl,
		const std::shared_ptr<DialogProperty>& spDialogProp,
		const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
		const std::shared_ptr<TextBoxProperty>& spTextBoxProp,
		const std::shared_ptr<CShellFolder>& folder,
		const std::vector<std::shared_ptr<CShellFile>>& files);
	virtual ~CPDFExtractDlg() = default;
};

class CPDFUnlockDlg : public CPDFOperationDlgBase
{
public:
	CPDFUnlockDlg(CD2DWControl* pParentControl,
		const std::shared_ptr<DialogProperty>& spDialogProp,
		const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
		const std::shared_ptr<TextBoxProperty>& spTextBoxProp,
		const std::shared_ptr<CShellFolder>& folder,
		const std::vector<std::shared_ptr<CShellFile>>& files);
	virtual ~CPDFUnlockDlg() = default;
};

