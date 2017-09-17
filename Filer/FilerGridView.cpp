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

#include "ConsoleTimer.h"
#include "ApplicationProperty.h"

#include "DropTarget.h"
#include "DropSource.h"

#include "FileDragger.h"

extern std::shared_ptr<CApplicationProperty> g_spApplicationProperty;

UINT CFilerGridView::WM_CHANGED = ::RegisterWindowMessage(L"CFilerGridView::WM_CHANGED");

CFilerGridView::CFilerGridView(std::wstring initPath, std::shared_ptr<CGridViewProperty> spGridViewProrperty)
			:CGridView(			
			spGridViewProrperty->m_spBackgroundProperty,
			spGridViewProrperty->m_spPropHeader,
			spGridViewProrperty->m_spPropCell,
			spGridViewProrperty->m_spPropCell,
			spGridViewProrperty->m_spDeltaScroll),
			m_initPath(initPath)
{
	m_cwa
	.dwExStyle(WS_EX_ACCEPTFILES);
	AddMsgHandler(CFilerGridView::WM_CHANGED,&CFilerGridView::OnFileChanged,this);
	AddMsgHandler(WM_INITMENUPOPUP,&CFilerGridView::OnInitMenuPopUp,this);
	AddMsgHandler(WM_DRAWITEM,&CFilerGridView::OnDrawItem,this);
	AddMsgHandler(WM_MEASUREITEM,&CFilerGridView::OnMeasureItem,this);
	AddCmdIDHandler(IDM_CUT,&CFilerGridView::OnCommandCut,this);
	//They are already assigned in GridView
	//AddCmdIDHandler(IDM_COPY,&CFilerGridView::OnCommandCopy,this);
	//AddCmdIDHandler(IDM_PASTE,&CFilerGridView::OnCommandPaste,this);
	//AddCmdIDHandler(IDM_DELETE,&CFilerGridView::OnCommandDelete,this);
	m_spFileDragger = std::make_shared<CFileDragger>();
	m_mouseObservers.push_back(m_spFileDragger);

	CellLButtonDblClk.connect(std::bind(&CFilerGridView::OnCellLButtonDblClk,this,std::placeholders::_1));
}

