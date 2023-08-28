#include "FilerGridView.h"
#include "GridViewStateMachine.h"
#include "ShellFile.h"
#include "ShellFolder.h"
#include "ShellFunction.h"
#include "Row.h"
#include "Column.h"
#include "Cell.h"
#include "RowIndexColumn.h"
#include "FileColumn.h"
#include "FileSizeColumn.h"
#include "FileLastWriteColumn.h"
#include "IImageColumn.h"
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
#include "SearchWnd.h"
#include "Textbox.h"
#include "D2DWWindow.h"
#include "FileOperationDlg.h"
#include "DropTargetManager.h"

#include "MouseStateMachine.h"
#include "FilerWnd.h"
#include <format>

#include "ResourceIDFactory.h"

#define SCRATCH_QCM_FIRST 1
#define SCRATCH_QCM_NEW 600//200,500 are used by system
#define SCRATCH_QCM_LAST  0x7FFF

CLIPFORMAT CFilerGridView::s_cf_shellidlist = ::RegisterClipboardFormat(CFSTR_SHELLIDLIST);
CLIPFORMAT CFilerGridView::s_cf_filecontents = ::RegisterClipboardFormat(CFSTR_FILECONTENTS);
CLIPFORMAT CFilerGridView::s_cf_filegroupdescriptor = ::RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);

CLIPFORMAT CFilerGridView::s_cf_renprivatemessages = ::RegisterClipboardFormat(L"RenPrivateMessages");

#include <mapix.h>
#include <mapitags.h>
#include <mapidefs.h>
#include <mapiutil.h>
#include <imessage.h>

#define SETFormatEtc(fe, cf, asp, td, med, li)   \
    {\
    (fe).cfFormat=cf;\
    (fe).dwAspect=asp;\
    (fe).ptd=td;\
    (fe).tymed=med;\
    (fe).lindex=li;\
    }

#define SETDefFormatEtc(fe, cf, med)   \
    {\
    (fe).cfFormat=cf;\
    (fe).dwAspect=DVASPECT_CONTENT;\
    (fe).ptd=NULL;\
    (fe).tymed=med;\
    (fe).lindex=-1;\
    }


CFilerGridView::CFilerGridView(CD2DWControl* pParentControl, const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp)
	:CFilerBindGridView(pParentControl, spFilerGridViewProp)
{
	m_commandMap.emplace(IDM_SELECTALL, std::bind(&CFilerGridView::OnCommandSelectAll, this, phs::_1));
	m_commandMap.emplace(IDM_CUT, std::bind(&CFilerGridView::OnCommandCut, this, phs::_1));
	m_commandMap.emplace(IDM_COPY, std::bind(&CFilerGridView::OnCommandCopy, this, phs::_1));
	m_commandMap.emplace(IDM_PASTE, std::bind(&CFilerGridView::OnCommandPaste, this, phs::_1));
	m_commandMap.emplace(IDM_DELETE, std::bind(&CFilerGridView::OnCommandDelete, this, phs::_1));
	m_commandMap.emplace(IDM_FIND, std::bind(&CFilerGridView::OnCommandFind, this, phs::_1));

	m_spItemDragger = std::make_shared<CFileDragger>();
}

CFilerGridView::~CFilerGridView()
{
	if (m_spWatcher) { m_spWatcher->QuitWatching(); }
}

void CFilerGridView::OnCreate(const CreateEvt& e)
{
	//Base Create
	CFilerBindGridView::OnCreate(e);

	//Directory watcher
	m_spWatcher = std::make_shared<CDirectoryWatcher>(this, std::bind(&CFilerGridView::OnDirectoryWatch, this, phs::_1));

	//Drag & Drop
	//DropTarget
	auto pDropTarget = new CDropTarget(this);
	pDropTarget->IsDroppable = ([this](const std::vector<FORMATETC>& formats)->bool { return IsDroppable(formats); });
	pDropTarget->Dropped = ([this](IDataObject *pDataObj, DWORD dwEffect)->void { Dropped(pDataObj, dwEffect); });
	m_pDropTarget = CComPtr<IDropTarget>(pDropTarget);
	GetWndPtr()->GetDropTargetManagerPtr()->RegisterDragDrop(this, m_pDropTarget);
	//DropSource
	m_pDropSource = CComPtr<IDropSource>(new CDropSource);
	m_pDragSourceHelper.CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER);

	//Insert rows
	m_pHeaderRow = std::make_shared<CPathRow>(this, GetHeaderProperty());
	m_pNameHeaderRow = std::make_shared<CHeaderRow>(this, GetHeaderProperty());
	m_pFilterRow = std::make_shared<CRow>(this, GetCellProperty());

	m_allRows.idx_push_back(m_pHeaderRow);
	m_allRows.idx_push_back(m_pNameHeaderRow);
	m_allRows.idx_push_back(m_pFilterRow);

	m_frozenRowCount = 3;

	//Insert columns if not initialized

	
	//if (m_allCols.empty()) {

		if (!std::any_of(m_allCols.cbegin(), m_allCols.cend(), [](const std::shared_ptr<CColumn>& pCol) { return typeid(*pCol) == typeid(CRowIndexColumn); })) 
		{ 
			m_pHeaderColumn = std::make_shared<CRowIndexColumn>(this, GetHeaderProperty());
			m_allCols.idx_push_back(m_pHeaderColumn);
		}
		if (!std::any_of(m_allCols.cbegin(), m_allCols.cend(), [](const std::shared_ptr<CColumn>& pCol) { return typeid(*pCol) == typeid(CFileNameColumn<std::shared_ptr<CShellFile>>); })) 
		{ 
			m_pNameColumn = std::make_shared<CFileNameColumn<std::shared_ptr<CShellFile>>>(this, L"Name");
			m_allCols.idx_push_back(m_pNameColumn);
		}
		//if (!std::any_of(m_allCols.cbegin(), m_allCols.cend(), [](const std::shared_ptr<CColumn>& pCol) { return typeid(*pCol) == typeid(CFileNameColumn<std::shared_ptr<CShellFile>>); })) {
		//	m_allCols.idx_push_back(std::make_shared<CFileNameColumn<std::shared_ptr<CShellFile>>>(this));
		//}
		if (!std::any_of(m_allCols.cbegin(), m_allCols.cend(), [](const std::shared_ptr<CColumn>& pCol) { return typeid(*pCol) == typeid(CFileDispExtColumn<std::shared_ptr<CShellFile>>); })) {
			m_allCols.idx_push_back(std::make_shared<CFileDispExtColumn<std::shared_ptr<CShellFile>>>(this, L"Ext"));
		}
		if (!std::any_of(m_allCols.cbegin(), m_allCols.cend(), [](const std::shared_ptr<CColumn>& pCol) { return typeid(*pCol) == typeid(CFileSizeColumn<std::shared_ptr<CShellFile>>); })) {
			m_allCols.idx_push_back(std::make_shared<CFileSizeColumn<std::shared_ptr<CShellFile>>>(this, GetFilerGridViewPropPtr()->FileSizeArgsPtr));
		}
		if (!std::any_of(m_allCols.cbegin(), m_allCols.cend(), [](const std::shared_ptr<CColumn>& pCol) { return typeid(*pCol) == typeid(CFileLastWriteColumn<std::shared_ptr<CShellFile>>); })) {
			m_allCols.idx_push_back(std::make_shared<CFileLastWriteColumn<std::shared_ptr<CShellFile>>>(this, GetFilerGridViewPropPtr()->FileTimeArgsPtr));
		}

		m_frozenColumnCount = 1;
	//}

	//Header menu items
	//for (auto iter = m_columnAllDictionary.begin(); iter != m_columnAllDictionary.end(); ++iter) {
	//	auto cell = Cell(m_rowNameHeader, iter->DataPtr);
	//	auto str = cell->GetString();
	//	m_headerMenuItems.push_back(std::make_shared<CShowHideColumnMenuItem>(
	//		IDM_VISIBLEROWHEADERCOLUMN + std::distance(m_columnAllDictionary.begin(), iter),
	//		Cell(m_rowNameHeader, iter->DataPtr)->GetString(), this, iter->DataPtr.get()));
	//}

	//for (auto& item : m_headerMenuItems) {
	//	AddCmdIDHandler(item->GetID(), std::bind(&CMenuItem::OnCommand, item.get(), phs::_1, phs::_2, phs::_3, phs::_4));
	//}
}

