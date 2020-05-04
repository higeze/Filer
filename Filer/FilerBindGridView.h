#pragma once

#include "BindGridView.h"
#include "IDL.h"
#include "Resource.h"

#include "FilerGridViewProperty.h"

#include "shobjidl.h"
#include "MyFriendSerializer.h"
#include "RowIndexColumn.h"
#include "FileColumn.h"
#include "FileSizeColumn.h"
#include "FileLastWriteColumn.h"
#include "MyXMLSerializer.h"
#include "ShellFile.h"
#include "ShellFolder.h"
#include "FileDragger.h"
#include "SheetEventArgs.h"
#include "ShowHideMenuItem.h"

#define WM_UAHMEASUREMENUITEM 0x0094

template<typename... TItems>
class CFilerBindGridView :public CBindGridView<TItems...>
{
protected:
	//Columns
	std::shared_ptr<CColumn> m_pNameColumn;

	//HeaderMenuItems
	std::vector<std::shared_ptr<CShowHideColumnMenuItem>> m_headerMenuItems;
public:
	//Constructor
	CFilerBindGridView(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
					   std::shared_ptr<observable_vector<std::tuple<TItems...>>> spItemsSource = nullptr)
		:CBindGridView(std::static_pointer_cast<GridViewProperty>(spFilerGridViewProp), spItemsSource)
	{
		m_cwa.dwExStyle(WS_EX_ACCEPTFILES);

		AddCmdIDHandler(IDM_CUT, &CFilerBindGridView::OnCommandCut, this);

		//They are already assigned in GridView
		//AddCmdIDHandler(IDM_COPY,&CFilerGridViewBase::OnCommandCopy,this);
		//AddCmdIDHandler(IDM_PASTE,&CFilerGridViewBase::OnCommandPaste,this);
		//AddCmdIDHandler(IDM_DELETE,&CFilerGridViewBase::OnCommandDelete,this);

		m_spItemDragger = std::make_shared<CFileDragger>();

		CellLButtonDblClk.connect(std::bind(&CFilerBindGridView::OnCellLButtonDblClk, this, std::placeholders::_1));
	}
	//Destructor
	virtual ~CFilerBindGridView() = default;

	//Getter
	std::shared_ptr<FilerGridViewProperty> GetFilerGridViewPropPtr() { return std::static_pointer_cast<FilerGridViewProperty>(m_spGridViewProp); }
	std::shared_ptr<CColumn> GetFileNameColumnPtr() { return m_pNameColumn; }
	void SetFileNameColumnPtr(const std::shared_ptr<CColumn>& spCol) { m_pNameColumn = spCol; }
	virtual bool HasSheetCell()override { return false; }
	virtual bool IsVirtualPage()override { return true; }

	/**********/
	/* Signal */
	/**********/
	std::function<void(CMenu&)> AddCustomContextMenu;
	std::function<bool(int, CComPtr<IShellFolder>, std::vector<PITEMID_CHILD>)> ExecCustomContextMenu;

	virtual void OnCellLButtonDblClk(CellEventArgs& e)
	{
		auto pCell = e.CellPtr;
		if (auto spRow = dynamic_cast<CBindRow<TItems...>*>(e.CellPtr->GetRowPtr())) {
			auto spFile = spRow->GetItem<std::shared_ptr<CShellFile>>();
			OpenFile(spFile);
		}
	}

	virtual void Open(std::shared_ptr<CShellFile>& spFile)
	{
		if (auto spFolder = std::dynamic_pointer_cast<CShellFolder>(spFile)) {
			OpenFolder(spFolder);
		} else {
			OpenFile(spFile);
		}
	}

	virtual void OpenFile(std::shared_ptr<CShellFile>& spFile)
	{
		SHELLEXECUTEINFO	sei = { 0 };
		sei.cbSize = sizeof(SHELLEXECUTEINFO);
		sei.fMask = SEE_MASK_INVOKEIDLIST;
		sei.hwnd = m_hWnd;
		sei.lpVerb = NULL;
		sei.lpFile = NULL;
		sei.lpParameters = NULL;
		sei.lpDirectory = NULL;
		sei.nShow = SW_SHOWNORMAL;
		sei.hInstApp = NULL;
		sei.lpIDList = (LPVOID)(spFile->GetAbsoluteIdl().ptr());

		::ShellExecuteEx(&sei);
	}	
	
