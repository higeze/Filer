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

#include "MyWin32.h"

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

	AddCmdIDHandler(IDM_CUT,&CFilerGridView::OnCommandCut,this);
	//They are already assigned in GridView
	//AddCmdIDHandler(IDM_COPY,&CFilerGridView::OnCommandCopy,this);
	//AddCmdIDHandler(IDM_PASTE,&CFilerGridView::OnCommandPaste,this);
	//AddCmdIDHandler(IDM_DELETE,&CFilerGridView::OnCommandDelete,this);
	m_spItemDragger = std::make_shared<CFileDragger>();

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
			CIDLPtr idlFolderPtr(::ILCloneFull((LPCITEMIDLIST)(((LPBYTE)pida) + (pida)->aoffset[0])));

			//std::wstring path1;
			//::SHGetPathFromIDList(idlFolderPtr, ::GetBuffer(path1, MAX_PATH));
			//::ReleaseBuffer(path1);

			//std::wstring path2;
			//::SHGetPathFromIDList(m_spFolder->GetAbsolutePidl(), ::GetBuffer(path2, MAX_PATH));
			//::ReleaseBuffer(path2);

			if (::ILIsEqual(idlFolderPtr, m_spFolder->GetAbsolutePidl())) {
				//Do nothing
			}
			else {
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

	InsertRow(CRow::kMinIndex, m_rowFilter);
	InsertRow(CRow::kMinIndex,m_rowNameHeader);
	InsertRow(CRow::kMinIndex, m_rowHeader);

	//HeaderColumn
	auto pHeaderColumn=std::make_shared<CParentRowHeaderColumn>(this);
	InsertColumn(CColumn::kMinIndex,pHeaderColumn);
	//IconColumn
	{
		auto pColumn = std::make_shared<CFileIconColumn>(this);
		InsertColumn(CColumn::kMaxIndex,pColumn);
	}
	//NameColumn
	{
		auto pColumn = std::make_shared<CFileNameColumn>(this);
		InsertColumn(CColumn::kMaxIndex, pColumn);
	}

	//ExtColumn
	{
		auto pColumn = std::make_shared<CFileExtColumn>(this);
		InsertColumn(CColumn::kMaxIndex,pColumn);
	}

	//SizeColumn
	{
		auto pColumn = std::make_shared<CFileSizeColumn>(this);
		InsertColumn(CColumn::kMaxIndex,pColumn);
	}

	//GetLastWriteColumn
	{
		auto pColumn = std::make_shared<CFileLastWriteColumn>(this);
		InsertColumn(CColumn::kMaxIndex,pColumn);
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
	auto visibleIndexes = Coordinates2Indexes<VisTag>(e.Point);
	auto& rowDictionary = m_rowVisibleDictionary.get<IndexTag>();
	auto& colDictionary = m_columnVisibleDictionary.get<IndexTag>();

	auto maxRow = GetMaxIndex<RowTag, VisTag>();
	//auto minRow = rowDictionary.begin()->DataPtr->GetIndex<VisTag>();
	auto maxCol = GetMaxIndex<ColTag, VisTag>();
	//auto minCol = colDictionary.begin()->DataPtr->GetIndex<VisTag>();
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
		ShowShellContextMenu(m_hWnd, ptScreen, pFolder, vPidl, true);
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
#define SCRATCH_QCM_FIRST 1
#define SCRATCH_QCM_NEW 600//200,500 are used by system
#define SCRATCH_QCM_LAST  0x7FFF
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
			hr = psei->Initialize(m_spFolder->GetAbsolutePidl(), NULL, NULL);
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

		hr = m_pcm3->QueryContextMenu(menu, 0, SCRATCH_QCM_FIRST, SCRATCH_QCM_LAST, CMF_NORMAL);
		if (FAILED(hr)) { return; }

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
				if (verb.find(L"�V�K�쐬") != std::wstring::npos) {
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

			if (idCmd >= SCRATCH_QCM_NEW) {
				info.lpVerb = MAKEINTRESOURCEA(idCmd - SCRATCH_QCM_NEW);
				info.lpVerbW = MAKEINTRESOURCEW(idCmd - SCRATCH_QCM_NEW);
				hr = m_pcmNew3->InvokeCommand((LPCMINVOKECOMMANDINFO)&info);
			}
			else {
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