bool CFilerGridView::IsDroppable(const std::vector<FORMATETC>& formats)
{
	for (auto f : formats) {
		std::wstring str;
		::GetClipboardFormatNameW(f.cfFormat, ::GetBuffer(str, MAX_PATH), MAX_PATH);
		::ReleaseBuffer(str);
		::OutputDebugStringW(std::format(L"{} : {}", f.cfFormat, str).c_str());
	}

	bool isShellIdList = false;
	bool isFileDescriptor = false;

	for (const auto& format : formats) {
		isShellIdList |= format.cfFormat == s_cf_shellidlist;
		isFileDescriptor |= format.cfFormat == s_cf_filegroupdescriptor;
	}
	return  isShellIdList || isFileDescriptor;
}

void CFilerGridView::Dropped(IDataObject *pDataObj, DWORD dwEffect)
{
	//When DropTarget Dropped, LButtonUp is not Fired. Therefore need to cal here to change state.
	//TODOLOW
	auto pt = GetWndPtr()->GetCursorPosInClient();
	GetWndPtr()->SendMessage(WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(pt.x, pt.y));
	//m_pMouseMachine->process_event(LButtonUpEvent(this, NULL, NULL));

	std::vector<FORMATETC> formats;
	CComPtr<IEnumFORMATETC> pEnumFormatEtc;
	if (SUCCEEDED(pDataObj->EnumFormatEtc(DATADIR::DATADIR_GET, &pEnumFormatEtc))) {
		FORMATETC rgelt[100];
		ULONG celtFetched = 0UL;
		if (SUCCEEDED(pEnumFormatEtc->Next(100, rgelt, &celtFetched))) {
			for (size_t i = 0; i < celtFetched; ++i) {
				formats.push_back(rgelt[i]);
			}
		}
	}
	bool isShellIdList = false;
	bool isFileDescriptor = false;
	bool isRenPrivateMessages = false;

	for (const auto& format : formats) {
		isShellIdList |= format.cfFormat == s_cf_shellidlist;
		isFileDescriptor |= format.cfFormat == s_cf_filegroupdescriptor;
		isRenPrivateMessages |= format.cfFormat == s_cf_renprivatemessages;
	}

	auto medium_global_deleter = [](LPSTGMEDIUM pMedium)
	{
		::GlobalUnlock(pMedium->hGlobal);
		::ReleaseStgMedium(pMedium);
		delete pMedium;
	};

	auto medium_deleter = [](LPSTGMEDIUM pMedium)
	{
		::ReleaseStgMedium(pMedium);
		delete pMedium;
	};

	auto message_deleter = [](LPMESSAGE pMessage)
	{
		pMessage->Release();
	};

	if (isShellIdList) {
		// General
		CComPtr<IShellItem2> pDestShellItem;
		CComPtr<IFileOperation> pFileOperation;

		FAILED_RETURN(::SHCreateItemFromIDList(m_spFolder->GetAbsoluteIdl().ptr(), IID_IShellItem2, reinterpret_cast<LPVOID*>(&pDestShellItem)));

		FAILED_RETURN(pFileOperation.CoCreateInstance(CLSID_FileOperation));

		FORMATETC formatetc = { 0 };
		formatetc.cfFormat = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_SHELLIDLIST);
		formatetc.ptd = NULL;
		formatetc.dwAspect = DVASPECT_CONTENT;
		formatetc.lindex = -1;
		formatetc.tymed = TYMED_HGLOBAL;

		std::unique_ptr<STGMEDIUM, decltype(medium_global_deleter)> pMedium(new STGMEDIUM(), medium_global_deleter);
		FAILED_RETURN(pDataObj->GetData(&formatetc, pMedium.get()));

		LPIDA pida = (LPIDA)GlobalLock(pMedium->hGlobal);
		CIDL folderIdl(::ILCloneFull((LPCITEMIDLIST)(((LPBYTE)pida) + (pida)->aoffset[0])));

		switch (dwEffect) {
		case DROPEFFECT_MOVE:
		{
			//if folder is same, do not need to move
			if (folderIdl == m_spFolder->GetAbsoluteIdl()) {
				//Do nothing
			} else {
				FAILED_RETURN(pFileOperation->MoveItems(pDataObj, pDestShellItem));
				FAILED_RETURN(pFileOperation->PerformOperations());
			}
		}
		break;
		case DROPEFFECT_COPY:
		{
			//if folder is same, do not need to move
			if (folderIdl == m_spFolder->GetAbsoluteIdl()) {
				//Do nothing
			} else {				
				FAILED_RETURN(pFileOperation->CopyItems(pDataObj, pDestShellItem));
				FAILED_RETURN(pFileOperation->PerformOperations());
			}
		}
		break;
		case DROPEFFECT_LINK:
		{
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
		}
		break;
		default:
			break;
		}

	} else if (isFileDescriptor && isRenPrivateMessages) {
		// Outlook mail
		auto mapi_deleter = [](MAPIINIT_0* pMapi)
		{
			::MAPIUninitialize();
			delete pMapi;
		};
		std::unique_ptr<MAPIINIT_0, decltype(mapi_deleter)> pMapi(new MAPIINIT_0{ MAPI_INIT_VERSION, MAPI_MULTITHREAD_NOTIFICATIONS }, mapi_deleter);
		FAILED_RETURN(::MAPIInitialize(pMapi.get()));

		FORMATETC descriptor_format = { s_cf_filegroupdescriptor, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		std::unique_ptr<STGMEDIUM, decltype(medium_global_deleter)> pMedium(new STGMEDIUM(), medium_global_deleter);
		FAILED_RETURN(pDataObj->GetData(&descriptor_format, pMedium.get()));
		LPFILEGROUPDESCRIPTOR lpfgd = (LPFILEGROUPDESCRIPTOR)GlobalLock(pMedium->hGlobal);

		if (lpfgd != NULL) {
			LPMALLOC	lpMalloc = MAPIGetDefaultMalloc();
			auto msgsess_deleter = [](LPMSGSESS ptr)
			{
				CloseIMsgSession(ptr);
			};
			LPMSGSESS pTmp = nullptr;
			FAILED_RETURN(OpenIMsgSession(lpMalloc, NULL, &pTmp));
			std::unique_ptr<std::remove_pointer<LPMSGSESS>::type, decltype(msgsess_deleter)> pSess(pTmp, msgsess_deleter);

			if (pSess) {
				CComPtr<IFileOperation> pFileOperation;
				FAILED_RETURN(pFileOperation.CoCreateInstance(CLSID_FileOperation));
				for (UINT i = 0; i < lpfgd->cItems; i++) {
					LPFILEDESCRIPTOR lpfd = (LPFILEDESCRIPTOR)&lpfgd->fgd[i];

					FORMATETC contents_format = { s_cf_filecontents, NULL, DVASPECT_CONTENT, (LONG)i, TYMED_ISTORAGE };
					std::unique_ptr<STGMEDIUM, decltype(medium_deleter)> pMedium(new STGMEDIUM(), medium_deleter);
					FAILED_RETURN(pDataObj->GetData(&contents_format, pMedium.get()));

					if ( pMedium->pstg != NULL) {
						LPMESSAGE	pTmp = NULL;
						FAILED_RETURN(OpenIMsgOnIStg(pSess.get(), MAPIAllocateBuffer, MAPIAllocateMore, MAPIFreeBuffer, lpMalloc, NULL, pMedium->pstg, NULL, 0, 0, &pTmp));
						std::unique_ptr<std::remove_pointer<LPMESSAGE>::type, decltype(message_deleter)> pMsg(pTmp, message_deleter);
						if (pMsg) {
							CDropTarget::CopyMessage(pFileOperation, m_spFolder, pMsg.get());
						}
					}
					pFileOperation->PerformOperations();
				}
			}
		}
	}else if(isFileDescriptor){
		//From outlook or notes
		//Set up format structure for the descriptor and contents
		FORMATETC descriptor_format = { s_cf_filegroupdescriptor, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		FORMATETC contents_format = { s_cf_filecontents,   NULL, DVASPECT_CONTENT, -1, TYMED_ISTREAM }; //TYMED_ISTREAM means use stream as a tempoary meory instead of RAM's global memory

		// Get the descriptor information
		std::unique_ptr<STGMEDIUM, decltype(medium_global_deleter)> pMedium(new STGMEDIUM(), medium_global_deleter);
		FAILED_RETURN(pDataObj->GetData(&descriptor_format, pMedium.get()));
		FILEGROUPDESCRIPTOR* file_group_descriptor = (FILEGROUPDESCRIPTOR*)GlobalLock(pMedium->hGlobal);

		// For each file, get the name and copy the stream to a file
		CComPtr<IFileOperation> pFileOperation;
		FAILED_RETURN(pFileOperation.CoCreateInstance(CLSID_FileOperation));
		for (unsigned int file_index = 0; file_index < file_group_descriptor->cItems; file_index++) {
			FILEDESCRIPTOR file_descriptor = file_group_descriptor->fgd[file_index];
			contents_format.lindex = file_index;
			std::unique_ptr<STGMEDIUM, decltype(medium_deleter)> pContentMedium(new STGMEDIUM(), medium_deleter);
			FAILED_RETURN(pDataObj->GetData(&contents_format, pContentMedium.get()));
			// Dump stream to a file
			HRESULT hr = CDropTarget::CopyStream(pFileOperation, m_spFolder, pContentMedium->pstm, file_descriptor.cFileName);
		}
		pFileOperation->PerformOperations();
	}
}


void CFilerGridView::Added(const std::wstring& fileName)
{
	LOG_THIS_1("Added " + wstr2str(fileName));
	CIDL idl;
	ULONG chEaten;
	ULONG dwAttributes;
	HRESULT hr = m_spFolder->GetShellFolderPtr()->ParseDisplayName(GetWndPtr()->m_hWnd, NULL, const_cast<LPWSTR>(fileName.c_str()), &chEaten, idl.ptrptr(), &dwAttributes);
	if (SUCCEEDED(hr) && idl) {
		GetItemsSource().push_back(m_spFolder->CreateShExFileFolder(idl));
		auto spRow = m_allRows.back();

		PostUpdate(Updates::ColumnVisible);
		PostUpdate(Updates::RowVisible);
		PostUpdate(Updates::Row);
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);

		if (m_isNewFile) {
			m_spCursorer->OnCursor(Cell(spRow, m_pNameColumn));
			PostUpdate(Updates::EnsureVisibleFocusedCell);
		}
		PostUpdate(Updates::Sort);
		FilterAll();
		if (m_isNewFile) {
			std::static_pointer_cast<CFileNameCell<std::shared_ptr<CShellFile>>>(Cell(spRow, m_pNameColumn))->OnEdit(Event(GetWndPtr()));
		}
		m_isNewFile = false;
	}
	else {
		LOG_THIS_1("Added FAILED " + wstr2str(fileName));
	}
}

