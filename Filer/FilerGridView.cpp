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

//#include "PathRow.h"
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
#include "ShellContextMenu.h"
#include "ExeExtensionProperty.h"

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


CFilerGridView::CFilerGridView(CD2DWControl* pParentControl)
	:CFilerBindGridView(pParentControl), Dummy(std::make_shared<int>(0))
{
	m_commandMap.emplace(IDM_SELECTALL, std::bind(&CFilerGridView::OnCommandSelectAll, this, phs::_1));
	m_commandMap.emplace(IDM_CUT, std::bind(&CFilerGridView::OnCommandCut, this, phs::_1));
	m_commandMap.emplace(IDM_COPY, std::bind(&CFilerGridView::OnCommandCopy, this, phs::_1));
	m_commandMap.emplace(IDM_PASTE, std::bind(&CFilerGridView::OnCommandPaste, this, phs::_1));
	m_commandMap.emplace(IDM_DELETE, std::bind(&CFilerGridView::OnCommandDelete, this, phs::_1));
	m_commandMap.emplace(IDM_FIND, std::bind(&CFilerGridView::OnCommandFind, this, phs::_1));

	m_spItemDragger = std::make_shared<CFileDragger>();

	Folder.subscribe([this](auto value) {
		bool isReload = m_spPreviousFolder ? m_spPreviousFolder->GetPath() == value->GetPath() : false;
		OpenFolder(value, isReload);
		SubmitUpdate();
		m_spPreviousFolder = value;
	}, Dummy);
}

CFilerGridView::~CFilerGridView()
{
	if (m_spWatcher) { m_spWatcher->QuitWatching(); }
}

CShellContextMenu& CFilerGridView::GetFileContextMenu()
{
	if (!m_optFileContextMenu.has_value()) {
		CShellContextMenu menu;

		auto GetSelectedFiles = [this]()->std::vector<std::shared_ptr<CShellFile>> {
			std::vector<std::shared_ptr<CShellFile>> files;
			for(auto rowPtr : m_visRows){
				if(rowPtr->GetIsSelected()){
					auto spRow=std::dynamic_pointer_cast<CBindRow<std::shared_ptr<CShellFile>>>(rowPtr);
					auto spFile = spRow->GetItem<std::shared_ptr<CShellFile>>();
					files.push_back(spFile);
				}
			}
			return files;
		};
		menu.Add(
			std::make_unique<CMenuSeparator2>(),
			std::make_unique<CMenuItem2>(L"Copy Text", 
				[this]()->void {
				BOOL bHandled = FALSE;
				CGridView::OnCommandCopy(CommandEvent(GetWndPtr(), (WPARAM)0, (LPARAM)GetWndPtr()->m_hWnd, &bHandled));}),
			std::make_unique<CMenuSeparator2>(),
			std::make_unique<CMenuItem2>(L"PDF Split",
				[this, GetSelectedFiles]()->void {
				auto spDlg = std::make_shared<CPDFSplitDlg>(this, Folder.get_shared_unconst(), GetSelectedFiles());
				spDlg->OnCreate(CreateEvt(GetWndPtr(), const_cast<CFilerGridView*>(this), CalcCenterRectF(CSizeF(300, 200))));
				GetWndPtr()->SetFocusToControl(spDlg); }),
			std::make_unique<CMenuItem2>(L"PDF Merge", 
				[this, GetSelectedFiles]()->void {
				auto spDlg = std::make_shared<CPDFMergeDlg>(this, Folder.get_shared_unconst(), GetSelectedFiles());
				spDlg->OnCreate(CreateEvt(GetWndPtr(), const_cast<CFilerGridView*>(this), CalcCenterRectF(CSizeF(300, 400))));
				GetWndPtr()->SetFocusToControl(spDlg); }),
			std::make_unique<CMenuItem2>(L"PDF Extract", 
				[this, GetSelectedFiles]()->void {
				auto spDlg = std::make_shared<CPDFExtractDlg>(this, Folder.get_shared_unconst(), GetSelectedFiles());
				spDlg->OnCreate(CreateEvt(GetWndPtr(), const_cast<CFilerGridView*>(this), CalcCenterRectF(CSizeF(300, 400))));
				GetWndPtr()->SetFocusToControl(spDlg); }),
			std::make_unique<CMenuItem2>(L"PDF Unlock",
				[this, GetSelectedFiles]()->void {
				auto spDlg = std::make_shared<CPDFUnlockDlg>(this, Folder.get_shared_unconst(), GetSelectedFiles());
				spDlg->OnCreate(CreateEvt(GetWndPtr(), const_cast<CFilerGridView*>(this), CalcCenterRectF(CSizeF(300, 400))));
				GetWndPtr()->SetFocusToControl(spDlg); })
		);

		menu.Add(std::make_unique<CMenuSeparator2>());

		for (auto iter = ExeExtensions->begin(); iter != ExeExtensions->end(); ++iter) {
			menu.Add(
			std::make_unique<CMenuItem2>(iter->Name.c_str(),
				[this, GetSelectedFiles, ee = *iter]()->void {
				auto spDlg = std::make_shared<CExeExtensionDlg>(this, Folder.get_shared_unconst(), GetSelectedFiles(), ee);
				spDlg->OnCreate(CreateEvt(GetWndPtr(), const_cast<CFilerGridView*>(this), CalcCenterRectF(CSizeF(300, 400))));
				GetWndPtr()->SetFocusToControl(spDlg); })
			);
		}
		m_optFileContextMenu.emplace(std::move(menu));
	}
	return m_optFileContextMenu.value();

}

