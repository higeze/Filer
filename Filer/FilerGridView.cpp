#include "FilerGridView.h"

#include "ShellFile.h"
#include "ShellFolder.h"
#include "Row.h"
#include "Column.h"
#include "Cell.h"
#include "ParentRowHeaderColumn.h"
#include "FileRow.h"
#include "FileNameColumn.h"
#include "FileSizeColumn.h"
#include "FileIconColumn.h"
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

#include "FileNameCell.h"

#include "MyMenu.h"
#include "MenuItem.h"
#include "ShowHideMenuItem.h"

#include "PathRow.h"


#define SCRATCH_QCM_FIRST 1
#define SCRATCH_QCM_NEW 600//200,500 are used by system
#define SCRATCH_QCM_LAST  0x7FFF

extern std::shared_ptr<CApplicationProperty> g_spApplicationProperty;

CFilerGridView::CFilerGridView(std::shared_ptr<CGridViewProperty> spGridViewProp, std::shared_ptr<FilerGridViewProperty> spFilerGridViewProp)
	:CGridView(spGridViewProp), m_spFilerGridViewProp(spFilerGridViewProp)
{
	m_cwa
	.dwExStyle(WS_EX_ACCEPTFILES);
	AddMsgHandler(WM_DIRECTORYWATCH,&CFilerGridView::OnDirectoryWatch,this);

	AddCmdIDHandler(IDM_CUT,&CFilerGridView::OnCommandCut,this);
	//AddCmdIDHandler(IDM_COPYTEXT, &CFilerGridView::OnCommandCopyText, this);

	//They are already assigned in GridView
	//AddCmdIDHandler(IDM_COPY,&CFilerGridView::OnCommandCopy,this);
	//AddCmdIDHandler(IDM_PASTE,&CFilerGridView::OnCommandPaste,this);
	//AddCmdIDHandler(IDM_DELETE,&CFilerGridView::OnCommandDelete,this);
	m_spItemDragger = std::make_shared<CFileDragger>();

	CellLButtonDblClk.connect(std::bind(&CFilerGridView::OnCellLButtonDblClk,this,std::placeholders::_1));
}

LRESULT CFilerGridView::OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	//Directory watcher
	m_spWatcher = std::make_shared<CDirectoryWatcher>(m_hWnd);

	//Drag & Drop
	//DropTarget
	auto pDropTarget = new CDropTarget(m_hWnd);
	pDropTarget->Dropped.connect([this](IDataObject *pDataObj, DWORD dwEffect)->void {Dropped(pDataObj, dwEffect); });
	m_pDropTarget = CComPtr<IDropTarget>(pDropTarget);
	::RegisterDragDrop(m_hWnd, m_pDropTarget);
	//DropSource
	m_pDropSource = CComPtr<IDropSource>(new CDropSource);
	m_pDragSourceHelper.CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER);

	//Desktop IShellFolder
	::SHGetDesktopFolder(&m_pDesktopShellFolder);

	//Base Create
	CGridView::OnCreate(uMsg,wParam,lParam,bHandled);

	//Insert rows
	m_rowHeader = std::make_shared<CPathRow>(this);
	m_rowNameHeader = std::make_shared<CParentHeaderRow>(this);
	m_rowFilter = std::make_shared<CParentRow>(this);

	InsertRowNotify(CRow::kMinIndex, m_rowFilter, false);
	InsertRowNotify(CRow::kMinIndex, m_rowNameHeader, false);
	InsertRowNotify(CRow::kMinIndex, m_rowHeader, false);

	//Insert columns if not initialized
	if (m_columnAllDictionary.empty()) {
		auto insertFun = [this](std::shared_ptr<CColumn> col, size_type defaultIndex) {
			InsertColumnNotify(col->GetSerializedIndex() == CColumn::kInvalidIndex ? defaultIndex : col->GetSerializedIndex(), col, false);
		};

		insertFun(std::make_shared<CParentRowHeaderColumn>(this), CColumn::kMinIndex);
		insertFun(std::make_shared<CFileIconColumn>(this), CColumn::kMaxIndex);
		m_pNameColumn = std::make_shared<CFileNameColumn>(this);
		insertFun(m_pNameColumn, CColumn::kMaxIndex);
		insertFun(std::make_shared<CFileExtColumn>(this), CColumn::kMaxIndex);
		insertFun(std::make_shared<CFileSizeColumn>(this, GetFilerGridViewPropPtr()->FileSizeArgsPtr), CColumn::kMaxIndex);
		insertFun(std::make_shared<CFileLastWriteColumn>(this), CColumn::kMaxIndex);
	}

	//Header menu items
	//for (auto iter = m_columnAllDictionary.begin(); iter != m_columnAllDictionary.end(); ++iter) {
	//	auto cell = CSheet::Cell(m_rowNameHeader, iter->DataPtr);
	//	auto str = cell->GetString();
	//	m_headerMenuItems.push_back(std::make_shared<CShowHideColumnMenuItem>(
	//		IDM_VISIBLEROWHEADERCOLUMN + std::distance(m_columnAllDictionary.begin(), iter),
	//		Cell(m_rowNameHeader, iter->DataPtr)->GetString(), this, iter->DataPtr.get()));
	//}

	//for (auto& item : m_headerMenuItems) {
	//	AddCmdIDHandler(item->GetID(), std::bind(&CMenuItem::OnCommand, item.get(), phs::_1, phs::_2, phs::_3, phs::_4));
	//}

	return 0;
}