void CFilerGridView::Modified(const std::wstring& fileName)
{
	LOG_THIS_1("Modified " + wstr2str(fileName));
	auto iter = std::find_if(ItemsSource.get_unconst()->begin(), ItemsSource.get_unconst()->end(),
							[fileName](const auto& value)->bool {
								return std::get<std::shared_ptr<CShellFile>>(value)->GetPathName() == fileName;
							});
//	auto iter = FindIfRowIterByFileNameExt(fileName);

	if (iter == ItemsSource.get_unconst()->end()) {
		LOG_THIS_1("Modified NoMatch " + wstr2str(fileName));
		return;
	} else {
		//Because ItemIdList includes, size, last write time, etc., it is necessary to get new one.
		ULONG chEaten;
		ULONG dwAttributes;
		CIDL newIdl;
		HRESULT hRes = std::get<std::shared_ptr<CShellFile>>(*iter)->GetParentShellFolderPtr()
			->ParseDisplayName(GetWndPtr()->m_hWnd, NULL, (LPWSTR)fileName.c_str(), &chEaten, newIdl.ptrptr(), &dwAttributes);

		if (SUCCEEDED(hRes) && newIdl) {
			//std::get<std::shared_ptr<CShellFile>>(*iter) = m_spFolder->CreateShExFileFolder(newIdl);
			ItemsSource.replace(iter, std::make_tuple(m_spFolder->CreateShExFileFolder(newIdl)));
			m_allRows[iter-ItemsSource->cbegin() + m_frozenRowCount]->SetIsMeasureValid(false);
			PostUpdate(Updates::ColumnVisible);
			PostUpdate(Updates::RowVisible);
			PostUpdate(Updates::Row);
			PostUpdate(Updates::Scrolls);
			PostUpdate(Updates::Invalidate);
			PostUpdate(Updates::Sort);
			FilterAll();
		} else {
			LOG_THIS_1("Modified FAILED " + wstr2str(fileName));
		}
	}
}
void CFilerGridView::Removed(const std::wstring& fileName)
{
	LOG_THIS_1("Removed " + wstr2str(fileName));

	auto iter = std::find_if(ItemsSource->cbegin(), ItemsSource->cend(),
							 [fileName](const auto& value)->bool {
								 return std::get<std::shared_ptr<CShellFile>>(value)->GetPathName() == fileName;
							 });

	if (iter == ItemsSource->cend()) {
		LOG_THIS_1("Removed NoMatch " + wstr2str(fileName));
		return;
	} else {

		ItemsSource.erase(iter);

		m_spCursorer->OnCursorClear(this);
		m_spCeller->OnClear();

		PostUpdate(Updates::ColumnVisible);
		PostUpdate(Updates::RowVisible);
		PostUpdate(Updates::Row);
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);
		PostUpdate(Updates::Sort);

		FilterAll();
	}

}
void CFilerGridView::Renamed(const std::wstring& oldName, const std::wstring& newName)
{
	LOG_THIS_1("Renamed " + wstr2str(oldName) + "=>"+ wstr2str(newName));
	auto iter = std::find_if(ItemsSource.get_unconst()->begin(), ItemsSource.get_unconst()->end(),
							 [oldName](const auto& value)->bool {
								 return std::get<std::shared_ptr<CShellFile>>(value)->GetPathName() == oldName;
							 });

	if (iter == ItemsSource.get_unconst()->end()) 
	{
		LOG_THIS_1("Renamed NoMatch " + wstr2str(oldName) + "=>" + wstr2str(newName));
	} else {
		ULONG chEaten;
		ULONG dwAttributes;
		CIDL newIdl;

		HRESULT hRes = std::get<std::shared_ptr<CShellFile>>(*iter)->GetParentShellFolderPtr()
			->ParseDisplayName(GetWndPtr()->m_hWnd, NULL, (LPWSTR)newName.c_str(), &chEaten, newIdl.ptrptr(), &dwAttributes);

		if (SUCCEEDED(hRes) && newIdl) {
			std::get<std::shared_ptr<CShellFile>>(*iter) = m_spFolder->CreateShExFileFolder(newIdl);
			m_allRows[iter - ItemsSource->cbegin() + m_frozenRowCount]->SetIsMeasureValid(false);
			PostUpdate(Updates::ColumnVisible);
			PostUpdate(Updates::RowVisible);
			PostUpdate(Updates::Row);
			PostUpdate(Updates::Scrolls);
			PostUpdate(Updates::Invalidate);
			PostUpdate(Updates::Sort);
			FilterAll();
		}
		else {
			LOG_THIS_1("Renamed FAILED " + wstr2str(oldName) + "=>" + wstr2str(newName));
			return Removed(oldName);
		}
	}
}