LRESULT CFilerGridView::OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	//
	auto pDropTarget = new CDropTarget(m_hWnd);

	pDropTarget->Dropped.connect([this](IDataObject *pDataObj, DWORD dwEffect)->void{	
		CComPtr<IShellItem2> pDestShellItem;
		CComPtr<IFileOperation> pFileOperation;

		HRESULT hr = ::SHCreateItemFromIDList(m_spFolder->GetAbsolutePidl(), IID_IShellItem2, reinterpret_cast<LPVOID*>(&pDestShellItem));
		if(FAILED(hr)){ return; }

		hr =pFileOperation.CoCreateInstance(CLSID_FileOperation);
		if(FAILED(hr)){ return; }

		switch(dwEffect){
		case DROPEFFECT_MOVE:
			{
				HRESULT hr = pFileOperation->MoveItems(pDataObj, pDestShellItem);
				if(FAILED(hr)){ return; }
				hr = pFileOperation->PerformOperations();
				if(FAILED(hr)){ return; }
			}
			break;
		case DROPEFFECT_COPY:
			{
				HRESULT hr = pFileOperation->CopyItems(pDataObj, pDestShellItem);
				hr = pFileOperation->PerformOperations();
				if(FAILED(hr)){ return; }
			}
			break;
		case DROPEFFECT_LINK:
			{
				FORMATETC formatetc = {0};
				formatetc.cfFormat = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_SHELLIDLIST);
				formatetc.ptd      = NULL;
				formatetc.dwAspect = DVASPECT_CONTENT;
				formatetc.lindex   = -1;
				formatetc.tymed    = TYMED_HGLOBAL;
	
				STGMEDIUM medium;
				HRESULT hr = pDataObj->GetData(&formatetc, &medium);
						if(FAILED(hr)){ return; }
				LPIDA pida = (LPIDA)GlobalLock(medium.hGlobal);
				CIDLPtr idlFolderPtr(::ILCloneFull((LPCITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[0])));
				for (UINT i = 0; i < pida->cidl; i++) {
					CIDLPtr idlChildPtr(::ILCloneFull((LPCITEMIDLIST)(((LPBYTE)pida)+pida->aoffset[1 + i])));
					CComPtr<IShellLink> pShellLink;
	
			
					pShellLink.CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER);
					pShellLink->SetIDList(idlFolderPtr + idlChildPtr);
					CComQIPtr<IPersistFile> pPersistFile(pShellLink);

					std::wstring destPath = m_spFolder->GetPath();
					WCHAR szSrcPath[MAX_PATH];
					WCHAR szDestPath[MAX_PATH];
					::SHGetPathFromIDList(idlFolderPtr + idlChildPtr, szSrcPath);
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

	});

	//pDropTarget->Dropped.connect([this](std::string verb, std::vector<CIDLPtr> idlPtrs)->void{
	//	for each(auto& idlPtr in idlPtrs){
	//		LPTSTR           lpszFileName;
	//		TCHAR            szSrcFilePath[256];
	//		TCHAR            szDestFilePath[256];
	//			SHGetPathFromIDList(idlPtr, szSrcFilePath);

	//			lpszFileName = PathFindFileName(szSrcFilePath);
	//			lstrcpy(szDestFilePath, m_spFolder->GetPath().c_str());
	//			PathAppend(szDestFilePath, lpszFileName);

	//			if (verb == "Move")
	//				MoveFile(szSrcFilePath, szDestFilePath);
	//			else if (verb == "Copy")
	//				CopyFile(szSrcFilePath, szDestFilePath, TRUE);
	//			else if (verb == "Link") {
	//				IShellLink   *pShellLink;
	//				IPersistFile *pPersistFile;
	//				WCHAR        szLinkPath[256];
	//				WCHAR        szUnicode[256];
	//				LPWSTR       lpsz;
	//		
	//				CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pShellLink));
	//				pShellLink->SetPath(szSrcFilePath);
	//				pShellLink->QueryInterface(IID_PPV_ARGS(&pPersistFile));

	//				lpsz = szDestFilePath;

	//				wsprintfW(szLinkPath, L"%s.lnk", lpsz);
	//				pPersistFile->Save(szLinkPath, TRUE);
	//				pPersistFile->Release();
	//				pShellLink->Release();
	//			}
	//			else
	//				;
	//			}
	//});
	 
	m_pDropTarget = CComPtr<IDropTarget>(pDropTarget);
	::RegisterDragDrop(m_hWnd, m_pDropTarget);

	m_pDropSource = CComPtr<IDropSource>(new CDropSource);

	m_pDragSourceHelper.CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER);

	//Desktop IShellFolder
	::SHGetDesktopFolder(&m_pDesktopShellFolder);

	//Watcher
	m_watcher.Changed.connect([this]()->void{::PostMessage(m_hWnd, CFilerGridView::WM_CHANGED,NULL,NULL);});

	//Base Create
	CGridView::OnCreate(uMsg,wParam,lParam,bHandled);

	//Open
	if(!m_initPath.empty()){
		SetPath(m_initPath);
	}else{
		//Open Desktop
		OpenFolder(std::make_shared<CShellFolder>());
	}
	return 0;
}