void CFilerGridView::Dropped(IDataObject *pDataObj, DWORD dwEffect)
{
	CComPtr<IShellItem2> pDestShellItem;
	CComPtr<IFileOperation> pFileOperation;

	HRESULT hr = ::SHCreateItemFromIDList(m_spFolder->GetAbsoluteIdl().ptr(), IID_IShellItem2, reinterpret_cast<LPVOID*>(&pDestShellItem));
	if (FAILED(hr)) { return; }

	hr = pFileOperation.CoCreateInstance(CLSID_FileOperation);
	if (FAILED(hr)) { return; }

	switch (dwEffect) {
	case DROPEFFECT_MOVE:
	{
		//if folder is same, do not need to move
		FORMATETC formatetc = { 0 };
		formatetc.cfFormat = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_SHELLIDLIST);
		formatetc.ptd = NULL;
		formatetc.dwAspect = DVASPECT_CONTENT;
		formatetc.lindex = -1;
		formatetc.tymed = TYMED_HGLOBAL;

		STGMEDIUM medium;
		HRESULT hr = pDataObj->GetData(&formatetc, &medium);
		if (FAILED(hr)) { return; }
		LPIDA pida = (LPIDA)GlobalLock(medium.hGlobal);
		CIDL folderIdl(::ILCloneFull((LPCITEMIDLIST)(((LPBYTE)pida) + (pida)->aoffset[0])));

		if (folderIdl == m_spFolder->GetAbsoluteIdl()) {
			//Do nothing
		} else {
			HRESULT hr = pFileOperation->MoveItems(pDataObj, pDestShellItem);
			if (FAILED(hr)) { return; }
			hr = pFileOperation->PerformOperations();
			if (FAILED(hr)) { return; }
		}
	}
	break;
	case DROPEFFECT_COPY:
	{
		HRESULT hr = pFileOperation->CopyItems(pDataObj, pDestShellItem);
		hr = pFileOperation->PerformOperations();
		if (FAILED(hr)) { return; }
	}
	break;
	case DROPEFFECT_LINK:
	{
		FORMATETC formatetc = { 0 };
		formatetc.cfFormat = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_SHELLIDLIST);
		formatetc.ptd = NULL;
		formatetc.dwAspect = DVASPECT_CONTENT;
		formatetc.lindex = -1;
		formatetc.tymed = TYMED_HGLOBAL;

		STGMEDIUM medium;
		HRESULT hr = pDataObj->GetData(&formatetc, &medium);
		if (FAILED(hr)) { return; }
		LPIDA pida = (LPIDA)GlobalLock(medium.hGlobal);
		CIDL folderIdl(::ILCloneFull((LPCITEMIDLIST)(((LPBYTE)pida) + (pida)->aoffset[0])));
		for (UINT i = 0; i < pida->cidl; i++) {
			CIDL childIdl(::ILCloneFull((LPCITEMIDLIST)(((LPBYTE)pida) + pida->aoffset[1 + i])));
			CIDL absoluteIdl(folderIdl + childIdl);
			CComPtr<IShellLink> pShellLink;

			pShellLink.CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER);
			pShellLink->SetIDList(absoluteIdl.ptr());
			CComQIPtr<IPersistFile> pPersistFile(pShellLink);

			std::wstring destPath = m_spFolder->GetPath();
			WCHAR szSrcPath[MAX_PATH];
			WCHAR szDestPath[MAX_PATH];
			::SHGetPathFromIDList(absoluteIdl.ptr(), szSrcPath);
			LPTSTR lpszFileName = PathFindFileName(szSrcPath);
			lstrcpyW(szDestPath, destPath.c_str());
			PathAppend(szDestPath, lpszFileName);

			wsprintfW(szDestPath, L"%s.lnk", szDestPath);
			pPersistFile->Save(szDestPath, TRUE);
		}
		GlobalUnlock(medium.hGlobal);
		ReleaseStgMedium(&medium);
	}
	break;
	default:
		break;
	}
}

RowDictionary::const_iterator CFilerGridView::FindIfRowIterByFileNameExt(const std::wstring& fileNameExt)
{
	return std::find_if(m_rowAllDictionary.begin(), m_rowAllDictionary.end(),
		[&](const RowData& data)->bool {
		if (auto p = std::dynamic_pointer_cast<CFileRow>(data.DataPtr)) {
			return p->GetFilePointer()->GetFileName() == fileNameExt;
		}
		else {
			return false;
		}
	});
}


void CFilerGridView::Added(const std::wstring& fileName)
{
	BOOST_LOG_TRIVIAL(trace) << "Added " << wstr2str(fileName);
	CIDL idl;
	ULONG chEaten;
	ULONG dwAttributes;
	HRESULT hr = m_spFolder->GetShellFolderPtr()->ParseDisplayName(m_hWnd, NULL, const_cast<LPWSTR>(fileName.c_str()), &chEaten, idl.ptrptr(), &dwAttributes);
	if (SUCCEEDED(hr) && idl) {
		auto spFile(m_spFolder->CreateShExFileFolder(idl));
		auto spRow = std::make_shared<CFileRow>(this, spFile);
		InsertRow(CRow::kMaxIndex, spRow);

		PostUpdate(Updates::ColumnVisible);
		PostUpdate(Updates::RowVisible);
		PostUpdate(Updates::Row);
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);

		if (m_bNewFile) {
			m_spCursorer->OnCursor(CSheet::Cell(spRow, m_pNameColumn));
			PostUpdate(Updates::EnsureVisibleFocusedCell);
		}
		PostUpdate(Updates::Sort);
		FilterAll();
		SubmitUpdate();
		if (m_bNewFile) {
			std::static_pointer_cast<CFileNameCell>(CSheet::Cell(spRow, m_pNameColumn))->OnEdit(EventArgs(this));
		}
		m_bNewFile = false;
	}
	else {
		BOOST_LOG_TRIVIAL(trace) << "Added FAILED " << wstr2str(fileName);
	}
}