void CFilerGridView::Reload()
{
	OpenFolder(m_spFolder);
	SubmitUpdate();
}

void CFilerGridView::Normal_KeyDown(const KeyDownEvent& e)
{
	m_keepEnsureVisibleFocusedCell = false;
	switch (e.Char)
	{
	case 'R':
		if (::GetAsyncKeyState(VK_CONTROL)) {
			OpenFolder(m_spFolder);
			(*e.HandledPtr) = true;
		}
		break;
	case VK_F8:
		{
			int okcancel = ::MessageBox(GetWndPtr()->m_hWnd, L"Delete?", L"Delete?", MB_OKCANCEL);
			if (okcancel == IDOK) {
				DeleteSelectedFiles();
				(*e.HandledPtr) = true;
			}
		}
		break;
	case 'A':
		if(::GetAsyncKeyState(VK_CONTROL)){
			SelectAll();
			(*e.HandledPtr) = true;
		}
		break;
	case 'F':
		if(::GetAsyncKeyState(VK_CONTROL)){
			//TODOLOW
			GetWndPtr()->SendMessage(WM_COMMAND,IDM_FIND,NULL);
			(*e.HandledPtr) = true;
		}
		break;
	case VK_BACK:
		{
			OpenFolder(m_spFolder->GetParent());
			(*e.HandledPtr) = true;
		}
		break;
	case VK_F2:
		{
			if (m_spCursorer->GetFocusedCell()) {
				std::static_pointer_cast<CFileNameCell<std::shared_ptr<CShellFile>>>(Cell(m_spCursorer->GetFocusedCell()->GetRowPtr(), m_pNameColumn.get()))->OnEdit(Event(GetWndPtr()));
				(*e.HandledPtr) = true;
			}
		}
		break;
	case VK_F4:
		{
			auto pCell = m_spCursorer->GetFocusedCell();
			if (!pCell) { break; }
			if(auto spRow = dynamic_cast<CBindRow<std::shared_ptr<CShellFile>>*>(pCell->GetRowPtr())){
				auto spFile = spRow->GetItem<std::shared_ptr<CShellFile>>();
				auto path = spFile->GetPath();
				SHELLEXECUTEINFO	sei = { 0 };
				sei.cbSize = sizeof(SHELLEXECUTEINFO);
				sei.hwnd = GetWndPtr()->m_hWnd;
				sei.lpVerb = L"open";
				sei.lpFile = L"notepad.exe";
				sei.lpParameters = path.c_str();//When open with notepad, need to pass path. ItemIDList couldn't work for this case.
				sei.lpDirectory = NULL;
				sei.nShow = SW_SHOWNORMAL;
				sei.hInstApp = NULL;

				::ShellExecuteEx(&sei);
				(*e.HandledPtr) = true;
			}
		}
		break;
	case VK_F7:
		NewFolder();
		(*e.HandledPtr) = true;
		break;
	default:
		break;
	}
	if (!(*e.HandledPtr)) {
		CFilerBindGridView::Normal_KeyDown(e);
	}

};

