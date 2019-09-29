#include "FilerGridViewBase.h"

#include "ShellFile.h"
#include "ShellFolder.h"
#include "Row.h"
#include "Column.h"
#include "Cell.h"
#include "ParentRowHeaderColumn.h"
#include "FileRow.h"
#include "FileNameColumn.h"
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

CFilerGridViewBase::CFilerGridViewBase(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp)
	:CGridView(std::static_pointer_cast<GridViewProperty>(spFilerGridViewProp))
{
	m_cwa.dwExStyle(WS_EX_ACCEPTFILES);

	AddCmdIDHandler(IDM_CUT, &CFilerGridViewBase::OnCommandCut, this);

	//They are already assigned in GridView
	//AddCmdIDHandler(IDM_COPY,&CFilerGridViewBase::OnCommandCopy,this);
	//AddCmdIDHandler(IDM_PASTE,&CFilerGridViewBase::OnCommandPaste,this);
	//AddCmdIDHandler(IDM_DELETE,&CFilerGridViewBase::OnCommandDelete,this);

	m_spItemDragger = std::make_shared<CFileDragger>();

	CellLButtonDblClk.connect(std::bind(&CFilerGridViewBase::OnCellLButtonDblClk, this, std::placeholders::_1));
}

LRESULT CFilerGridViewBase::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//Base Create
	CGridView::OnCreate(uMsg, wParam, lParam, bHandled);
	return 0;
}

RowDictionary::const_iterator CFilerGridViewBase::FindIfRowIterByFileNameExt(const std::wstring& fileNameExt)
{
	return std::find_if(m_rowAllDictionary.begin(), m_rowAllDictionary.end(),
		[&](const RowData& data)->bool {
		if (auto p = std::dynamic_pointer_cast<CFileRow>(data.DataPtr)) {
			return p->GetFilePointer()->GetFileName() == fileNameExt;
		} else {
			return false;
		}
	});
}

void CFilerGridViewBase::OnKeyDown(const KeyDownEvent& e)
{
	m_keepEnsureVisibleFocusedCell = false;
	switch (e.Char) {
	//case 'R':
	//	if (::GetAsyncKeyState(VK_CONTROL)) {
	//		OpenFolder(m_spFolder);
	//	}
	//	break;
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
	//case VK_F8:
	//{
	//	int okcancel = ::MessageBox(m_hWnd, L"Delete?", L"Delete?", MB_OKCANCEL);
	//	if (okcancel == IDOK) {
	//		Delete();
	//	}
	//}
	//break;
	//case 'A':
	//	if (::GetAsyncKeyState(VK_CONTROL)) {
	//		SendMessage(WM_COMMAND, IDM_SELECTALL, NULL);
	//	}
	//	break;
	//case 'F':
	//	if (::GetAsyncKeyState(VK_CONTROL)) {
	//		SendMessage(WM_COMMAND, IDM_FIND, NULL);
	//	}
	//	break;

	case VK_RETURN:
	{
		if (m_spCursorer->GetFocusedCell()) {
			if (auto p = dynamic_cast<CFileRow*>(m_spCursorer->GetFocusedCell()->GetRowPtr())) {
				auto spFile = p->GetFilePointer();
				Open(spFile);
			}
		}
	}
	break;
	//case VK_BACK:
	//{
	//	OpenFolder(m_spFolder->GetParent());
	//}
	//break;
	//case VK_F2:
	//{
	//	if (m_spCursorer->GetFocusedCell()) {
	//		std::static_pointer_cast<CFileIconNameCell>(CSheet::Cell(m_spCursorer->GetFocusedCell()->GetRowPtr(), m_pNameColumn.get()))->OnEdit(EventArgs(this));
	//	}
	//}
	//break;
	//case VK_F4:
	//{
	//	auto pCell = m_spCursorer->GetFocusedCell();
	//	if (!pCell) { break; }
	//	if (auto p = dynamic_cast<CFileRow*>(pCell->GetRowPtr())) {
	//		auto spFile = p->GetFilePointer();
	//		SHELLEXECUTEINFO	sei = { 0 };
	//		sei.cbSize = sizeof(SHELLEXECUTEINFO);
	//		sei.hwnd = m_hWnd;
	//		sei.lpVerb = L"open";
	//		sei.lpFile = L"notepad.exe";
	//		sei.lpParameters = spFile->GetPath().c_str();//NULL;
	//		sei.lpDirectory = NULL;
	//		sei.nShow = SW_SHOWNORMAL;
	//		sei.hInstApp = NULL;

	//		::ShellExecuteEx(&sei);
	//	}
	//}
	//break;
	//case VK_F7:
	//	NewFolder();
	//	break;
	default:
		break;
	}

	CGridView::OnKeyDown(e);
};