void CFilerGridView::Modified(const std::wstring& fileName)
{
	BOOST_LOG_TRIVIAL(trace) << "Modified " << wstr2str(fileName);
	auto iter = FindIfRowIterByFileNameExt(fileName);

	if (iter == m_rowAllDictionary.end()) {
		BOOST_LOG_TRIVIAL(trace) << "Modified NoMatch " << wstr2str(fileName);
		return;
	}
	else if (auto p = std::dynamic_pointer_cast<CFileRow>(iter->DataPtr)) {
		p->GetFilePointer()->Reset();
		p->SetMeasureValid(false);
	}
	PostUpdate(Updates::ColumnVisible);
	PostUpdate(Updates::RowVisible);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);
	PostUpdate(Updates::Sort);
	FilterAll();
	SubmitUpdate();

}
void CFilerGridView::Removed(const std::wstring& fileName)
{
	BOOST_LOG_TRIVIAL(trace) << "Removed " << wstr2str(fileName);
	auto iter = FindIfRowIterByFileNameExt(fileName);

	if (iter == m_rowAllDictionary.end()) {
		BOOST_LOG_TRIVIAL(trace) << "Removed NoMatch " << wstr2str(fileName);
		return;
	}

	EraseRowNotify(iter->DataPtr.get(), false);
	for (const auto& col : m_columnAllDictionary) {
		std::dynamic_pointer_cast<CParentMapColumn>(col.DataPtr)->Clear();
	}

	m_spCursorer->OnCursorClear(this);

	PostUpdate(Updates::ColumnVisible);
	PostUpdate(Updates::RowVisible);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);
	PostUpdate(Updates::Sort);

	FilterAll();
	SubmitUpdate();

}
void CFilerGridView::Renamed(const std::wstring& oldName, const std::wstring& newName)
{
	BOOST_LOG_TRIVIAL(trace) << "Renamed " << wstr2str(oldName) << "=>"<< wstr2str(newName);
	auto iter = FindIfRowIterByFileNameExt(oldName);

	if (iter == m_rowAllDictionary.end()) 
	{
		BOOST_LOG_TRIVIAL(trace) << "Renamed NoMatch " << wstr2str(oldName) << "=>" << wstr2str(newName);
		return;
	}

	if (auto p = std::dynamic_pointer_cast<CFileRow>(iter->DataPtr)) {
		ULONG chEaten;
		ULONG dwAttributes;
		CIDL newIdl;

		HRESULT hRes = p->GetFilePointer()->GetParentShellFolderPtr()
			->ParseDisplayName(m_hWnd, NULL, (LPWSTR)newName.c_str(), &chEaten, newIdl.ptrptr(), &dwAttributes);

		if (SUCCEEDED(hRes) && newIdl) {
			p->SetFilePointer(m_spFolder->CreateShExFileFolder(newIdl));
			p->SetMeasureValid(false);
			PostUpdate(Updates::ColumnVisible);
			PostUpdate(Updates::RowVisible);
			PostUpdate(Updates::Row);
			PostUpdate(Updates::Scrolls);
			PostUpdate(Updates::Invalidate);
			PostUpdate(Updates::Sort);
			FilterAll();
			SubmitUpdate();
		}
		else {
			BOOST_LOG_TRIVIAL(trace) << "Renamed FAILED " << wstr2str(oldName) << "=>" << wstr2str(newName);
			return Removed(oldName);
		}
	}
}

void CFilerGridView::OnKeyDown(const KeyDownEvent& e)
{
	switch (e.Char)
	{
	case 'R':
		if (::GetAsyncKeyState(VK_CONTROL)) {
			OpenFolder(m_spFolder);
		}
		break;
	case 'X':
		CutToClipboard();
		break;
	case 'C':
		CopyToClipboard();
		break;
	case 'V':
		PasteFromClipboard();
		break;
	case VK_DELETE:
		Delete();
		break;
	case VK_F8:
		{
			int okcancel = ::MessageBox(m_hWnd, L"Delete?", L"Delete?", MB_OKCANCEL);
			if (okcancel == IDOK) {
				Delete();
			}
		}
		break;
	case 'A':
		if(::GetAsyncKeyState(VK_CONTROL)){
			SendMessage(WM_COMMAND,IDM_SELECTALL,NULL);
		}
		break;
	case 'F':
		if(::GetAsyncKeyState(VK_CONTROL)){
			SendMessage(WM_COMMAND,IDM_FIND,NULL);
		}
		break;

	case VK_RETURN:
		{
			if(m_spCursorer->GetFocusedCell()){
				if(auto p = dynamic_cast<CFileRow*>(m_spCursorer->GetFocusedCell()->GetRowPtr())){
					auto spFile = p->GetFilePointer();
					Open(spFile);
				}
			}

		}
		break;
	case VK_BACK:
		{
			OpenFolder(m_spFolder->GetParent());
		}
		break;
	case VK_F2:
		{
			if (m_spCursorer->GetFocusedCell()) {
				std::static_pointer_cast<CFileNameCell>(CSheet::Cell(m_spCursorer->GetFocusedCell()->GetRowPtr(), m_pNameColumn.get()))->OnEdit(EventArgs(this));
			}
		}
		break;
	case VK_F4:
		{
			auto pCell = m_spCursorer->GetFocusedCell();
			if (!pCell) { break; }
			if (auto p = dynamic_cast<CFileRow*>(pCell->GetRowPtr())) {
				auto spFile = p->GetFilePointer();
				SHELLEXECUTEINFO	sei = { 0 };
				sei.cbSize = sizeof(SHELLEXECUTEINFO);
				//sei.fMask = SEE_MASK_INVOKEIDLIST;
				sei.hwnd = m_hWnd;
				sei.lpVerb = L"open";
				sei.lpFile = L"notepad.exe";
				sei.lpParameters = spFile->GetPath().c_str();//NULL;
				sei.lpDirectory = NULL;
				sei.nShow = SW_SHOWNORMAL;
				sei.hInstApp = NULL;
				//sei.lpIDList = (LPVOID)(spFile->GetAbsolutePidl().m_pIDL);

				::ShellExecuteEx(&sei);
			}
		}
		break;
	case VK_F7:
		NewFolder();
		break;
	default:
		break;
	}

	CGridView::OnKeyDown(e);
};

void CFilerGridView::InsertDefaultRowColumn()
{
	//Row
	m_rowHeader = std::make_shared<CParentHeaderRow>(this);
	m_rowNameHeader=std::make_shared<CParentHeaderRow>(this);
	m_rowFilter=std::make_shared<CParentRow>(this);

	InsertRowNotify(CRow::kMinIndex, m_rowFilter);
	InsertRowNotify(CRow::kMinIndex,m_rowNameHeader);
	InsertRowNotify(CRow::kMinIndex, m_rowHeader);

}

void CFilerGridView::Open(std::shared_ptr<CShellFile>& spFile)
{
	if (auto spFolder = std::dynamic_pointer_cast<CShellFolder>(spFile)) {
		OpenFolder(spFolder);
	}
	else {
		OpenFile(spFile);
	}
	::SetFocus(m_hWnd);
}