void CFilerGridView::OpenFolder(const std::shared_ptr<CShellFolder>& spFolder)
{
	if (!spFolder->GetIsExist()) {
		m_spFolder = m_spFolder->GetParent();
		return OpenFolder(m_spFolder);
	}

	//OpenFolder is called when DeviceChange, even inactive.
	if (!spFolder) {
		return;
	}

	if (m_pEdit) {
		EndEdit();
	}

	LOG_THIS_1("CFilerGridView::OpenFolder : " + wstr2str(spFolder->GetDispName()));

	LOG_SCOPED_TIMER_THIS_1("OpenFolder Total");
	bool isUpdate = m_spFolder ? m_spFolder->GetPath() == spFolder->GetPath() : false;
	m_spPreviousFolder = m_spFolder;
	m_spFolder = spFolder;
	{
		LOG_SCOPED_TIMER_THIS_1("OpenFolder Pre-Process");
		
		//StateMachine // Do not reset
		//m_pMachine.reset(new CGridStateMachine(this));
		//Direct2DWrite
		if (!isUpdate) {
			GetWndPtr()->GetDirectPtr()->ClearTextLayoutMap();
			//GetWndPtr()->GetDirectPtr()->GetFileThumbnailDrawerPtr()->Clear();
		}
		//Celler
		m_spCeller->Clear();

		//Cursor
		m_spCursorer->Clear();
		if (!isUpdate) {
			DeselectAll();
		}


		//Save and Restore Filter value
		if (!isUpdate) {
			//Update Map
			std::unordered_map<std::shared_ptr<CColumn>, std::wstring> map;
			bool isAllEmpty = std::all_of(m_allCols.begin(), m_allCols.end(), [](const auto& ptr) { return ptr->GetFilter().empty(); });
			if (isAllEmpty) {
				if (m_spPreviousFolder) {
					m_filterMap.erase(m_spPreviousFolder->GetPath());
				}
			} else {
				for (const auto& colPtr : m_allCols) {
					map.emplace(colPtr, colPtr->GetFilter());
				}
				if (m_spPreviousFolder) {
					m_filterMap.insert_or_assign(m_spPreviousFolder->GetPath(), map);
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
				for (const auto& colPtr : m_allCols) {
					colPtr->SetFilter(L"");
				}
			}

		}

		//Clear RowDictionary From 0 to last
		GetItemsSource().clear();
		//m_allRows.idx_erase(m_allRows.begin() + m_frozenRowCount, m_allRows.end());
	}

	{
		LOG_SCOPED_TIMER_THIS_1("OpenFolder Enumeration");
		try {
		//Enumerate child IDL
			shell::for_each_idl_in_shellfolder(GetWndPtr()->m_hWnd, m_spFolder->GetShellFolderPtr(),
				[this](const CIDL& idl) {
					if (auto spFile = m_spFolder->CreateShExFileFolder(idl)) {
						GetItemsSource().push_back(std::make_tuple(spFile));
					}
				});
		} catch (std::exception&) {
			throw std::exception(FILE_LINE_FUNC);
		}

		//Set up Watcher
		try {
			if (::PathFileExists(m_spFolder->GetPath().c_str()) &&
				!boost::iequals(m_spFolder->GetDispExt(), L".zip")) {
				m_spWatcher->QuitWatching();
				m_spWatcher->StartWatching(m_spFolder->GetPath(), m_spFolder->GetAbsoluteIdl());
			} else {
				m_spWatcher->QuitWatching();
			}
		}
		catch (std::exception & e) {
			GetWndPtr()->MessageBox(L"Watcher", L"Error", 0);
			throw e;
		}

	}

	{
		LOG_SCOPED_TIMER_THIS_1("OpenFolder Updating");
		//auto& cells = m_allCells.get<rowcol_tag>();
		//auto iter = std::remove_if(m_allCells.begin(), m_allCells.end(), [this](const auto& cell)->bool { return (size_t)(cell->GetRowPtr()->GetIndex<AllTag>()) >= m_frozenRowCount; });
		//m_allCells.erase(iter, m_allCells.end());
		m_allCells.clear();
		//for (const auto& colPtr : m_allCols) {
		//	std::dynamic_pointer_cast<CMapColumn>(colPtr)->Clear();
		//}

		GetHeaderColumnPtr()->SetIsFitMeasureValid(false);


		//PathCell
		auto pPathCell = Cell(m_pHeaderRow, m_pNameColumn);
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

		if (!isUpdate) {
			m_pVScroll->SetScrollPos(0);

			//If previous folder is found, set cursor for that.
			if (m_spPreviousFolder) {
				auto iter = std::find_if(ItemsSource->cbegin(), ItemsSource->cend(), [this](const auto& tup)->bool {
					return std::get<std::shared_ptr<CShellFile>>(tup)->GetAbsoluteIdl() == m_spPreviousFolder->GetAbsoluteIdl();
				});

				if (iter != ItemsSource->cend()) {

					if (auto cell = Cell(m_allRows[iter-ItemsSource->cbegin() + m_frozenRowCount], m_visCols[m_frozenColumnCount])) {
						m_spCursorer->OnCursor(cell);
						m_keepEnsureVisibleFocusedCell = true;
						PostUpdate(Updates::EnsureVisibleFocusedCell);
					}
				}
			}

		}
	}
}

void CFilerGridView::OnBkGndLButtondDblClk(const LButtonDblClkEvent& e)
{
	OpenFolder(m_spFolder->GetParent());
	SubmitUpdate();
}

void CFilerGridView::OnCellLButtonDblClk(const CellEventArgs& e)
{
	auto pCell = e.CellPtr;
	if(auto spRow = dynamic_cast<CBindRow<std::shared_ptr<CShellFile>>*>(e.CellPtr->GetRowPtr())){
		auto spFile = spRow->GetItem<std::shared_ptr<CShellFile>>();
		Open(spFile);
	}
}

std::shared_ptr<CShellFile> CFilerGridView::GetFocusedFile()
{
	CRow* pRow = GetCursorerPtr()->GetFocusedCell()->GetRowPtr();
	if (auto pBindRow = dynamic_cast<CBindRow<std::shared_ptr<CShellFile>>*>(pRow)) {
		return pBindRow->GetItem<std::shared_ptr<CShellFile>>();
	}
	return nullptr;
}


std::vector<std::shared_ptr<CShellFile>> CFilerGridView::GetSelectedFiles()
{
	std::vector<std::shared_ptr<CShellFile>> files;
	for (const auto& rowPtr : m_visRows) {
		if (rowPtr->GetIsSelected()) {
			if (auto spRow = std::dynamic_pointer_cast<CBindRow<std::shared_ptr<CShellFile>>>(rowPtr)) {
				files.push_back(spRow->GetItem<std::shared_ptr<CShellFile>>());
			}
		}
	}
	return files;

}

std::vector<LPITEMIDLIST> CFilerGridView::GetSelectedLastPIDLVector()
{
	std::vector<LPITEMIDLIST> vPidl;
	for (const auto& rowPtr : m_visRows) {
		if (rowPtr->GetIsSelected()) {
			if (auto spRow = std::dynamic_pointer_cast<CBindRow<std::shared_ptr<CShellFile>>>(rowPtr)) {
				auto spFile = spRow->GetItem<std::shared_ptr<CShellFile>>();
				vPidl.push_back(spFile->GetAbsoluteIdl().FindLastID());
			}
		}
	}
	return vPidl;
}

std::vector<LPITEMIDLIST> CFilerGridView::GetSelectedAbsolutePIDLVector()
{
	std::vector<LPITEMIDLIST> vPidl;
	for (const auto& rowPtr : m_visRows) {
		if (rowPtr->GetIsSelected()) {
			if (auto spRow = std::dynamic_pointer_cast<CBindRow<std::shared_ptr<CShellFile>>>(rowPtr)) {
				auto spFile = spRow->GetItem<std::shared_ptr<CShellFile>>();
				vPidl.push_back(spFile->GetAbsoluteIdl().m_pIDL);
			}
		}
	}
	return vPidl;
}

std::vector<CIDL> CFilerGridView::GetSelectedChildIDLVector()
{
	std::vector<CIDL> childIDLs;
	for (const auto& rowPtr : m_visRows) {
		if (rowPtr->GetIsSelected()) {
			if (auto spRow = std::dynamic_pointer_cast<CBindRow<std::shared_ptr<CShellFile>>>(rowPtr)) {
				auto spFile = spRow->GetItem<std::shared_ptr<CShellFile>>();
				childIDLs.push_back(spFile->GetChildIdl());
			}
		}
	}
	return childIDLs;
}


bool CFilerGridView::CopyIncrementalSelectedFilesTo(const CIDL& destIDL)
{
	auto pDlg = std::make_shared<CIncrementalCopyDlg>(
		GetWndPtr(), 
		static_cast<CFilerWnd*>(GetWndPtr())->GetDialogPropPtr(),
		std::static_pointer_cast<FilerGridViewProperty>(m_spGridViewProp),
		destIDL, m_spFolder->GetAbsoluteIdl(), GetSelectedChildIDLVector());

	pDlg->OnCreate(CreateEvt(GetWndPtr(), GetWndPtr(), GetWndPtr()->CalcCenterRectF(CSizeF(400, 600))));
	GetWndPtr()->SetFocusedControlPtr(pDlg);

	return true;
}

bool CFilerGridView::CopySelectedFilesTo(const CIDL& destIDL)
{
	//Create
	auto pDlg = std::make_shared<CCopyDlg>(
		GetWndPtr(), 
		static_cast<CFilerWnd*>(GetWndPtr())->GetDialogPropPtr(),
		std::static_pointer_cast<FilerGridViewProperty>(m_spGridViewProp),
		destIDL, m_spFolder->GetAbsoluteIdl(), GetSelectedChildIDLVector());

	pDlg->OnCreate(CreateEvt(GetWndPtr(), GetWndPtr(), GetWndPtr()->CalcCenterRectF(CSizeF(300, 400))));
	GetWndPtr()->SetFocusedControlPtr(pDlg);

	return true;
}

bool CFilerGridView::MoveSelectedFilesTo(const CIDL& destIDL)
{
	//Create
	auto pDlg = std::make_shared<CMoveDlg>(
		GetWndPtr(),
		static_cast<CFilerWnd*>(GetWndPtr())->GetDialogPropPtr(),
		std::static_pointer_cast<FilerGridViewProperty>(m_spGridViewProp),
		destIDL, m_spFolder->GetAbsoluteIdl(), GetSelectedChildIDLVector());

	pDlg->OnCreate(CreateEvt(GetWndPtr(), GetWndPtr(), GetWndPtr()->CalcCenterRectF(CSizeF(300, 400))));
	GetWndPtr()->SetFocusedControlPtr(pDlg);

	return true;
}

bool CFilerGridView::DeleteSelectedFiles()
{
	//Create
	auto pDlg = std::make_shared<CDeleteDlg>(
		GetWndPtr(),
		static_cast<CFilerWnd*>(GetWndPtr())->GetDialogPropPtr(),
		std::static_pointer_cast<FilerGridViewProperty>(m_spGridViewProp),
		m_spFolder->GetAbsoluteIdl(), GetSelectedChildIDLVector());

	pDlg->OnCreate(CreateEvt(GetWndPtr(), GetWndPtr(), GetWndPtr()->CalcCenterRectF(CSizeF(300, 400))));
	GetWndPtr()->SetFocusedControlPtr(pDlg);

	return true;
}

bool CFilerGridView::NewFolder()
{
	return InvokeNewShellContextmenuCommand(GetWndPtr()->m_hWnd, CMDSTR_NEWFOLDERA, m_spFolder->GetShellFolderPtr());
}

bool CFilerGridView::CutToClipboard()
{
	return InvokeNormalShellContextmenuCommand(GetWndPtr()->m_hWnd, "Cut", m_spFolder->GetShellFolderPtr(), GetSelectedFiles());
}

bool CFilerGridView::CopyToClipboard()
{
	return InvokeNormalShellContextmenuCommand(GetWndPtr()->m_hWnd, "Copy", m_spFolder->GetShellFolderPtr(), GetSelectedFiles());
}

bool CFilerGridView::PasteFromClipboard()
{
	//std::vector<LPITEMIDLIST> vPidl;
	CIDL idl = m_spFolder->GetAbsoluteIdl().CloneParentIDL();
	CComPtr<IShellFolder> pDesktop;
	::SHGetDesktopFolder(&pDesktop);
	CComPtr<IShellFolder> pFolder;
	::SHBindToObject(pDesktop,idl.ptr(),0,IID_IShellFolder,(void**)&pFolder);
	if(!pFolder){
		pFolder = pDesktop;
	}
	//vPidl.push_back(m_spFolder->GetAbsoluteIdl().FindLastID());
	return InvokeNormalShellContextmenuCommand(GetWndPtr()->m_hWnd, "Paste", pFolder, {m_spFolder});
}


void CFilerGridView::OnDirectoryWatch(const DirectoryWatchEvent& e)
{
	if (e.Infos.size() > 1) {
		auto a = 1.f;
	}
	for(auto info : e.Infos){
		switch (info.first) {
		case FILE_ACTION_ADDED:
			LOG_THIS_1("FILE_ACTION_ADDED");
			Added(info.second);
			break;
		case FILE_ACTION_MODIFIED:
			LOG_THIS_1("FILE_ACTION_MODIFIED");
			Modified(info.second);
			break;
		case FILE_ACTION_REMOVED:
			LOG_THIS_1("FILE_ACTION_REMOVED");
			Removed(info.second);
			break;
		case FILE_ACTION_RENAMED_NEW_NAME:
			LOG_THIS_1("FILE_ACTION_RENAMED_NEW_NAME");
			{
				std::list<std::wstring> oldnew;
				boost::split(oldnew, info.second, boost::is_any_of(L"/"));
				Renamed(oldnew.front(), oldnew.back());
			}
			break;
		case FILE_ACTION_RENAMED_OLD_NAME:
			LOG_THIS_1("FILE_ACTION_RENAMED_OLD_NAME");
			break;
		default:
			break;
		}
	}
	
	SubmitUpdate();
}

void CFilerGridView::OnMouseWheel(const MouseWheelEvent& e)
{
	bool ctrl = (::GetKeyState(VK_CONTROL) & 0x80) != 0;
	if (ctrl) {
		bool changed = false;
		std::for_each(m_allCols.cbegin(), m_allCols.cend(), [&](const std::shared_ptr<CColumn>& spCol)->void {
			if (auto p = std::dynamic_pointer_cast<IImageColumn>(spCol)) {
				UINT32 size = static_cast<UINT32>(std::clamp(static_cast<int>(p->GetImageSize()) + 16l * e.Delta / WHEEL_DELTA, 16l, 256l));
				if (p->GetImageSize() != size) {
					p->SetImageSize(size);
					changed = true;
				}
			}
		});
		if (changed) {
			Reload();
		}
	} else {
		CGridView::OnMouseWheel(e);
	}
}

bool CFilerGridView::InvokeNormalShellContextmenuCommand(HWND hWnd, LPCSTR lpVerb, CComPtr<IShellFolder> psf, std::vector<std::shared_ptr<CShellFile>> files)
{
	std::vector<PITEMID_CHILD> vpIdl;
	for (auto file : files) {
		vpIdl.push_back(file->GetAbsoluteIdl().FindLastID());
	}
    CComPtr<IContextMenu> pcm;
	HRESULT hr=psf->GetUIObjectOf(hWnd, vpIdl.size(),(LPCITEMIDLIST*)(vpIdl.data()),IID_IContextMenu,nullptr,(LPVOID *)&pcm);
    if(SUCCEEDED(hr)){
		CMenu menu=CMenu(CreatePopupMenu());
		if(!menu.IsNull()){
			hr=pcm->QueryContextMenu(menu,0,1,0x7FFF, CMF_NORMAL);
			if (SUCCEEDED(hr)){
				CMINVOKECOMMANDINFO cmi = {0};
				cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
				cmi.fMask = CMIC_MASK_NOASYNC;
				cmi.hwnd = hWnd;
				cmi.lpVerb = lpVerb;
				cmi.nShow = SW_SHOWNORMAL;

				hr=pcm->InvokeCommand((LPCMINVOKECOMMANDINFO)&cmi);
			}
		}
    }

	if (SUCCEEDED(hr)) {
		if (StatusLog) {
			StatusLog(fmt::format(L"SUCCEEDED {}:{}", str2wstr(lpVerb), files[0]->GetPathName()));
		}
	} else {
		if (StatusLog) {
			StatusLog(fmt::format(L"FAILED {}:{}", str2wstr(lpVerb), files[0]->GetPathName()));
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
	m_isNewFile = true;
	hr = m_pcmNew3->InvokeCommand((LPCMINVOKECOMMANDINFO)&cmi);

	m_pcmNew2 = nullptr;
	m_pcmNew3 = nullptr;

	return SUCCEEDED(hr);
}

void CFilerGridView::Normal_ContextMenu(const ContextMenuEvent& e)
{
	auto cell = Cell(GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
	std::vector<std::shared_ptr<CShellFile>> files;

	if (!cell) {
		//Folder menu
		//CIDL idl = m_spFolder->GetAbsoluteIdl().CloneParentIDL();
		//CComPtr<IShellFolder> pDesktop;
		//::SHGetDesktopFolder(&pDesktop);
		//CComPtr<IShellFolder> pFolder;
		//::SHBindToObject(pDesktop, idl.ptr(), 0, IID_IShellFolder, (void**)&pFolder);
		//if (!pFolder) {
		//	pFolder = pDesktop;
		//}
		files.push_back(m_spFolder);
		ShowShellContextMenu(GetWndPtr()->m_hWnd, e.PointInScreen, m_spFolder->GetParent(), files, true);
	}else if(cell->GetRowPtr() == m_pHeaderRow.get() || cell->GetRowPtr() == m_pNameHeaderRow.get()){
		//Header menu
		CMenu menu(::CreatePopupMenu());
		if (menu.IsNull()) { return; }

		MENUITEMINFO mii = { 0 };
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_FTYPE | MIIM_STRING | MIIM_STATE | MIIM_ID;
		mii.fType = MFT_STRING;
		for (const std::shared_ptr<CColumn>& colPtr : m_allCols) {
			mii.fState = colPtr->GetIsVisible() ? MFS_CHECKED : MFS_UNCHECKED;
			mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, Cell(m_pNameHeaderRow, colPtr)->GetString());
			std::wstring dwTypeData = Cell(m_pNameHeaderRow, colPtr)->GetString();
			//std::array<wchar_t, 256> dwTypeData; 
			//std::copy(Cell(m_pNameHeaderRow, colPtr)->GetString().cbegin(), Cell(m_pNameHeaderRow, colPtr)->GetString().cend(), dwTypeData.begin());
			//dwTypeData[Cell(m_pNameHeaderRow, colPtr)->GetString().size()] = L'\0';
			mii.dwTypeData = const_cast<LPWSTR>(dwTypeData.c_str());
			menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);
		}

		int idCmd = menu.TrackPopupMenu(
			TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON,
			e.PointInScreen.x,
			e.PointInScreen.y,
			GetWndPtr()->m_hWnd);

		if (idCmd) {
			for (const std::shared_ptr<CColumn>& colPtr : m_allCols) {
				if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, Cell(m_pNameHeaderRow, colPtr)->GetString())){
					colPtr->SetIsVisible(!colPtr->GetIsVisible());
					Reload();
					/*std::ranges::for_each(m_allCells, [](const std::shared_ptr<CCell>& pCell) { pCell->SetActMeasureValid(false); });
					SetAllRowMeasureValid(false);
					PostUpdate(Updates::Column);
					PostUpdate(Updates::ColumnVisible);
					PostUpdate(Updates::Row);
					PostUpdate(Updates::RowVisible);
					PostUpdate(Updates::Scrolls);
					PostUpdate(Updates::Invalidate);
					SubmitUpdate();*/
				}
			}
		}
		



		//if (m_headerMenuItems.empty()) {
		//	for (const auto& colPtr : m_allCols ) {
		//		auto cell = Cell(m_pNameHeaderRow, colPtr);
		//		auto str = cell->GetString();
		//		m_headerMenuItems.push_back(std::make_shared<CShowHideColumnMenuItem>(
		//			IDM_VISIBLEROWHEADERCOLUMN + colPtr->GetIndex<AllTag>(),
		//			Cell(m_pNameHeaderRow, colPtr)->GetString(), this, colPtr.get()));
		//	}

		//	for (auto& item : m_headerMenuItems) {
		//		GetWndPtr()->AddCmdIDHandler(item->GetID(), std::bind(&CMenuItem::OnCommand, item.get(), phs::_1, phs::_2, phs::_3, phs::_4));
		//	}
		//}

		//for (auto& item : m_headerMenuItems) {
		//	menu.InsertMenuItemW(menu.GetMenuItemCount(), TRUE, item.get());
		//}

		//menu.TrackPopupMenu(
		//	TPM_LEFTALIGN | TPM_RIGHTBUTTON,
		//	e.PointInScreen.x,
		//	e.PointInScreen.y,
		//	GetWndPtr()->m_hWnd);
	}else{
		//Cell menu
		for(auto rowPtr : m_visRows){
			if(rowPtr->GetIsSelected()){
				auto spRow=std::dynamic_pointer_cast<CBindRow<std::shared_ptr<CShellFile>>>(rowPtr);
				auto spFile = spRow->GetItem<std::shared_ptr<CShellFile>>();
				files.push_back(spFile);
			}
		}
			ShowShellContextMenu(GetWndPtr()->m_hWnd, e.PointInScreen, m_spFolder, files);
	}

	//CFilerGridViewBase::Normal_ContextMenu(e);
}