void CFilerGridView::OnKeyDown(KeyEventArgs& e)
{
	switch (e.Char)
	{
	case 'X':
		if(::GetAsyncKeyState(VK_CONTROL)){
			SendMessage(WM_COMMAND,IDM_CUT, NULL);
		}
		break;
	case 'C':
		if(::GetAsyncKeyState(VK_CONTROL)){
			SendMessage(WM_COMMAND,IDM_COPY,NULL);
		}
		break;
	case 'V':
		if(::GetAsyncKeyState(VK_CONTROL)){
			SendMessage(WM_COMMAND,IDM_PASTE,NULL);
		}
		break;
	case VK_DELETE:
		SendMessage(WM_COMMAND, IDM_DELETE,NULL);
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
			if(!m_spCursorer->GetFocusedRowColumn().IsInvalid()){
				if(auto p = dynamic_cast<CFileRow*>(m_spCursorer->GetFocusedRowColumn().GetRowPtr())){
					auto spFile = p->GetFilePointer();
					Open(spFile);
				}
			}

		}
		break;
	case VK_BACK:
		{//TODO Implement Back function in CShellFolder
			CIDLPtr parentIDL = m_spFolder->GetAbsolutePidl().GetPreviousIDLPtr();
			CIDLPtr grandParentIDL = parentIDL.GetPreviousIDLPtr();
			CComPtr<IShellFolder> pGrandParentFolder;
			m_pDesktopShellFolder->BindToObject(grandParentIDL,0,IID_IShellFolder,(void**)&pGrandParentFolder);

			if(!pGrandParentFolder){
				pGrandParentFolder = m_pDesktopShellFolder;
			}
			OpenFolder(std::make_shared<CShellFolder>(m_spFolder->GetParentShellFolderPtr(), pGrandParentFolder, parentIDL));
		}
		break;
	case VK_F7:
		{
			CComQIPtr<IStorage> pStorage(m_spFolder->GetShellFolderPtr());
			if (!pStorage) break;
			CComPtr<IStorage> dummy;
			unsigned int n = 0;
			while(STG_E_FILEALREADYEXISTS==pStorage->CreateStorage((L"New folder (" + boost::lexical_cast<std::wstring>(n) + L")").c_str(), STGM_FAILIFTHERE, 0, 0, &dummy)){
				n++;
			}

		}
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

	InsertRow(-1,m_rowHeader);
	InsertRow(-1,m_rowNameHeader);
	InsertRow(-1,m_rowFilter);

	//HeaderColumn
	auto pHeaderColumn=std::make_shared<CParentRowHeaderColumn>(this);
	InsertColumn(-1,pHeaderColumn);
	//IconColumn
	{
		auto pColumn = std::make_shared<CFileIconColumn>(this);
		InsertColumn(COLUMN_INDEX_MAX,pColumn);
	}
	//NameColumn
	{
		auto pColumn = std::make_shared<CFileNameColumn>(this);
		InsertColumn(COLUMN_INDEX_INVALID, pColumn);
	}

	//ExtColumn
	{
		auto pColumn = std::make_shared<CFileExtColumn>(this);
		InsertColumn(COLUMN_INDEX_MAX,pColumn);
	}

	//SizeColumn
	{
		auto pColumn = std::make_shared<CFileSizeColumn>(this);
		InsertColumn(COLUMN_INDEX_MAX,pColumn);
	}

	//GetLastWriteColumn
	{
		auto pColumn = std::make_shared<CFileLastWriteColumn>(this);
		InsertColumn(COLUMN_INDEX_MAX,pColumn);
	}
}

void CFilerGridView::Open(std::shared_ptr<CShellFile>& spFile)
{
	//Try BindToObject and EnumObjects to identify folder
	CIDLPtr pidl=spFile->GetAbsolutePidl();
	CComPtr<IShellFolder> pFolder;
	HRESULT hr = m_pDesktopShellFolder->BindToObject(pidl, 0, IID_IShellFolder, (void**)&pFolder);

	if(SUCCEEDED(hr)){
		CComPtr<IEnumIDList> enumIdl;
		hr = pFolder->EnumObjects(m_hWnd, SHCONTF_NONFOLDERS|SHCONTF_INCLUDEHIDDEN|SHCONTF_FOLDERS, &enumIdl);
	}

	if(SUCCEEDED(hr)){
		auto spFolder = std::make_shared<CShellFolder>(pFolder, m_spFolder->GetShellFolderPtr(), pidl);
		OpenFolder(spFolder);
	}else{
		OpenFile(spFile);
	}

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
	sei.lpIDList = (LPVOID)(spFile->GetAbsolutePidl().m_pIDL);

	::ShellExecuteEx( &sei);
}