void CFilerGridView::OpenFile(std::shared_ptr<CShellFile>& spFile)
{
	SHELLEXECUTEINFO	sei = {0};
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

void CFilerGridView::OpenFolder(std::shared_ptr<CShellFolder>& spFolder)
{
	BOOST_LOG_TRIVIAL(trace) << "CFilerGridView::OpenFolder : " + wstr2str(spFolder->GetFileName());

	CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, L"OpenFolder Total")
	bool isUpdate = m_spFolder == spFolder;
	{
		CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, L"OpenFolder Pre-Process")

		//Save and Restore Filter value
		if (!isUpdate) {
			//Update Map
			std::unordered_map<std::shared_ptr<CColumn>, std::wstring> map;
			bool isAllEmpty = true;
			for (auto iter = m_columnAllDictionary.begin(); iter != m_columnAllDictionary.end(); ++iter) {
				isAllEmpty &= iter->DataPtr->GetFilter().empty();
				if (!isAllEmpty)break;
			}
			if (isAllEmpty) {
				if (m_spFolder) {
					m_filterMap.erase(m_spFolder->GetPath());
				}
			} else {
				for (auto iter = m_columnAllDictionary.begin(); iter != m_columnAllDictionary.end(); ++iter) {
					map.emplace(iter->DataPtr, iter->DataPtr->GetFilter());
				}
				if (m_spFolder) {
					m_filterMap.insert_or_assign(m_spFolder->GetPath(), map);
				}
			}
			//Load Map
			auto iter = m_filterMap.find(spFolder->GetPath());
			if (iter != m_filterMap.end()) {
				//Load filter from map
				for (auto pr : iter->second) {
					pr.first->SetFilter(pr.second);
				}
			} else {
				//Clear filter
				for (auto iter = m_columnAllDictionary.begin(); iter != m_columnAllDictionary.end(); ++iter) {
					iter->DataPtr->SetFilter(L"");
				}
			}
			//Update
			m_spPreviousFolder = m_spFolder;
			m_spFolder = spFolder;
		}

		if (Empty()) {
			InsertDefaultRowColumn();
		}

		//Clear RowDictionary From 0 to last
		auto& rowDictionary = m_rowAllDictionary.get<IndexTag>();
		rowDictionary.erase(rowDictionary.find(0), rowDictionary.end());
		//Set up Watcher
		try {
			if (::PathFileExists(m_spFolder->GetPath().c_str()) &&
				!boost::iequals(m_spFolder->GetExt(), L".zip")) {
				if (m_spWatcher->GetPath() != m_spFolder->GetPath()) {
					m_spWatcher->QuitWatching();
					m_spWatcher->SetPath(m_spFolder->GetPath());
					m_spWatcher->StartWatching();
				}
			} else {
				m_spWatcher->QuitWatching();
				m_spWatcher->SetPath(L"");
			}
		} catch (std::exception& e) {
			MessageBox(L"Watcher", L"Error", 0);
			throw e;
		}
	}
	{
		CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, L"OpenFolder Enumeration")
			try {
			//Enumerate child IDL

			CComPtr<IEnumIDList> enumIdl;
			if (SUCCEEDED(m_spFolder->GetShellFolderPtr()->EnumObjects(m_hWnd, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &enumIdl)) &&
				enumIdl) {
				CIDL nextIdl;
				ULONG ulRet(0);
				while (SUCCEEDED(enumIdl->Next(1, nextIdl.ptrptr(), &ulRet))) {
					if (!nextIdl) { break; }
					if (auto spFile = m_spFolder->CreateShExFileFolder(nextIdl)) {
						InsertRow(CRow::kMaxIndex, std::make_shared<CFileRow>(this, spFile));
					}
					nextIdl.Clear();
				}
			}
		} catch (std::exception&) {
			MessageBox(L"Enumeration", L"Error", 0);
			//		throw e;
		}
	}
	{
		CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, L"OpenFolder Updating")
		//Path change //TODO
		//m_rowHeader->SetMeasureValid(false);
		//m_rowNameHeader->SetMeasureValid(false);
		//m_rowFilter->SetMeasureValid(false);
		//for (const auto& row : m_rowAllDictionary) {
		//	row.DataPtr->SetMeasureValid(false);
		//}
		for (const auto& col : m_columnAllDictionary) {
			std::dynamic_pointer_cast<CParentMapColumn>(col.DataPtr)->Clear();
			//col.DataPtr->SetMeasureValid(false);
		}
		//for (const auto& row : m_rowAllDictionary) {
		//	for (const auto& col : m_columnAllDictionary) {
		//		auto cell = CSheet::Cell(row.DataPtr, col.DataPtr);
		//		cell->SetActMeasureValid(false);
		//		cell->SetFitMeasureValid(false);
		//	}
		//}

		//PathCell
		auto pPathCell = CSheet::Cell(m_rowHeader, m_pNameColumn);
		pPathCell->OnPropertyChanged(L"value");

		PostUpdate(Updates::Sort);
		PostUpdate(Updates::Filter);
		PostUpdate(Updates::ColumnVisible);
		PostUpdate(Updates::RowVisible);
		//PostUpdate(Updates::Column);
		PostUpdate(Updates::Row);
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);

		if (!isUpdate) {
			FolderChanged(m_spFolder);
		}

		//::OutputDebugStringA("m_rowAllDictionary\r\n");
		//boost::range::for_each(m_rowAllDictionary, [](const RowData& data) {
		//	::OutputDebugStringA((boost::format("Display:%1%, Pointer:%2%\r\n") % data.Index%data.DataPtr.get()).str().c_str());
		//});

		//Celler
		m_spCeller->Clear();

		//Cursor
		//Clear Cursor
		m_spCursorer->Clear();

		if (!isUpdate) {
			//If previous folder is found, set cursor for that.
			if (m_spPreviousFolder) {
				auto iter = std::find_if(m_rowAllDictionary.begin(), m_rowAllDictionary.end(), [this](const auto& rowData)->bool {
					if (auto spFileRow = std::dynamic_pointer_cast<CFileRow>(rowData.DataPtr)) {
						return spFileRow->GetFilePointer()->GetAbsoluteIdl() == m_spPreviousFolder->GetAbsoluteIdl();
					} else {
						return false;
					}
				});

				if (iter != m_rowAllDictionary.end()) {

					if (auto cell = CSheet::Cell(iter->DataPtr, Index2Pointer<ColTag, VisTag>(0))) {
						m_spCursorer->OnCursor(cell);
						PostUpdate(Updates::EnsureVisibleFocusedCell);
					}
				}
			}

		}

		SubmitUpdate();
	}
}

void CFilerGridView::OnBkGndLButtondDblClk(const LButtonDblClkEvent& e)
{
	OpenFolder(m_spFolder->GetParent());
}

