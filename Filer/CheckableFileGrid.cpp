#include "CheckableFileGrid.h"

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

extern std::shared_ptr<CApplicationProperty> g_spApplicationProperty;

CCheckableFileGrid::CCheckableFileGrid(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp)
	:CFilerGridViewBase(spFilerGridViewProp)
{
	m_cwa
		.dwExStyle(WS_EX_ACCEPTFILES);

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



