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

#include "Debug.h"
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

#include "ShellFunction.h"
#include "IncrementalCopyDlg.h"
#include "ProgressBar.h"
#include "ResourceIDFactory.h"
#include "Textbox.h"
#include "D2DWWindow.h"

extern std::shared_ptr<CApplicationProperty> g_spApplicationProperty;
extern HWND g_main;

CCheckableFileGrid::CCheckableFileGrid(CD2DWControl* pParentControl, const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp)
	:CFilerBindGridView(pParentControl, spFilerGridViewProp){}


void CCheckableFileGrid::OnCreate(const CreateEvt& e)
{
	//Base Create
	CFilerBindGridView::OnCreate(e);

	//Insert rows
	m_pNameHeaderRow = std::make_shared<CHeaderRow>(this, GetCellProperty());
	m_pFilterRow = std::make_shared<CRow>(this, GetCellProperty());

	m_allRows.idx_push_back(m_pNameHeaderRow);
	m_allRows.idx_push_back(m_pFilterRow);

	m_frozenRowCount = 2;


	//Insert columns if not initialized
	if (m_allCols.empty()) {
		m_pNameColumn = std::make_shared<CFileIconPathColumn>(this, L"Name");
		PushColumns(
			std::make_shared<CRowIndexColumn>(this, GetHeaderProperty()),
			m_pNameColumn,
			std::make_shared<CFileDispExtColumn>(this, L"Ext"),
			std::make_shared<CFileSizeColumn>(this, GetFilerGridViewPropPtr()->FileSizeArgsPtr),
			std::make_shared<CFileLastWriteColumn>(this, GetFilerGridViewPropPtr()->FileTimeArgsPtr));

		m_frozenColumnCount = 1;
	}

	PostUpdate(Updates::All);
	SubmitUpdate();
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

	ItemsSource.emplace_back(std::move(spFile));

	for (auto& spCol : m_allCols) {
		spCol->SetIsFitMeasureValid(false);
		spCol->SetIsMeasureValid(false);
	}
	PostUpdate(Updates::All);
	SubmitUpdate();
}

void CCheckableFileGrid::OnCellLButtonDblClk(const CellEventArgs& e)
{
	auto pCell = e.CellPtr;
	if (auto spRow = dynamic_cast<CBindRow*>(e.CellPtr->GetRowPtr())) {
		auto spFile = spRow->GetItem<std::shared_ptr<CShellFile>>();
		Open(spFile);
	}
}

void CCheckableFileGrid::OpenFolder(const std::shared_ptr<CShellFolder>& spFolder)
{
	auto pFilerWnd = new CD2DWSingleControlWnd<CFilerGridView>(std::static_pointer_cast<FilerGridViewProperty>(m_spGridViewProp));


	pFilerWnd->RegisterClassExArgument()
		.lpszClassName(L"CFilerGridViewWnd")
		.style(CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS)
		.hCursor(::LoadCursor(NULL, IDC_ARROW))
		.hbrBackground((HBRUSH)GetStockObject(GRAY_BRUSH));

	pFilerWnd->CreateWindowExArgument()
		.lpszClassName(_T("CFilerGridViewWnd"))
		.lpszWindowName(spFolder->GetDispName().c_str())
		.dwStyle(WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)
		.dwExStyle(WS_EX_ACCEPTFILES)
		.hMenu(NULL);

	pFilerWnd->GetChildControlPtr()->FolderChanged = [pFilerWnd](std::shared_ptr<CShellFolder>& pFolder) {
		pFilerWnd->SetWindowTextW(pFolder->GetDispName().c_str());};
	pFilerWnd->SetIsDeleteOnFinalMessage(true);

	pFilerWnd->CreateOnCenterOfParent(GetWndPtr()->m_hWnd, CSize(300, 500));
	pFilerWnd->GetChildControlPtr()->OpenFolder(spFolder);
	pFilerWnd->ShowWindow(SW_SHOW);
	pFilerWnd->UpdateWindow();
}