void CFilerGridView::OnCellLButtonDblClk(CellEventArgs& e)
{
	auto pCell = e.CellPtr;
	if(auto p = dynamic_cast<CFileRow*>(e.CellPtr->GetRowPtr())){
		auto spFile = p->GetFilePointer();
		Open(spFile);
	}
}

std::vector<LPITEMIDLIST> CFilerGridView::GetSelectedLastPIDLVector()
{
	auto& rowDictionary = m_rowVisibleDictionary.get<IndexTag>();
	std::vector<LPITEMIDLIST> vPidl;
	for (auto rowIter = rowDictionary.begin(), rowEnd = rowDictionary.end(); rowIter != rowEnd; ++rowIter) {
		if (rowIter->DataPtr->GetSelected()) {
			if (auto spRow = std::dynamic_pointer_cast<CFileRow>(rowIter->DataPtr)) {
				auto spFile = spRow->GetFilePointer();
				vPidl.push_back(spFile->GetAbsoluteIdl().FindLastID());
			}
		}
	}
	return vPidl;
}

std::vector<LPITEMIDLIST> CFilerGridView::GetSelectedAbsolutePIDLVector()
{
	auto& rowDictionary = m_rowVisibleDictionary.get<IndexTag>();
	std::vector<LPITEMIDLIST> vPidl;
	for (auto rowIter = rowDictionary.begin(), rowEnd = rowDictionary.end(); rowIter != rowEnd; ++rowIter) {
		if (rowIter->DataPtr->GetSelected()) {
			if (auto spRow = std::dynamic_pointer_cast<CFileRow>(rowIter->DataPtr)) {
				auto spFile = spRow->GetFilePointer();
				vPidl.push_back(spFile->GetAbsoluteIdl().m_pIDL);
			}
		}
	}
	return vPidl;
}

bool CFilerGridView::CopyTo(CComPtr<IShellItem2> pDestItem)
{

	auto vPidl = GetSelectedAbsolutePIDLVector();
	CComPtr<IShellItemArray> pItemAry = nullptr;
	HRESULT hr = ::SHCreateShellItemArrayFromIDLists(vPidl.size(), (LPCITEMIDLIST*)(vPidl.data()), &pItemAry);
	if (FAILED(hr)) { return false;}

	CComPtr<IFileOperation> pFileOperation;

	hr = pFileOperation.CoCreateInstance(CLSID_FileOperation);
	if (FAILED(hr)) { return false; }
	hr = pFileOperation->CopyItems(pItemAry, pDestItem);
	if (FAILED(hr)) { return false; }
	hr = pFileOperation->PerformOperations();
	return SUCCEEDED(hr);
}

bool CFilerGridView::MoveTo(CComPtr<IShellItem2> pDestItem)
{
	auto vPidl = GetSelectedAbsolutePIDLVector();
	CComPtr<IShellItemArray> pItemAry = nullptr;
	HRESULT hr = ::SHCreateShellItemArrayFromIDLists(vPidl.size(), (LPCITEMIDLIST*)(vPidl.data()), &pItemAry);
	if (FAILED(hr)) { return false; }

	CComPtr<IFileOperation> pFileOperation;

	hr = pFileOperation.CoCreateInstance(CLSID_FileOperation);
	if (FAILED(hr)) { return false; }
	hr = pFileOperation->MoveItems(pItemAry, pDestItem);
	if (FAILED(hr)) { return false; }
	hr = pFileOperation->PerformOperations();
	return SUCCEEDED(hr);
}


bool CFilerGridView::NewFolder()
{
	return InvokeNewShellContextmenuCommand(m_hWnd, CMDSTR_NEWFOLDERA, m_spFolder->GetShellFolderPtr());
}

bool CFilerGridView::CutToClipboard()
{
	return InvokeNormalShellContextmenuCommand(m_hWnd, "Cut", m_spFolder->GetShellFolderPtr(), GetSelectedLastPIDLVector());
}

bool CFilerGridView::CopyToClipboard()
{
	return InvokeNormalShellContextmenuCommand(m_hWnd, "Copy", m_spFolder->GetShellFolderPtr(), GetSelectedLastPIDLVector());
}

bool CFilerGridView::PasteFromClipboard()
{
	std::vector<LPITEMIDLIST> vPidl;
	CIDL idl = m_spFolder->GetAbsoluteIdl().CloneParentIDL();
	CComPtr<IShellFolder> pDesktop;
	::SHGetDesktopFolder(&pDesktop);
	CComPtr<IShellFolder> pFolder;
	::SHBindToObject(pDesktop,idl.ptr(),0,IID_IShellFolder,(void**)&pFolder);
	if(!pFolder){
		pFolder = pDesktop;
	}
	vPidl.push_back(m_spFolder->GetAbsoluteIdl().FindLastID());
	return InvokeNormalShellContextmenuCommand(m_hWnd, "Paste", pFolder, vPidl);
}

bool CFilerGridView::Delete()
{
	return InvokeNormalShellContextmenuCommand(m_hWnd, "Delete", m_spFolder->GetShellFolderPtr(), GetSelectedLastPIDLVector());
}