void CFilerGridView::OpenFolder(std::shared_ptr<CShellFolder>& spFolder)
{
	CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, "OpenFolder")

	bool isUpdate = m_spFolder == spFolder;

	m_spCursorer->Clear();

	if(Empty()){
		InsertDefaultRowColumn();
	}

	try{
		m_spFolder = spFolder;

		//Clear RowDictionary From 0 to last
		auto& rowDictionary=m_rowAllDictionary.get<IndexTag>();
		rowDictionary.erase(rowDictionary.find(0),rowDictionary.end());

		//Set up Watcher
		if(::PathFileExists(m_spFolder->GetPath().c_str())){
			if(m_watcher.GetPath()!=m_spFolder->GetPath()){
				m_watcher.QuitWatching();
				m_watcher.SetPath(m_spFolder->GetPath());
				m_watcher.StartWatching();
			}
		}else{
			m_watcher.QuitWatching();
			m_watcher.SetPath(L"");
		}

		//Enumerate child IDL
		CComPtr<IEnumIDList> enumIdl;
		if(SUCCEEDED(m_spFolder->GetShellFolderPtr()->EnumObjects(m_hWnd, SHCONTF_NONFOLDERS|SHCONTF_INCLUDEHIDDEN|SHCONTF_FOLDERS, &enumIdl))){
			if(enumIdl){
				for(;;){

					CIDLPtr nextIdl;
					ULONG ulRet(0);
					if(SUCCEEDED(enumIdl->Next(1,&nextIdl,&ulRet))){
						if(nextIdl.IsEmpty()){break;}
						auto spFile(std::make_shared<CShellFile>(m_spFolder->GetShellFolderPtr(),::ILCombine(m_spFolder->GetAbsolutePidl(), nextIdl)));
						InsertRow(100,std::make_shared<CFileRow>(this,spFile));
					}
				}
			}
		}

		//Path change //TODO
		m_rowHeader->SetMeasureValid(false);
		m_rowNameHeader->SetMeasureValid(false);
		m_rowFilter->SetMeasureValid(false);

		for(auto iter=m_columnAllDictionary.begin();iter!=m_columnAllDictionary.end();++iter){
			std::dynamic_pointer_cast<CParentMapColumn>(iter->DataPtr)->Clear();
			iter->DataPtr->SetMeasureValid(false);
		}

	}catch(...){
		MessageBox(L"Error on Open", L"Error",MB_ICONWARNING);
	}

	PostUpdate(Updates::ColumnVisible);
	PostUpdate(Updates::RowVisible);
	PostUpdate(Updates::Column);
	PostUpdate(Updates::Row);
	PostUpdate(Updates::Scrolls);
	PostUpdate(Updates::Invalidate);
	SortAll();
	if(isUpdate){
		FilterAll();		
	}else{
		ClearFilter();
		FolderChanged(m_spFolder); 
	}
	SubmitUpdate();
}

void CFilerGridView::OnCellLButtonDblClk(CellEventArgs& e)
{
	auto pCell = e.CellPtr;
	if(auto p = dynamic_cast<CFileRow*>(e.CellPtr->GetRowPtr())){
		auto spFile = p->GetFilePointer();
		Open(spFile);
	}
}

void CFilerGridView::OnShellCommand(LPCSTR lpVerb)
{
	if(strcmp(lpVerb, "Paste")==0){
		std::vector<LPITEMIDLIST> vPidl;
		CIDLPtr idl = m_spFolder->GetAbsolutePidl().GetPreviousIDLPtr();
		CComPtr<IShellFolder> pDesktop;
		::SHGetDesktopFolder(&pDesktop);
		CComPtr<IShellFolder> pFolder;
		::SHBindToObject(pDesktop,idl,0,IID_IShellFolder,(void**)&pFolder);
		if(!pFolder){
			pFolder = pDesktop;
		}
		vPidl.push_back(m_spFolder->GetAbsolutePidl().FindLastID());
		InvokeShellCommand(m_hWnd, lpVerb, pFolder, vPidl);
	}else{

		auto& rowDictionary=m_rowVisibleDictionary.get<IndexTag>();
		auto& colDictionary=m_columnVisibleDictionary.get<IndexTag>();

		//int selectedNumber = 0;

		//for(auto rowIter=rowDictionary.begin(),rowEnd=rowDictionary.end();rowIter!=rowEnd;++rowIter){
		//	if(rowIter->DataPtr->GetSelected()){
		//		selectedNumber++;
		//	}
		//}
		std::vector<LPITEMIDLIST> vPidl;
		for(auto rowIter=rowDictionary.begin(),rowEnd=rowDictionary.end();rowIter!=rowEnd;++rowIter){
			if(rowIter->DataPtr->GetSelected()){
				if(auto spRow=std::dynamic_pointer_cast<CFileRow>(rowIter->DataPtr)){
					auto spFile = spRow->GetFilePointer();
					vPidl.push_back(spFile->GetAbsolutePidl().FindLastID());
				}
			}
		}

		InvokeShellCommand(m_hWnd, lpVerb, m_spFolder->GetShellFolderPtr(), vPidl);
	}
}

