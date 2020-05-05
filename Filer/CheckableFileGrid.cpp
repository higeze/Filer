#include "CheckableFileGrid.h"
#include "FilerGridView.h"
#include "ShellFile.h"
#include "ShellFolder.h"
#include "Row.h"
#include "Column.h"
#include "Cell.h"
#include "RowIndexColumn.h"
#include "FileColumn.h"
#include "FileSizeColumn.h"
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
#include "Scroll.h"
#include "ShellFileFactory.h"
#include "ThreadPool.h"

#include "ShellFunction.h"
#include "IncrementalCopyWnd.h"
#include "ProgressBar.h"
#include "ResourceIDFactory.h"
#include "Textbox.h"

extern std::shared_ptr<CApplicationProperty> g_spApplicationProperty;
extern HWND g_main;

CCheckableFileGrid::CCheckableFileGrid(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp)
	:CFilerBindGridView(spFilerGridViewProp)
{
	m_cwa
		.dwExStyle(WS_EX_ACCEPTFILES);
}


LRESULT CCheckableFileGrid::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//Base Create
	CFilerBindGridView::OnCreate(uMsg, wParam, lParam, bHandled);

	//Insert rows
	m_rowNameHeader = std::make_shared<CHeaderRow>(this);
	m_rowFilter = std::make_shared<CRow>(this);

	m_allRows.idx_push_back(m_rowNameHeader);
	m_allRows.idx_push_back(m_rowFilter);

	m_frozenRowCount = 2;


	//Insert columns if not initialized
	if (m_allCols.empty()) {
		m_pNameColumn = std::make_shared<CFileIconPathColumn<std::shared_ptr<CShellFile>>>(this, L"Name");
		PushColumns(
			std::make_shared<CRowIndexColumn>(this),
			m_pNameColumn,
			std::make_shared<CFileDispExtColumn<std::shared_ptr<CShellFile>>>(this, L"Ext"),
			std::make_shared<CFileSizeColumn<std::shared_ptr<CShellFile>>>(this, GetFilerGridViewPropPtr()->FileSizeArgsPtr),
			std::make_shared<CFileLastWriteColumn<std::shared_ptr<CShellFile>>>(this, GetFilerGridViewPropPtr()->FileTimeArgsPtr));

		m_frozenColumnCount = 1;
	}

	PostUpdate(Updates::All);
	SubmitUpdate();

	return 0;
}

void CCheckableFileGrid::AddItem(const std::shared_ptr<CShellFile>& spFile)
{
	//Edit
	if (m_pEdit) {
		EndEdit();
	}
	//Celler
	m_spCeller->Clear();
	//Cursor
	m_spCursorer->Clear();

	m_spItemsSource->notify_push_back(std::make_tuple(spFile));

	PostUpdate(Updates::All);
	SubmitUpdate();
}

void CCheckableFileGrid::OnCellLButtonDblClk(CellEventArgs& e)
{
	auto pCell = e.CellPtr;
	if (auto spRow = dynamic_cast<CBindRow<std::shared_ptr<CShellFile>>*>(e.CellPtr->GetRowPtr())) {
		auto spFile = spRow->GetItem<std::shared_ptr<CShellFile>>();
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
		.lpszWindowName(spFolder->GetDispName().c_str())
		.dwStyle(WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)
		.dwExStyle(WS_EX_ACCEPTFILES)
		.hMenu(NULL);

	pWnd->FolderChanged = [pWnd](std::shared_ptr<CShellFolder>& pFolder) {
		pWnd->SetWindowTextW(pFolder->GetDispName().c_str());};

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