CShellContextMenu& CFilerGridView::GetFolderContextMenu()
{
	if (!m_optFolderContextMenu.has_value()) {
		m_optFolderContextMenu.emplace(CShellContextMenu());
	}
	return m_optFolderContextMenu.value();
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
	//m_pHeaderRow = std::make_shared<CPathRow>(this, GetHeaderProperty());
	m_pNameHeaderRow = std::make_shared<CHeaderRow>(this);
	m_pFilterRow = std::make_shared<CRow>(this);

	//m_allRows.idx_push_back(m_pHeaderRow);
	m_allRows.idx_push_back(m_pNameHeaderRow);
	m_allRows.idx_push_back(m_pFilterRow);

	m_frozenRowCount = 2;

	//Insert columns if not initialized

	
	//if (m_allCols.empty()) {

		if (!std::any_of(m_allCols.cbegin(), m_allCols.cend(), [](const std::shared_ptr<CColumn>& pCol) { return typeid(*pCol) == typeid(CRowIndexColumn); })) 
		{ 
			m_allCols.idx_push_back(std::make_shared<CRowIndexColumn>(this));
		}
		if (!std::any_of(m_allCols.cbegin(), m_allCols.cend(), [](const std::shared_ptr<CColumn>& pCol) { return typeid(*pCol) == typeid(CFileNameColumn<std::shared_ptr<CShellFile>>); })) 
		{ 
			m_allCols.idx_push_back(std::make_shared<CFileNameColumn<std::shared_ptr<CShellFile>>>(this));
		}
		if (!std::any_of(m_allCols.cbegin(), m_allCols.cend(), [](const std::shared_ptr<CColumn>& pCol) { return typeid(*pCol) == typeid(CFileDispExtColumn<std::shared_ptr<CShellFile>>); })) {
			m_allCols.idx_push_back(std::make_shared<CFileDispExtColumn<std::shared_ptr<CShellFile>>>(this));
		}
		if (!std::any_of(m_allCols.cbegin(), m_allCols.cend(), [](const std::shared_ptr<CColumn>& pCol) { return typeid(*pCol) == typeid(CFileSizeColumn<std::shared_ptr<CShellFile>>); })) {
			m_allCols.idx_push_back(std::make_shared<CFileSizeColumn<std::shared_ptr<CShellFile>>>(this));
		}
		if (!std::any_of(m_allCols.cbegin(), m_allCols.cend(), [](const std::shared_ptr<CColumn>& pCol) { return typeid(*pCol) == typeid(CFileLastWriteColumn<std::shared_ptr<CShellFile>>); })) {
			m_allCols.idx_push_back(std::make_shared<CFileLastWriteColumn<std::shared_ptr<CShellFile>>>(this));
		}

		m_frozenColumnCount = 2;
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

		FAILED_RETURN(::SHCreateItemFromIDList(Folder->GetAbsoluteIdl().ptr(), IID_IShellItem2, reinterpret_cast<LPVOID*>(&pDestShellItem)));

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
			if (folderIdl == Folder->GetAbsoluteIdl()) {
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
			if (folderIdl == Folder->GetAbsoluteIdl()) {
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

				std::wstring destPath = Folder->GetPath();
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
							CDropTarget::CopyMessage(pFileOperation, Folder.get_shared_unconst(), pMsg.get());
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
			HRESULT hr = CDropTarget::CopyStream(pFileOperation, Folder.get_shared_unconst(), pContentMedium->pstm, file_descriptor.cFileName);
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
	HRESULT hr = Folder->GetShellFolderPtr()->ParseDisplayName(GetWndPtr()->m_hWnd, NULL, const_cast<LPWSTR>(fileName.c_str()), &chEaten, idl.ptrptr(), &dwAttributes);
	if (SUCCEEDED(hr) && idl) {
		GetItemsSource().push_back(Folder->CreateShExFileFolder(idl));
		auto spRow = m_allRows.back();

		PostUpdate(Updates::ColumnVisible);
		PostUpdate(Updates::RowVisible);
		PostUpdate(Updates::Row);
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);

		if (m_isNewFile) {
			m_spCursorer->OnCursor(Cell(spRow, m_allCols.at(m_frozenColumnCount)));
			PostUpdate(Updates::EnsureVisibleFocusedCell);
		}
		PostUpdate(Updates::Sort);
		UpdateFilter();
		if (m_isNewFile) {
			if (auto cols = FindColumns<CFileNameColumn<std::shared_ptr<CShellFile>>>(); !cols.empty()) {
				std::static_pointer_cast<CFileNameCell<std::shared_ptr<CShellFile>>>(Cell(spRow, cols.front()))->OnEdit(Event(GetWndPtr()));
			}
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
							[fileName](const std::shared_ptr<CShellFile>& value)->bool {
								return value->GetPathName() == fileName;
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
		HRESULT hRes = (*iter)->GetParentShellFolderPtr()
			->ParseDisplayName(GetWndPtr()->m_hWnd, NULL, (LPWSTR)fileName.c_str(), &chEaten, newIdl.ptrptr(), &dwAttributes);

		if (SUCCEEDED(hRes) && newIdl) {
			//std::get<std::shared_ptr<CShellFile>>(*iter) = m_spFolder->CreateShExFileFolder(newIdl);
			ItemsSource.replace(iter, Folder->CreateShExFileFolder(newIdl));
			m_allRows[iter-ItemsSource->cbegin() + m_frozenRowCount]->SetIsMeasureValid(false);
			PostUpdate(Updates::ColumnVisible);
			PostUpdate(Updates::RowVisible);
			PostUpdate(Updates::Row);
			PostUpdate(Updates::Scrolls);
			PostUpdate(Updates::Invalidate);
			PostUpdate(Updates::Sort);
			UpdateFilter();
		} else {
			LOG_THIS_1("Modified FAILED " + wstr2str(fileName));
		}
	}
}
void CFilerGridView::Removed(const std::wstring& fileName)
{
	LOG_THIS_1("Removed " + wstr2str(fileName));

	auto iter = std::find_if(ItemsSource->cbegin(), ItemsSource->cend(),
							 [fileName](const std::shared_ptr<CShellFile>& value)->bool {
								 return value->GetPathName() == fileName;
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

		UpdateFilter();
	}

}
void CFilerGridView::Renamed(const std::wstring& oldName, const std::wstring& newName)
{
	LOG_THIS_1("Renamed " + wstr2str(oldName) + "=>"+ wstr2str(newName));
	auto iter = std::find_if(ItemsSource.get_unconst()->begin(), ItemsSource.get_unconst()->end(),
							 [oldName](const std::shared_ptr<CShellFile>& value)->bool {
								 return value->GetPathName() == oldName;
							 });

	if (iter == ItemsSource.get_unconst()->end()) 
	{
		LOG_THIS_1("Renamed NoMatch " + wstr2str(oldName) + "=>" + wstr2str(newName));
	} else {
		ULONG chEaten;
		ULONG dwAttributes;
		CIDL newIdl;

		HRESULT hRes = (*iter)->GetParentShellFolderPtr()
			->ParseDisplayName(GetWndPtr()->m_hWnd, NULL, (LPWSTR)newName.c_str(), &chEaten, newIdl.ptrptr(), &dwAttributes);

		if (SUCCEEDED(hRes) && newIdl) {
			(*iter) = Folder->CreateShExFileFolder(newIdl);
			m_allRows[iter - ItemsSource->cbegin() + m_frozenRowCount]->SetIsMeasureValid(false);
			PostUpdate(Updates::ColumnVisible);
			PostUpdate(Updates::RowVisible);
			PostUpdate(Updates::Row);
			PostUpdate(Updates::Scrolls);
			PostUpdate(Updates::Invalidate);
			PostUpdate(Updates::Sort);
			UpdateFilter();
		}
		else {
			LOG_THIS_1("Renamed FAILED " + wstr2str(oldName) + "=>" + wstr2str(newName));
			return Removed(oldName);
		}
	}
}

void CFilerGridView::Reload()
{
	if (Folder) {
		OpenFolder(Folder.get_shared_unconst(), true);
		SubmitUpdate();
	}
}

void CFilerGridView::Normal_KeyDown(const KeyDownEvent& e)
{
	m_keepEnsureVisibleFocusedCell = false;
	switch (e.Char)
	{
	case 'R':
		if (::GetAsyncKeyState(VK_CONTROL)) {
			Reload();
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
			Folder.set(Folder->GetParentFolderPtr());
			(*e.HandledPtr) = true;
		}
		break;
	case VK_F2:
		{
			if (m_spCursorer->GetFocusedCell()) {
				if (auto cols = FindColumns<CFileNameColumn<std::shared_ptr<CShellFile>>>(); !cols.empty()) {
					std::static_pointer_cast<CFileNameCell<std::shared_ptr<CShellFile>>>(Cell(m_spCursorer->GetFocusedCell()->GetRowPtr(), cols.front().get()))->OnEdit(Event(GetWndPtr()));
					(*e.HandledPtr) = true;
				}
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

void CFilerGridView::Open(const std::shared_ptr<CShellFile>& spFile)
{
	if (auto spFolder = std::dynamic_pointer_cast<CShellFolder>(spFile)) {
		Folder.set(spFolder);
	} else {
		spFile->AddToRecentDocs();
		OpenFile(spFile);
	}
}

void CFilerGridView::OpenFolder(const std::shared_ptr<CShellFolder>& spFolder, bool isReload)
{
	//OpenFolder is called when DeviceChange, even inactive.
	if (!spFolder) {
		return;
	} else if (!spFolder->GetIsExist()) {
		Folder.set(Folder->GetParentFolderPtr());
		return;
	}

	if (m_pEdit) {
		EndEdit();
	}

	LOG_THIS_1("CFilerGridView::OpenFolder : " + wstr2str(spFolder->GetDispName()));
	LOG_SCOPED_TIMER_THIS_1("OpenFolder Total");
	{
		LOG_SCOPED_TIMER_THIS_1("OpenFolder Pre-Process");
		
		//StateMachine // Do not reset
		//m_pMachine.reset(new CGridStateMachine(this));
		//Direct2DWrite
		if (!isReload) {
			GetWndPtr()->GetDirectPtr()->ClearTextLayoutMap();
			//GetWndPtr()->GetDirectPtr()->GetFileThumbnailDrawerPtr()->Clear();
		}
		//Celler
		m_spCeller->Clear();

		//Cursor
		m_spCursorer->Clear();
		if (!isReload) {
			DeselectAll();
		}


		//Save and Restore Filter value
		if (!isReload) {
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
			shell::for_each_idl_in_shellfolder(GetWndPtr()->m_hWnd, Folder->GetShellFolderPtr(),
				[this](const CIDL& idl) {
					if (auto spFile = Folder->CreateShExFileFolder(idl)) {
						GetItemsSource().push_back(spFile);
					}
				});
		} catch (std::exception&) {
			throw std::exception(FILE_LINE_FUNC);
		}

		//Set up Watcher
		try {
			if (::PathFileExists(Folder->GetPath().c_str()) &&
				!boost::iequals(Folder->GetDispExt(), L".zip")) {
				m_spWatcher->QuitWatching();
				m_spWatcher->StartWatching(Folder->GetPath(), Folder->GetAbsoluteIdl());
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

		//GetHeaderColumnPtr()->SetIsFitMeasureValid(false);

		PostUpdate(Updates::Sort);
		PostUpdate(Updates::Filter);
		PostUpdate(Updates::ColumnVisible);
		PostUpdate(Updates::RowVisible);
		//PostUpdate(Updates::Column);
		PostUpdate(Updates::Row);
		PostUpdate(Updates::Scrolls);
		PostUpdate(Updates::Invalidate);

		//if (!isUpdate) {
		//	FolderChanged(Folder.get_shared_unconst());
		//}

		if (!isReload) {
			m_pVScroll->SetScrollPos(0);

			//If previous folder is found, set cursor for that.
			if (m_spPreviousFolder) {
				auto iter = std::find_if(ItemsSource->cbegin(), ItemsSource->cend(), [this](const auto& value)->bool {
					return value->GetAbsoluteIdl() == m_spPreviousFolder->GetAbsoluteIdl();
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
	Folder.set(Folder->GetParentFolderPtr());
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
		destIDL, Folder->GetAbsoluteIdl(), GetSelectedChildIDLVector());

	pDlg->OnCreate(CreateEvt(GetWndPtr(), GetWndPtr(), GetWndPtr()->CalcCenterRectF(CSizeF(400, 600))));
	GetWndPtr()->SetFocusToControl(pDlg);

	return true;
}

bool CFilerGridView::CopySelectedFilesTo(const CIDL& destIDL)
{
	//Create
	auto pDlg = std::make_shared<CCopyDlg>(
		GetWndPtr(), 
		destIDL, Folder->GetAbsoluteIdl(), GetSelectedChildIDLVector());

	pDlg->OnCreate(CreateEvt(GetWndPtr(), GetWndPtr(), GetWndPtr()->CalcCenterRectF(CSizeF(300, 400))));
	GetWndPtr()->SetFocusToControl(pDlg);

	return true;
}

bool CFilerGridView::MoveSelectedFilesTo(const CIDL& destIDL)
{
	//Create
	auto pDlg = std::make_shared<CMoveDlg>(
		GetWndPtr(),
		destIDL, Folder->GetAbsoluteIdl(), GetSelectedChildIDLVector());

	pDlg->OnCreate(CreateEvt(GetWndPtr(), GetWndPtr(), GetWndPtr()->CalcCenterRectF(CSizeF(300, 400))));
	GetWndPtr()->SetFocusToControl(pDlg);

	return true;
}

bool CFilerGridView::DeleteSelectedFiles()
{
	//Create
	auto pDlg = std::make_shared<CDeleteDlg>(
		GetWndPtr(),
		Folder->GetAbsoluteIdl(), GetSelectedChildIDLVector());

	pDlg->OnCreate(CreateEvt(GetWndPtr(), GetWndPtr(), GetWndPtr()->CalcCenterRectF(CSizeF(300, 400))));
	GetWndPtr()->SetFocusToControl(pDlg);

	return true;
}

bool CFilerGridView::NewFolder()
{
	m_isNewFile = true;
	return CShellContextMenu::InvokeNewShellContextmenuCommand(GetWndPtr()->m_hWnd, CMDSTR_NEWFOLDERA, Folder.get_shared_unconst());
}

bool CFilerGridView::CutToClipboard()
{
	return CShellContextMenu::InvokeNormalShellContextmenuCommand(GetWndPtr()->m_hWnd, "Cut", GetSelectedFiles());
}

bool CFilerGridView::CopyToClipboard()
{
	return CShellContextMenu::InvokeNormalShellContextmenuCommand(GetWndPtr()->m_hWnd, "Copy", GetSelectedFiles());
}

bool CFilerGridView::PasteFromClipboard()
{
	return CShellContextMenu::InvokeNormalShellContextmenuCommand(GetWndPtr()->m_hWnd, "Paste", Folder.get_shared_unconst());
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

void CFilerGridView::Normal_ContextMenu(const ContextMenuEvent& e)
{
	auto cell = Cell(GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
	std::vector<std::shared_ptr<CShellFile>> files = GetSelectedFiles();

	if (!cell) {
		//Folder menu
		auto id = GetFolderContextMenu().PopupFolder(GetWndPtr(), e.PointInScreen, Folder.get_shared_unconst());
		m_isNewFile = id >= CShellContextMenu::SCRATCH_QCM_NEW && id <= CShellContextMenu::SCRATCH_QCM_LAST;
		*e.HandledPtr = TRUE;
	}else if(/*cell->GetRowPtr() == m_pHeaderRow.get() || */cell->GetRowPtr() == m_pNameHeaderRow.get()){
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
				}
			}
		}
		*e.HandledPtr = TRUE;
	}else{
		auto id = GetFileContextMenu().PopupFiles(GetWndPtr(), e.PointInScreen, files);
		m_isNewFile = id >= CShellContextMenu::SCRATCH_QCM_NEW && id <= CShellContextMenu::SCRATCH_QCM_LAST;

		*e.HandledPtr = TRUE;
	}
}

std::wstring CFilerGridView::GetPath()const
{
	return Folder->GetPath();
}

void CFilerGridView::SetPath(const std::wstring& path)
{
	std::shared_ptr<CShellFile> spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(path);
	Open(spFile);
	//if (auto pFolder = std::dynamic_pointer_cast<CShellFolder>(spFile)) {
	//	OpenFolder(pFolder);
	//}
	//else {
	//	//Do nothing
	//}
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
	Folder->GetShellFolderPtr()->GetUIObjectOf(NULL, vPidl.size(), (LPCITEMIDLIST *)vPidl.data(), IID_IDataObject, NULL, (void **)&pDataObject);
			
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