void CFilerGridView::ShowShellContextMenu(HWND hWnd, CPoint ptScreen, const std::shared_ptr<CShellFolder>& folder, const std::vector<std::shared_ptr<CShellFile>>& files, bool hasNew)
{
	std::vector<PITEMID_CHILD> vPidl;
	for (auto file : files) {
		vPidl.push_back(file->GetAbsoluteIdl().FindLastID());
	}

	try {
		GetWndPtr()->AddAllMsgHandler(&CFilerGridView::OnHandleMenuMsg, this);

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
		hr = folder->GetShellFolderPtr()->GetUIObjectOf(hWnd, vPidl.size(), (LPCITEMIDLIST*)(vPidl.data()), IID_IContextMenu, nullptr, (LPVOID *)&pcm);
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
		mii.dwTypeData = const_cast<LPWSTR>(L"Copy Text");
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
			GetWndPtr()->m_hWnd);

		if (idCmd) {

			CMINVOKECOMMANDINFOEX info = { 0 };
			info.cbSize = sizeof(info);
			info.fMask = /*CMIC_MASK_UNICODE |*/ CMIC_MASK_PTINVOKE;
			info.hwnd = hWnd;
			info.nShow = SW_SHOWNORMAL;
			info.ptInvoke = ptScreen;

			if (ExecCustomContextMenu(idCmd, folder, files)) {

			} else if (idCmd == IDM_COPYTEXT) {
				BOOL bHandled = FALSE;
				CGridView::OnCommandCopy(CommandEvent(GetWndPtr(), (WPARAM)idCmd, (LPARAM)GetWndPtr()->m_hWnd, &bHandled));
			}else if (idCmd >= SCRATCH_QCM_NEW) {
				info.lpVerb = MAKEINTRESOURCEA(idCmd - SCRATCH_QCM_NEW);
				info.lpVerbW = MAKEINTRESOURCEW(idCmd - SCRATCH_QCM_NEW);
				m_isNewFile = true;
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
		GetWndPtr()->RemoveAllMsgHandler();
		throw;
	}

	m_hNewMenu = NULL;
	m_pcmNew2 = nullptr;
	m_pcmNew3 = nullptr;
	m_pcm2 = nullptr;
	m_pcm3 = nullptr;
	GetWndPtr()->RemoveAllMsgHandler();
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

std::wstring CFilerGridView::GetPath()const
{
	return m_spFolder->GetPath();
}

void CFilerGridView::SetPath(const std::wstring& path)
{
	if (auto pFolder = std::dynamic_pointer_cast<CShellFolder>(CShellFileFactory::GetInstance()->CreateShellFilePtr(path))) {
		OpenFolder(pFolder);
	}
	else {
		//Do nothing
	}
}

void CFilerGridView::Drag()
{
	std::vector<LPITEMIDLIST> vPidl;
	CIDL pFirstIdl;
	for(const auto& rowPtr : m_visRows){
		if(rowPtr->GetIsSelected()){
			if(auto spRow=std::dynamic_pointer_cast<CBindRow<std::shared_ptr<CShellFile>>>(rowPtr)){
				auto spFile = spRow->GetItem<std::shared_ptr<CShellFile>>();
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

	SHGetFileInfo((LPCTSTR)firstIdl.ptr(), 0, &fileInfo, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_ICON | SHGFI_LARGEICON | SHGFI_ADDOVERLAYS);


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

	HDC hdc = ::GetDC(GetWndPtr()->m_hWnd);
	HDC hdcMem = ::CreateCompatibleDC(hdc);
	HBITMAP hbmp = ::CreateCompatibleBitmap(hdc, size.cx, size.cy);
	HBITMAP hbmpPrev = (HBITMAP)::SelectObject(hdcMem, hbmp);
	::DrawIcon(hdcMem, 0, 0, fileInfo.hIcon);
	::SelectObject(hdcMem, hbmpPrev);
	::DeleteDC(hdcMem);
	::ReleaseDC(GetWndPtr()->m_hWnd, hdc);
	::DestroyIcon(fileInfo.hIcon);

	dragImage.sizeDragImage = size;
	dragImage.ptOffset = pt;
	dragImage.hbmpDragImage = hbmp;
	dragImage.crColorKey = RGB(0, 0, 0);
	HRESULT hr = pDragSourceHelper->InitializeFromBitmap(&dragImage, pDataObject);

	return hr == S_OK;
}

void CFilerGridView::OnCommandFind(const CommandEvent& e)
{
	//TODOLOW
	//auto pWnd = new CSearchWnd(std::static_pointer_cast<FilerGridViewProperty>(m_spGridViewProp), m_spFolder->GetAbsoluteIdl());
	//pWnd->SetIsDeleteOnFinalMessage(true);

	//HWND hWnd = NULL;
	//if ((GetWndPtr()->GetWindowLongPtr(GWL_STYLE) & WS_OVERLAPPEDWINDOW) == WS_OVERLAPPEDWINDOW) {
	//	hWnd = GetWndPtr()->m_hWnd;
	//} else {
	//	hWnd = GetWndPtr()->GetAncestorByStyle(WS_OVERLAPPEDWINDOW);
	//}

	//pWnd->CreateOnCenterOfParent(hWnd, CSize(400, 600));
	//pWnd->ShowWindow(SW_SHOW);
	//pWnd->UpdateWindow();
}