LRESULT CFilerGridView::OnCommandCut(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
{
	CutToClipboard();
	return 0;
}

LRESULT CFilerGridView::OnCommandCopy(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
{
	CopyToClipboard();
	return 0;
}
LRESULT CFilerGridView::OnCommandPaste(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
{
	PasteFromClipboard();
	return 0;
}
LRESULT CFilerGridView::OnCommandDelete(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
{
	Delete();
	return 0;
}

LRESULT CFilerGridView::OnDirectoryWatch(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	auto pInfo0 = reinterpret_cast<PFILE_NOTIFY_INFORMATION>(wParam);
	auto pInfo = pInfo0;
	while (true) {
		std::wstring fileName;
		memcpy(::GetBuffer(fileName, pInfo->FileNameLength / sizeof(wchar_t)), pInfo->FileName, pInfo->FileNameLength);
		::ReleaseBuffer(fileName);

		switch (pInfo->Action) {
		case FILE_ACTION_ADDED:
			BOOST_LOG_TRIVIAL(trace) << "FILE_ACTION_ADDED";
			Added(fileName);
			break;
		case FILE_ACTION_MODIFIED:
			BOOST_LOG_TRIVIAL(trace) << "FILE_ACTION_MODIFIED";
			Modified(fileName);
			break;
		case FILE_ACTION_REMOVED:
			BOOST_LOG_TRIVIAL(trace) << "FILE_ACTION_REMOVED";
			Removed(fileName);
			break;
		case FILE_ACTION_RENAMED_NEW_NAME:
			BOOST_LOG_TRIVIAL(trace) << "FILE_ACTION_RENAMED_NEW_NAME";
			if (!m_oldName.empty()) {
				Renamed(m_oldName, fileName);
				m_oldName.clear();
			}
			else {
				FILE_LINE_FUNC_TRACE;
			}
			break;
		case FILE_ACTION_RENAMED_OLD_NAME:
			BOOST_LOG_TRIVIAL(trace) << "FILE_ACTION_RENAMED_OLD_NAME";
			m_oldName = fileName;
			break;
		default:
			break;
		}

		if (pInfo->NextEntryOffset == 0) { break; }
		
		pInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(reinterpret_cast<PBYTE>(pInfo) + pInfo->NextEntryOffset);
	}
	
	delete [] (PBYTE)pInfo0;

	return 0;
}

bool CFilerGridView::InvokeNormalShellContextmenuCommand(HWND hWnd, LPCSTR lpVerb, CComPtr<IShellFolder> psf, std::vector<PITEMID_CHILD> vpIdl)
{
    CComPtr<IContextMenu> pcm;
	HRESULT hr=psf->GetUIObjectOf(hWnd, vpIdl.size(),(LPCITEMIDLIST*)(vpIdl.data()),IID_IContextMenu,nullptr,(LPVOID *)&pcm);
    if(SUCCEEDED(hr)){
		CMenu menu=CMenu(CreatePopupMenu());
		if(!menu.IsNull()){
			hr=pcm->QueryContextMenu(menu,0,1,0x7FFF, CMF_NORMAL);
			if (SUCCEEDED(hr)){
				CMINVOKECOMMANDINFO cmi = {0};
				cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
				cmi.hwnd = hWnd;
				cmi.lpVerb = lpVerb;
				cmi.nShow = SW_SHOWNORMAL;

				hr=pcm->InvokeCommand((LPCMINVOKECOMMANDINFO)&cmi);
			}
		}
    }
	return SUCCEEDED(hr);
}

bool CFilerGridView::InvokeNewShellContextmenuCommand(HWND hWnd, LPCSTR lpVerb, CComPtr<IShellFolder> psf)
{
	CComPtr<IUnknown> puk;
	//{D969A300-E7FF-11d0-A93B-00A0C90F2719},
	CLSID clsid = { 0xd969a300, 0xe7ff, 0x11d0,{ 0xa9, 0x3b, 0x00, 0xa0, 0xc9, 0x0f, 0x27, 0x19 } };
	HRESULT hr = puk.CoCreateInstance(clsid);
	if (FAILED(hr)) { return SUCCEEDED(hr);}
	CComPtr<IShellExtInit> psei;
	hr = puk->QueryInterface(IID_PPV_ARGS(&psei));
	if (FAILED(hr)) { return SUCCEEDED(hr); }
	hr = psei->Initialize(m_spFolder->GetAbsoluteIdl().ptr(), NULL, NULL);
	if (FAILED(hr)) { return SUCCEEDED(hr); }
	hr = psei->QueryInterface(IID_PPV_ARGS(&m_pcmNew2));
	if (FAILED(hr)) { return SUCCEEDED(hr); }
	hr = psei->QueryInterface(IID_PPV_ARGS(&m_pcmNew3));
	if (FAILED(hr)) { return SUCCEEDED(hr);}
	CMenu menu = CMenu(CreatePopupMenu());
	hr = m_pcmNew3->QueryContextMenu(menu, 0, SCRATCH_QCM_NEW, SCRATCH_QCM_LAST, CMF_NORMAL);
	if (FAILED(hr)) { return SUCCEEDED(hr);
	}

	CMINVOKECOMMANDINFO cmi = { 0 };
	cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
	cmi.hwnd = hWnd;
	cmi.lpVerb = lpVerb;
	cmi.nShow = SW_SHOWNORMAL;
	m_bNewFile = true;
	hr = m_pcmNew3->InvokeCommand((LPCMINVOKECOMMANDINFO)&cmi);
	return SUCCEEDED(hr);
}

void CFilerGridView::OnContextMenu(const ContextMenuEvent& e)
{
	CPoint ptScreen(e.Point);
	ClientToScreen(ptScreen);
	auto cell = Cell(e.Point);
	std::vector<PITEMID_CHILD> vPidl;

	if (!cell) {
		//Folder menu
		CIDL idl = m_spFolder->GetAbsoluteIdl().CloneParentIDL();
		CComPtr<IShellFolder> pDesktop;
		::SHGetDesktopFolder(&pDesktop);
		CComPtr<IShellFolder> pFolder;
		::SHBindToObject(pDesktop, idl.ptr(), 0, IID_IShellFolder, (void**)&pFolder);
		if (!pFolder) {
			pFolder = pDesktop;
		}
		vPidl.push_back(m_spFolder->GetAbsoluteIdl().FindLastID());
		ShowShellContextMenu(m_hWnd, ptScreen, pFolder, vPidl, true);
	}else if(cell->GetRowPtr() == m_rowHeader.get() || cell->GetRowPtr() == m_rowNameHeader.get()){
		//Header menu
		CMenu menu(::CreatePopupMenu());
		if (menu.IsNull()) { return; }
		if (m_headerMenuItems.empty()) {
			for (auto iter = m_columnAllDictionary.begin(); iter != m_columnAllDictionary.end(); ++iter) {
				auto cell = CSheet::Cell(m_rowNameHeader, iter->DataPtr);
				auto str = cell->GetString();
				m_headerMenuItems.push_back(std::make_shared<CShowHideColumnMenuItem>(
					IDM_VISIBLEROWHEADERCOLUMN + std::distance(m_columnAllDictionary.begin(), iter),
					Cell(m_rowNameHeader, iter->DataPtr)->GetString(), this, iter->DataPtr.get()));
			}

			for (auto& item : m_headerMenuItems) {
				AddCmdIDHandler(item->GetID(), std::bind(&CMenuItem::OnCommand, item.get(), phs::_1, phs::_2, phs::_3, phs::_4));
			}
		}

		for (auto& item : m_headerMenuItems) {
			menu.InsertMenuItemW(menu.GetMenuItemCount(), TRUE, item.get());
		}

		menu.TrackPopupMenu(
			TPM_LEFTALIGN | TPM_RIGHTBUTTON,
			ptScreen.x,
			ptScreen.y,
			m_hWnd);
	}else{
		//Cell menu
		auto& rowDictionary = m_rowVisibleDictionary.get<IndexTag>();
		auto& colDictionary = m_columnVisibleDictionary.get<IndexTag>();
		for(auto rowIter=rowDictionary.begin(),rowEnd=rowDictionary.end();rowIter!=rowEnd;++rowIter){
			if(rowIter->DataPtr->GetSelected()){
				auto spRow=std::dynamic_pointer_cast<CFileRow>(rowIter->DataPtr);
				auto spFile = spRow->GetFilePointer();
				vPidl.push_back(spFile->GetAbsoluteIdl().FindLastID());
			}
		}
			ShowShellContextMenu(m_hWnd, ptScreen, m_spFolder->GetShellFolderPtr(), vPidl);
	}
}

void CFilerGridView::ShowShellContextMenu(HWND hWnd, CPoint ptScreen, CComPtr<IShellFolder> psf, std::vector<PITEMID_CHILD> vpIdl, bool hasNew)
{
	try {
		AddAllMsgHandler(&CFilerGridView::OnHandleMenuMsg, this);

		//CreateMenu
		CMenu menu(::CreatePopupMenu());
		if (menu.IsNull()) { return; }

		//New Context Menu
		HRESULT hr;
		if (hasNew) {

			CComPtr<IUnknown> puk;
			//{D969A300-E7FF-11d0-A93B-00A0C90F2719},
			CLSID clsid = { 0xd969a300, 0xe7ff, 0x11d0,{ 0xa9, 0x3b, 0x00, 0xa0, 0xc9, 0x0f, 0x27, 0x19 } };
			hr = puk.CoCreateInstance(clsid);
			if (FAILED(hr)) { return; }
			CComPtr<IShellExtInit> psei;
			hr = puk->QueryInterface(IID_PPV_ARGS(&psei));
			if (FAILED(hr)) { return; }
			hr = psei->Initialize(m_spFolder->GetAbsoluteIdl().ptr(), NULL, NULL);
			if (FAILED(hr)) { return; }
			hr = psei->QueryInterface(IID_PPV_ARGS(&m_pcmNew2));
			if (FAILED(hr)) { return; }
			hr = psei->QueryInterface(IID_PPV_ARGS(&m_pcmNew3));
			if (FAILED(hr)) { return; }

			hr = m_pcmNew3->QueryContextMenu(menu, 0, SCRATCH_QCM_NEW, SCRATCH_QCM_LAST, CMF_NORMAL);
			if (FAILED(hr)) { return; }
		}

		//Normal Context Menu
		CComPtr<IContextMenu> pcm;
		hr = psf->GetUIObjectOf(hWnd, vpIdl.size(), (LPCITEMIDLIST*)(vpIdl.data()), IID_IContextMenu, nullptr, (LPVOID *)&pcm);
		hr = pcm->QueryInterface(IID_PPV_ARGS(&m_pcm2));
		if (FAILED(hr)) { return; }
		hr = pcm->QueryInterface(IID_PPV_ARGS(&m_pcm3));
		if (FAILED(hr)) { return; }
		UINT uFlags = (::GetKeyState(VK_SHIFT) & 0x8000) ? CMF_NORMAL | CMF_EXTENDEDVERBS : CMF_NORMAL;
		hr = m_pcm3->QueryContextMenu(menu, 0, SCRATCH_QCM_FIRST, SCRATCH_QCM_LAST, uFlags);
		if (FAILED(hr)) { return; }

		//Add Copy Text
		menu.InsertSeparator(menu.GetMenuItemCount(), TRUE);
		MENUITEMINFO mii = { 0 };
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_TYPE | MIIM_ID;
		mii.fType = MFT_STRING;
		mii.fState = MFS_ENABLED;
		mii.wID = IDM_COPYTEXT;
		mii.dwTypeData = L"Copy Text";
		menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);

		//Add Custom menu
		if (AddCustomContextMenu) {
			AddCustomContextMenu(menu);
		}

		//
		int itemCount = menu.GetMenuItemCount();
		bool isLine(false);

		//Investigate New menu, Seperator
		for (auto i = itemCount - 1; i != 0; i--) {
			std::wstring verb;
			MENUITEMINFO mii = { 0 };
			mii.cbSize = sizeof(MENUITEMINFO);
			mii.fMask = MIIM_SUBMENU | MIIM_TYPE;
			mii.dwTypeData = ::GetBuffer(verb, 256);
			mii.cch = 256;
			menu.GetMenuItemInfo(i, TRUE, &mii);

			//Arrange Separator
			//Avoid Double Separamter, First Separator
			if (mii.fType == MFT_SEPARATOR) {
				if (isLine || i == 0 || i == itemCount - 1) {
					menu.DeleteMenu(i, MF_BYPOSITION);
				}
			}
			isLine = mii.fType == MFT_SEPARATOR;

			//Get Submenu of New
			if (m_pcmNew3) {
				if (verb.find(L"V‹Kì¬") != std::wstring::npos) {
					m_hNewMenu = mii.hSubMenu;
				}
			}
		}
		int idCmd = menu.TrackPopupMenu(
			TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON,
			ptScreen.x,
			ptScreen.y,
			m_hWnd);

		if (idCmd) {

			CMINVOKECOMMANDINFOEX info = { 0 };
			info.cbSize = sizeof(info);
			info.fMask = /*CMIC_MASK_UNICODE |*/ CMIC_MASK_PTINVOKE;
			info.hwnd = hWnd;
			info.nShow = SW_SHOWNORMAL;
			info.ptInvoke = ptScreen;

			if (ExecCustomContextMenu(idCmd, psf, vpIdl)) {
			} else if (idCmd == IDM_COPYTEXT) {
				BOOL bHandled = FALSE;
				CGridView::OnCommandCopy(0, idCmd, m_hWnd, bHandled);
			}else if (idCmd >= SCRATCH_QCM_NEW) {
				info.lpVerb = MAKEINTRESOURCEA(idCmd - SCRATCH_QCM_NEW);
				info.lpVerbW = MAKEINTRESOURCEW(idCmd - SCRATCH_QCM_NEW);
				m_bNewFile = true;
				hr = m_pcmNew3->InvokeCommand((LPCMINVOKECOMMANDINFO)&info);
			}else {
				info.lpVerb = MAKEINTRESOURCEA(idCmd - SCRATCH_QCM_FIRST);
				info.lpVerbW = MAKEINTRESOURCEW(idCmd - SCRATCH_QCM_FIRST);
				hr = m_pcm3->InvokeCommand((LPCMINVOKECOMMANDINFO)&info);
			}
		}
	}
	catch (...) {
		m_hNewMenu = NULL;
		m_pcmNew2 = nullptr;
		m_pcmNew3 = nullptr;
		m_pcm2 = nullptr;
		m_pcm3 = nullptr;
		RemoveAllMsgHandler();
		throw;
	}

	m_hNewMenu = NULL;
	m_pcmNew2 = nullptr;
	m_pcmNew3 = nullptr;
	m_pcm2 = nullptr;
	m_pcm3 = nullptr;
	RemoveAllMsgHandler();
}

HRESULT CFilerGridView::OnHandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if ((m_pcmNew3 || m_pcmNew2) && wParam == (WPARAM)m_hNewMenu) {
		if (m_pcmNew3) {
			LRESULT lres;
			if (SUCCEEDED(m_pcmNew3->HandleMenuMsg2(uMsg, wParam, lParam, &lres))) {
				bHandled = TRUE;
				return lres;
			}
		}
		else if (m_pcmNew2) {
			if (SUCCEEDED(m_pcmNew2->HandleMenuMsg(uMsg, wParam, lParam))) {
				bHandled = TRUE;
				return 0;
			}
		}
	}else{
		if (m_pcm3) {
			LRESULT lres;
			if (SUCCEEDED(m_pcm3->HandleMenuMsg2(uMsg, wParam, lParam, &lres))) {
				bHandled = TRUE;
				return lres;
			}
		}
		else if (m_pcm2) {
			if (SUCCEEDED(m_pcm2->HandleMenuMsg(uMsg, wParam, lParam))) {
				bHandled = TRUE;
				return 0;
			}
		}
	}

	bHandled = FALSE;
	return 0;
}

CFilerGridView::string_type CFilerGridView::GetPath()const
{
	return m_spFolder->GetPath();
}

void CFilerGridView::SetPath(const string_type& path)
{
	if (auto pFolder = std::dynamic_pointer_cast<CShellFolder>(CShellFolder::CreateShExFileFolder(path))) {
		OpenFolder(pFolder);
	}
	else {
		//Do nothing
	}
}

void CFilerGridView::Drag()
{
	auto& rowDictionary=m_rowVisibleDictionary.get<IndexTag>();
	auto& colDictionary=m_columnVisibleDictionary.get<IndexTag>();

	std::vector<LPITEMIDLIST> vPidl;
	CIDL pFirstIdl;
	for(auto rowIter=rowDictionary.begin(),rowEnd=rowDictionary.end();rowIter!=rowEnd;++rowIter){
		if(rowIter->DataPtr->GetSelected()){
			if(auto spRow=std::dynamic_pointer_cast<CFileRow>(rowIter->DataPtr)){
				auto spFile = spRow->GetFilePointer();
				if(vPidl.empty()){
					pFirstIdl = spFile->GetAbsoluteIdl();
				}
				vPidl.push_back(spFile->GetAbsoluteIdl().FindLastID());
			}
		}
	}
	CComPtr<IDataObject> pDataObject;
	m_spFolder->GetShellFolderPtr()->GetUIObjectOf(NULL, vPidl.size(), (LPCITEMIDLIST *)vPidl.data(), IID_IDataObject, NULL, (void **)&pDataObject);
			
	SetDragImage(pFirstIdl, m_pDragSourceHelper, pDataObject);
	DWORD dwEffect;
	DoDragDrop(pDataObject, m_pDropSource, DROPEFFECT_MOVE | DROPEFFECT_COPY | DROPEFFECT_LINK, &dwEffect);
}

BOOL CFilerGridView::SetDragImage(CIDL firstIdl, CComPtr<IDragSourceHelper> pDragSourceHelper, IDataObject *pDataObject)
{
	SHFILEINFO  fileInfo ={0};
	SHDRAGIMAGE dragImage = {0};
	POINT       pt = {0, 0};
	SIZE        size = {32, 32};

	SHGetFileInfo((LPCTSTR)firstIdl.ptr(), 0, &fileInfo, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_ICON | SHGFI_LARGEICON);


	//CClientDC dc(m_hWnd);
	//CBufferDC dcBuff(dc, size.cx, size.cy);
	//dcBuff.DrawIconEx(fileInfo.hIcon,CRect(0, 0, size.cx, size.cy),0,NULL,DI_NORMAL);
	////::DrawIcon(dcBuff, 0, 0, fileInfo.hIcon);
	//DestroyIcon(fileInfo.hIcon);

	//dragImage.sizeDragImage = size;
	//dragImage.ptOffset = pt;
	//dragImage.hbmpDragImage = (HBITMAP)dcBuff.GetBitMap();
	//dragImage.crColorKey = RGB(0, 0, 0);
	//HRESULT hr = pDragSourceHelper->InitializeFromBitmap(&dragImage, pDataObject);

	HDC hdc = ::GetDC(m_hWnd);
	HDC hdcMem = ::CreateCompatibleDC(hdc);
	HBITMAP hbmp = ::CreateCompatibleBitmap(hdc, size.cx, size.cy);
	HBITMAP hbmpPrev = (HBITMAP)::SelectObject(hdcMem, hbmp);
	::DrawIcon(hdcMem, 0, 0, fileInfo.hIcon);
	::SelectObject(hdcMem, hbmpPrev);
	::DeleteDC(hdcMem);
	::ReleaseDC(m_hWnd, hdc);
	::DestroyIcon(fileInfo.hIcon);

	dragImage.sizeDragImage = size;
	dragImage.ptOffset = pt;
	dragImage.hbmpDragImage = hbmp;
	dragImage.crColorKey = RGB(0, 0, 0);
	HRESULT hr = pDragSourceHelper->InitializeFromBitmap(&dragImage, pDataObject);

	return hr == S_OK;
}