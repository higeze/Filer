#include "CheckableFileGrid.h"
#include "FilerGridView.h"

#include "ShellFile.h"
#include "ShellFolder.h"
#include "Row.h"
#include "Column.h"
#include "Cell.h"
#include "ParentRowHeaderColumn.h"
#include "FileRow.h"
#include "FileNameColumn.h"
#include "FileIconPathColumn.h"
#include "FileSizeColumn.h"
#include "FileExtColumn.h"
#include "FileLastWriteColumn.h"
#include "shlwapi.h"

#include "SheetEventArgs.h"
#include "Resource.h"

#include "SingletonMalloc.h"
#include "MyPoint.h"
#include "Cursorer.h"
#include "Celler.h"

#include "ConsoleTimer.h"
#include "ApplicationProperty.h"

#include "DropTarget.h"
#include "DropSource.h"
#include "FileDragger.h"

#include "MyWin32.h"
#include "Debug.h"
#include "DirectoryWatcher.h"

#include "FileIconNameCell.h"

#include "MyMenu.h"
#include "MenuItem.h"
#include "ShowHideMenuItem.h"

#include "PathRow.h"
#include "KnownFolder.h"
#include "DriveFolder.h"
#include "InplaceEdit.h"
#include "Scroll.h"
#include "ShellFileFactory.h"
#include "ThreadPool.h"

#include "ShellFunction.h"
#include "IncrementalCopyWnd.h"
#include "ProgressBar.h"
#include "ResourceIDFactory.h"

extern std::shared_ptr<CApplicationProperty> g_spApplicationProperty;
extern HWND g_main;

CCheckableFileGrid::CCheckableFileGrid(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp)
	:CFilerGridViewBase(spFilerGridViewProp)
{
	m_cwa
		.dwExStyle(WS_EX_ACCEPTFILES);

//	AddMsgHandler(WM_ACTIVATE, &CCheckableFileGrid::OnActivate, this);

//	CellLButtonDblClk.connect(std::bind(&CFilerGridView::OnCellLButtonDblClk, this, std::placeholders::_1));
}


LRESULT CCheckableFileGrid::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//Base Create
	CFilerGridViewBase::OnCreate(uMsg, wParam, lParam, bHandled);

	//Insert rows
	m_rowNameHeader = std::make_shared<CParentHeaderRow>(this);
	m_rowFilter = std::make_shared<CParentRow>(this);

	InsertRowNotify(CRow::kMinIndex, m_rowFilter, false);
	InsertRowNotify(CRow::kMinIndex, m_rowNameHeader, false);

	//Insert columns if not initialized
	if (m_columnAllDictionary.empty()) {
		auto insertFun = [this](std::shared_ptr<CColumn> col, int defaultIndex) {
			InsertColumnNotify(col->GetSerializedIndex() == CColumn::kInvalidIndex ? defaultIndex : col->GetSerializedIndex(), col, false);
		};

		insertFun(std::make_shared<CParentRowHeaderColumn>(this), CColumn::kMinIndex);
		//insertFun(std::make_shared<CFileIconColumn>(this), CColumn::kMaxIndex);
		m_pNameColumn = std::make_shared<CFileIconPathColumn>(this);
		insertFun(m_pNameColumn, CColumn::kMaxIndex);
		insertFun(std::make_shared<CFileExtColumn>(this), CColumn::kMaxIndex);
		insertFun(std::make_shared<CFileSizeColumn>(this, GetFilerGridViewPropPtr()->FileSizeArgsPtr), CColumn::kMaxIndex);
		insertFun(std::make_shared<CFileLastWriteColumn>(this, GetFilerGridViewPropPtr()->FileTimeArgsPtr), CColumn::kMaxIndex);
	}

	PostUpdate(Updates::All);
	SubmitUpdate();

	return 0;
}



void CCheckableFileGrid::InsertDefaultRowColumn()
{
	//Row
//	m_rowHeader = std::make_shared<CParentHeaderRow>(this);
	m_rowNameHeader = std::make_shared<CParentHeaderRow>(this);
	m_rowFilter = std::make_shared<CParentRow>(this);

	InsertRowNotify(CRow::kMinIndex, m_rowFilter);
	InsertRowNotify(CRow::kMinIndex, m_rowNameHeader);
//	InsertRowNotify(CRow::kMinIndex, m_rowHeader);

}


void CCheckableFileGrid::AddItem(const std::shared_ptr<CShellFile>& spFile)
{
		if (false) {
			m_pDirect->ClearTextLayoutMap();
		}

		if (m_pEdit) {
			::SendMessage(m_pEdit->m_hWnd, WM_CLOSE, NULL, NULL);
		}

		if (Empty()) {
			InsertDefaultRowColumn();
		}

		InsertRow(CRow::kMaxIndex, std::make_shared<CFileRow>(this, spFile));
		for (auto& col : m_columnAllDictionary) {
			col.DataPtr->SetMeasureValid(false);
		}

		PostUpdate(Updates::Sort);
		PostUpdate(Updates::Filter);
		PostUpdate(Updates::ColumnVisible);
		PostUpdate(Updates::RowVisible);
		PostUpdate(Updates::Column);
		PostUpdate(Updates::Row);
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);

		//Celler
		m_spCeller->Clear();

		//Cursor
		m_spCursorer->Clear();

		SubmitUpdate();
}

void CCheckableFileGrid::OnCellLButtonDblClk(CellEventArgs& e)
{
	auto pCell = e.CellPtr;
	if (auto p = dynamic_cast<CFileRow*>(e.CellPtr->GetRowPtr())) {
		auto spFile = p->GetFilePointer();
		Open(spFile);
	}
}

void CCheckableFileGrid::OpenFolder(std::shared_ptr<CShellFolder>& spFolder)
{
	auto pWnd = new CFilerGridView(std::static_pointer_cast<FilerGridViewProperty>(m_spGridViewProp));

	pWnd->RegisterClassExArgument()
		.lpszClassName(L"CFilerGridViewWnd")
		.style(CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS)
		.hCursor(::LoadCursor(NULL, IDC_ARROW))
		.hbrBackground((HBRUSH)GetStockObject(GRAY_BRUSH));

	pWnd->CreateWindowExArgument()
		.lpszClassName(_T("CFilerGridViewWnd"))
		.lpszWindowName(spFolder->GetFileName().c_str())
		.dwStyle(WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)
		.dwExStyle(WS_EX_ACCEPTFILES)
		.hMenu(NULL);

	pWnd->FolderChanged.connect([pWnd](std::shared_ptr<CShellFolder>& pFolder) {
		pWnd->SetWindowTextW(pFolder->GetFileName().c_str());
	});

	pWnd->SetIsDeleteOnFinalMessage(true);

	HWND hWnd = NULL;
	if ((GetWindowLongPtr(GWL_STYLE) & WS_OVERLAPPEDWINDOW) == WS_OVERLAPPEDWINDOW) {
		hWnd = m_hWnd;
	} else {
		hWnd = GetAncestorByStyle(WS_OVERLAPPEDWINDOW);
	}

	pWnd->CreateOnCenterOfParent(hWnd, CSize(300, 500));
	pWnd->OpenFolder(spFolder);
	pWnd->ShowWindow(SW_SHOW);
	pWnd->UpdateWindow();
}