	virtual void OpenFolder(std::shared_ptr<CShellFolder>& spFolder)
	{
		//auto pWnd = new CFilerGridView(std::static_pointer_cast<FilerGridViewProperty>(m_spGridViewProp));

		//pWnd->RegisterClassExArgument()
		//	.lpszClassName(L"CFilerGridViewWnd")
		//	.style(CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS)
		//	.hCursor(::LoadCursor(NULL, IDC_ARROW))
		//	.hbrBackground((HBRUSH)GetStockObject(GRAY_BRUSH));

		//pWnd->CreateWindowExArgument()
		//	.lpszClassName(_T("CFilerGridViewWnd"))
		//	.lpszWindowName(spFolder->GetDispName().c_str())
		//	.dwStyle(WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)
		//	.dwExStyle(WS_EX_ACCEPTFILES)
		//	.hMenu(NULL);

		//pWnd->FolderChanged = [pWnd](std::shared_ptr<CShellFolder>& pFolder) {
		//	pWnd->SetWindowTextW(pFolder->GetDispName().c_str());};

		//pWnd->SetIsDeleteOnFinalMessage(true);

		//HWND hWnd = NULL;
		//if ((GetWindowLongPtr(GWL_STYLE) & WS_OVERLAPPEDWINDOW) == WS_OVERLAPPEDWINDOW) {
		//	hWnd = m_hWnd;
		//} else {
		//	hWnd = GetAncestorByStyle(WS_OVERLAPPEDWINDOW);
		//}

		//pWnd->CreateOnCenterOfParent(hWnd, CSize(300, 500));
		//pWnd->SetIsDeleteOnFinalMessage(true);
		//pWnd->OpenFolder(spFolder);
		//pWnd->ShowWindow(SW_SHOW);
		//pWnd->UpdateWindow();
	}

	index_vector<std::shared_ptr<CRow>>::const_iterator FindIfRowIterByFileNameExt(const std::wstring& fileNameExt)
	{
		return std::find_if(m_allRows.begin(), m_allRows.end(),
							[&](const std::shared_ptr<CRow>& rowPtr)->bool {
								if (auto p = std::dynamic_pointer_cast<CFileRow>(rowPtr)) {
									return p->GetFilePointer()->GetDispName() == fileNameExt;
								} else {
									return false;
								}
							});
	}

	virtual bool CutToClipboard() { return false; }
	virtual bool CopyToClipboard() { return false; }
	virtual bool PasteFromClipboard() { return false; }
	virtual bool DeleteSelectedFiles() { return false; }

	/******************/
	/* Window Message */
	/******************/
	virtual LRESULT OnCommandCut(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		CutToClipboard();
		return 0;
	}

	virtual LRESULT OnCommandCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		CopyToClipboard();
		return 0;
	}
	
	virtual LRESULT OnCommandPaste(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		PasteFromClipboard();
		return 0;
	}
	
	virtual LRESULT OnCommandDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DeleteSelectedFiles();
		return 0;
	}

	/*****************/
	/* State Message */
	/*****************/
	void Normal_KeyDown(const KeyDownEvent& e) override
	{
		m_keepEnsureVisibleFocusedCell = false;
		switch (e.Char) {
			case 'X':
				if (::GetAsyncKeyState(VK_CONTROL)) {
					CutToClipboard();
				}
				break;
			case 'C':
				if (::GetAsyncKeyState(VK_CONTROL)) {
					CopyToClipboard();
				}
				break;
			case 'V':
				if (::GetAsyncKeyState(VK_CONTROL)) {
					PasteFromClipboard();
				}
				break;
			case VK_DELETE:
				{
					int okcancel = ::MessageBox(m_hWnd, L"Delete?", L"Delete?", MB_OKCANCEL);
					if (okcancel == IDOK) {
						DeleteSelectedFiles();
					}
				}
				break;
			case VK_RETURN:
				{
					if (m_spCursorer->GetFocusedCell()) {
						if (auto spRow = dynamic_cast<CBindRow<TItems...>*>(m_spCursorer->GetFocusedCell()->GetRowPtr())) {
							auto spFile = spRow->GetItem<std::shared_ptr<CShellFile>>();
							Open(spFile);
						}
					}
				}
				break;
			default:
				break;
		}

		CGridView::Normal_KeyDown(e);
	};

	bool Edit_Guard_KeyDownWithNormal(const KeyDownEvent& e) override
	{
		return false;
	}

	bool Edit_Guard_KeyDownWithoutNormal(const KeyDownEvent& e) override
	{
		return Edit_Guard_KeyDown(e);
	}
};