LRESULT CFilerGridView::OnCommandCut(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
{
	OnShellCommand("Cut");
	return 0;
}

LRESULT CFilerGridView::OnCommandCopy(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
{
	OnShellCommand("Copy");
	return 0;
}
LRESULT CFilerGridView::OnCommandPaste(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
{
	OnShellCommand("Paste");
	return 0;
}
LRESULT CFilerGridView::OnCommandDelete(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
{
	OnShellCommand("Delete");
	return 0;
}

LRESULT CFilerGridView::OnFileChanged(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	Sleep(50);
	//Remove duplicated messages
	MSG msg;
	while(::PeekMessage(&msg, m_hWnd, CFilerGridView::WM_CHANGED, CFilerGridView::WM_CHANGED, PM_REMOVE)==TRUE)
	{
		std::cout<<"WM_CHANGED Removed"<<std::endl;
	}
	OpenFolder(m_spFolder);
	return 0;
}

void CFilerGridView::InvokeShellCommand(HWND hWnd, LPCSTR lpVerb, CComPtr<IShellFolder> psf, std::vector<PITEMID_CHILD> vpIdl)
{
    DWORD dwAttribs=0;

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
}

void CFilerGridView::OnContextMenu(ContextMenuEventArgs& e)
{
	auto visibleIndexes = Point2VisibleIndexes(e.Point);
	auto& rowDictionary = m_rowVisibleDictionary.get<IndexTag>();
	auto& colDictionary = m_columnVisibleDictionary.get<IndexTag>();

	auto maxRow = boost::prior(rowDictionary.end())->DataPtr->GetVisibleIndex();
	//auto minRow = rowDictionary.begin()->DataPtr->GetVisibleIndex();
	auto maxCol = boost::prior(colDictionary.end())->DataPtr->GetVisibleIndex();
	//auto minCol = colDictionary.begin()->DataPtr->GetVisibleIndex();
	CPoint ptScreen(e.Point);
	ClientToScreen(ptScreen);
	std::vector<PITEMID_CHILD> vPidl;

	if(visibleIndexes.first > maxRow ||  visibleIndexes.second > maxCol){
		CIDLPtr idl = m_spFolder->GetAbsolutePidl().GetPreviousIDLPtr();
		CComPtr<IShellFolder> pDesktop;
		::SHGetDesktopFolder(&pDesktop);
		CComPtr<IShellFolder> pFolder;
		::SHBindToObject(pDesktop,idl,0,IID_IShellFolder,(void**)&pFolder);
		if(!pFolder){
			pFolder = pDesktop;
		}
		vPidl.push_back(m_spFolder->GetAbsolutePidl().FindLastID());
		ShowShellContextMenu(m_hWnd, ptScreen, pFolder, vPidl);
	}else{
		for(auto rowIter=rowDictionary.begin(),rowEnd=rowDictionary.end();rowIter!=rowEnd;++rowIter){
			if(rowIter->DataPtr->GetSelected()){
				auto spRow=std::dynamic_pointer_cast<CFileRow>(rowIter->DataPtr);
				auto spFile = spRow->GetFilePointer();
				vPidl.push_back(spFile->GetAbsolutePidl().FindLastID());
			}
		}
			ShowShellContextMenu(m_hWnd, ptScreen, m_spFolder->GetShellFolderPtr(), vPidl);
	}
}

void CFilerGridView::ShowShellContextMenu(HWND hWnd, CPoint ptScreen, CComPtr<IShellFolder> psf, std::vector<PITEMID_CHILD> vpIdl)
{
    CComPtr<IContextMenu> pcm;
	HRESULT hr=psf->GetUIObjectOf(hWnd, vpIdl.size(),(LPCITEMIDLIST*)(vpIdl.data()),IID_IContextMenu,nullptr,(LPVOID *)&pcm);
    if(SUCCEEDED(hr)){
		CMenu menu=CMenu(CreatePopupMenu());

		if(!menu.IsNull()){
			hr = pcm->QueryInterface(IID_PPV_ARGS(&m_pcm3));
            hr = m_pcm3->QueryContextMenu(menu,0,1,0x7fff,CMF_NORMAL);

            if (SUCCEEDED(hr)){
                int idCmd=TrackPopupMenu(menu, 
                                    TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, 
                                    ptScreen.x, 
                                    ptScreen.y,
                                    0,
                                    hWnd,
                                    NULL);

                if (idCmd){
					CMINVOKECOMMANDINFO cmi = {0};
                    cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
                    cmi.hwnd   = hWnd;
                    cmi.lpVerb = MAKEINTRESOURCEA(idCmd-1);
                    cmi.nShow        = SW_SHOWNORMAL;
                    hr=m_pcm3->InvokeCommand((LPCMINVOKECOMMANDINFO)&cmi);
                }
            }
			m_pcm3 = nullptr;
        }
    }
}

HRESULT CFilerGridView::OnInitMenuPopUp(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	if(m_pcm3){
		return m_pcm3->HandleMenuMsg(uMsg, wParam, lParam);
	}else{
		bHandled = FALSE;
		return -1;
	}
}

HRESULT CFilerGridView::OnDrawItem(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	if(m_pcm3){
		return m_pcm3->HandleMenuMsg(uMsg, wParam, lParam);
	}else{
		bHandled = FALSE;
		return -1;
	}
}

HRESULT CFilerGridView::OnMeasureItem(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	if(m_pcm3){
		return m_pcm3->HandleMenuMsg(uMsg, wParam, lParam);
	}else{
		bHandled = FALSE;
		return -1;
	}
}

CFilerGridView::string_type CFilerGridView::GetPath()const
{
	return m_spFolder->GetPath();
}

void CFilerGridView::SetPath(const string_type& path)
{
	ULONG         chEaten;
	ULONG         dwAttributes;
 
	CComPtr<IShellFolder> pDesktop;
	::SHGetDesktopFolder(&pDesktop);

	CIDLPtr pIDL;
	HRESULT hr = NULL;
	if(path == L""){
		hr  =::SHGetSpecialFolderLocation(m_hWnd, CSIDL_PROFILE, &pIDL);
	}else{
		hr = pDesktop->ParseDisplayName(
			NULL,
			NULL,
			const_cast<LPWSTR>(path.c_str()),
			&chEaten,
			&pIDL,
			&dwAttributes);
	}
	if (SUCCEEDED(hr))
	{
		CComPtr<IShellFolder> pFolder, pParentFolder;
		::SHBindToObject(pDesktop,pIDL,0,IID_IShellFolder,(void**)&pFolder);
		::SHBindToObject(pDesktop,pIDL.GetPreviousIDLPtr(),0,IID_IShellFolder,(void**)&pParentFolder);

		if(!pFolder){
			pFolder = pDesktop;
		}
		if(!pParentFolder){
			pParentFolder = pDesktop;
		}
		OpenFolder(std::make_shared<CShellFolder>(pFolder,pParentFolder, pIDL));
	}
}

void CFilerGridView::Drag()
{
	auto& rowDictionary=m_rowVisibleDictionary.get<IndexTag>();
	auto& colDictionary=m_columnVisibleDictionary.get<IndexTag>();

	//int selectedNumber = 0;

	//for(auto rowIter=rowDictionary.begin(),rowEnd=rowDictionary.end();rowIter!=rowEnd;++rowIter){
	//	if(rowIter->DataPtr->GetSelected()){
	//		selectedNumber++;
	//	}
	//}
	std::vector<LPITEMIDLIST> vPidl;
	CIDLPtr pFirstIdl;
	for(auto rowIter=rowDictionary.begin(),rowEnd=rowDictionary.end();rowIter!=rowEnd;++rowIter){
		if(rowIter->DataPtr->GetSelected()){
			if(auto spRow=std::dynamic_pointer_cast<CFileRow>(rowIter->DataPtr)){
				auto spFile = spRow->GetFilePointer();
				if(vPidl.empty()){
					pFirstIdl = spFile->GetAbsolutePidl();
				}
				vPidl.push_back(spFile->GetAbsolutePidl().FindLastID());
			}
		}
	}
	CComPtr<IDataObject> pDataObject;
	m_spFolder->GetShellFolderPtr()->GetUIObjectOf(NULL, vPidl.size(), (LPCITEMIDLIST *)vPidl.data(), IID_IDataObject, NULL, (void **)&pDataObject);
			
	SetDragImage(pFirstIdl, m_pDragSourceHelper, pDataObject);
	DWORD dwEffect;
	DoDragDrop(pDataObject, m_pDropSource, DROPEFFECT_MOVE | DROPEFFECT_COPY | DROPEFFECT_LINK, &dwEffect);
}

BOOL CFilerGridView::SetDragImage(CIDLPtr pFirstIdl, CComPtr<IDragSourceHelper> pDragSourceHelper, IDataObject *pDataObject)
{
	SHFILEINFO  fileInfo ={0};
	SHDRAGIMAGE dragImage = {0};
	POINT       pt = {0, 0};
	SIZE        size = {32, 32};

	SHGetFileInfo((LPCTSTR)(LPITEMIDLIST)pFirstIdl, 0, &fileInfo, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_ICON | SHGFI_LARGEICON);


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