void CFilerGridViewBase::InsertDefaultRowColumn()
{
	//Row
	m_rowHeader = std::make_shared<CParentHeaderRow>(this);
	m_rowNameHeader = std::make_shared<CParentHeaderRow>(this);
	m_rowFilter = std::make_shared<CParentRow>(this);

	InsertRowNotify(CRow::kMinIndex, m_rowFilter);
	InsertRowNotify(CRow::kMinIndex, m_rowNameHeader);
	InsertRowNotify(CRow::kMinIndex, m_rowHeader);

}

void CFilerGridViewBase::OnCellLButtonDblClk(CellEventArgs& e)
{
	auto pCell = e.CellPtr;
	if (auto p = dynamic_cast<CFileRow*>(e.CellPtr->GetRowPtr())) {
		auto spFile = p->GetFilePointer();
		OpenFile(spFile);
	}
}

void CFilerGridViewBase::Open(std::shared_ptr<CShellFile>& spFile)
{
	if (auto spFolder = std::dynamic_pointer_cast<CShellFolder>(spFile)) {
		OpenFolder(spFolder);
	} else {
		OpenFile(spFile);
	}
}

void CFilerGridViewBase::OpenFile(std::shared_ptr<CShellFile>& spFile)
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

bool CFilerGridViewBase::CutToClipboard()
{
	return false;//TODOTODO
//	return InvokeNormalShellContextmenuCommand(m_hWnd, "Cut", m_spFolder->GetShellFolderPtr(), GetSelectedLastPIDLVector());
}

bool CFilerGridViewBase::CopyToClipboard()
{
	return false;//TODOTODO
	//	return InvokeNormalShellContextmenuCommand(m_hWnd, "Copy", m_spFolder->GetShellFolderPtr(), GetSelectedLastPIDLVector());
}

bool CFilerGridViewBase::PasteFromClipboard()
{
	return false;//TODOTODO
	//std::vector<LPITEMIDLIST> vPidl;
	//CIDL idl = m_spFolder->GetAbsoluteIdl().CloneParentIDL();
	//CComPtr<IShellFolder> pDesktop;
	//::SHGetDesktopFolder(&pDesktop);
	//CComPtr<IShellFolder> pFolder;
	//::SHBindToObject(pDesktop, idl.ptr(), 0, IID_IShellFolder, (void**)&pFolder);
	//if (!pFolder) {
	//	pFolder = pDesktop;
	//}
	//vPidl.push_back(m_spFolder->GetAbsoluteIdl().FindLastID());
	//return InvokeNormalShellContextmenuCommand(m_hWnd, "Paste", pFolder, vPidl);
}

LRESULT CFilerGridViewBase::OnCommandCut(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CutToClipboard();
	return 0;
}

LRESULT CFilerGridViewBase::OnCommandCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CopyToClipboard();
	return 0;
}
LRESULT CFilerGridViewBase::OnCommandPaste(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	PasteFromClipboard();
	return 0;
}
LRESULT CFilerGridViewBase::OnCommandDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	DeleteSelectedFiles();
	return 0;
}


