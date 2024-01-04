#include "FileOperationDlg.h"
#include "Textbox.h"
#include "TextBoxProperty.h"
#include "named_arguments.h"
#include "ShellFile.h"
#include "ShellFolder.h"
#include "scope_exit.h"
#include "future_then.h"
#include "Dispatcher.h"
#include "ThreadPool.h"
#include "PDFDoc.h"
#include "PDFViewProperty.h"
#include "ButtonProperty.h"
//TODOTODO
//RenameWnd

/*************************/
/* CFileOperationDlgBase */
/*************************/
CFileOperationDlgBase::CFileOperationDlgBase(
	CD2DWControl* pParentControl,
	const std::shared_ptr<DialogProperty>& spDialogProp,
	const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
	const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs)
	:CD2DWDialog(pParentControl, spDialogProp),
	m_spButtonDo(std::make_shared<CButton>(this, std::make_shared<ButtonProperty>())),
	m_spButtonCancel(std::make_shared<CButton>(this, std::make_shared<ButtonProperty>())),
	m_srcIDL(srcIDL), m_srcChildIDLs(srcChildIDLs), Dummy(std::make_shared<int>(0))
{
	m_spButtonCancel->Command.subscribe([this]()->void{
		GetWndPtr()->GetDispatcherPtr()->PostInvoke([this]() { OnClose(CloseEvent(GetWndPtr(), NULL, NULL)); });
	}, Dummy);

	m_spButtonCancel->Content.set(L"Cancel");
}

/*******************************/
/* CSimpleFileOperationDlgBase */
/*******************************/
std::tuple<CRectF, CRectF, CRectF> CSimpleFileOperationDlgBase::GetRects()
{
	CRectF rc = this->GetRectInWnd();
	CRectF rcTitle = this->GetTitleRect();
	rc.top = rcTitle.bottom;
	CRectF rcGrid(rc.left + 5.f, rc.top + 5.f, rc.right - 5.f, rc.bottom - 30.f);
	CRectF rcBtnCancel(rc.right - 5.f - 50.f, rc.bottom - 25.f, rc.right - 5.f, rc.bottom - 5.f);
	CRectF rcBtnDo(rcBtnCancel.left - 5.f - 50.f, rc.bottom - 25.f, rcBtnCancel.left - 5.f, rc.bottom - 5.f);

	return { rcGrid, rcBtnDo, rcBtnCancel };
}

void CSimpleFileOperationDlgBase::OnCreate(const CreateEvt& e)
{
	//Create
	CFileOperationDlgBase::OnCreate(e);
	//Size
	auto [rcGrid, rcBtnDo, rcBtnCancel] = GetRects();
		
	//Create FilerControl
	this->m_spFilerControl->OnCreate(CreateEvt(this->GetWndPtr(), this, rcGrid));

	//OK button
	this->m_spButtonDo->OnCreate(CreateEvt(this->GetWndPtr(), this, rcBtnDo));

	//Cancel button
	this->m_spButtonCancel->OnCreate(CreateEvt(this->GetWndPtr(), this, rcBtnCancel));

	//Focus
	this->GetWndPtr()->SetFocusToControl(this->m_spButtonDo);
}

void CSimpleFileOperationDlgBase::OnRect(const RectEvent& e)
{
	CD2DWControl::OnRect(e);

	auto [rcGrid, rcBtnDo, rcBtnCancel] = GetRects();		
	//Create FilerControl
	this->m_spFilerControl->OnRect(RectEvent(this->GetWndPtr(), rcGrid));
	//OK button
	this->m_spButtonDo->OnRect(RectEvent(this->GetWndPtr(), rcBtnDo));
	//Cancel button
	this->m_spButtonCancel->OnRect(RectEvent(this->GetWndPtr(), rcBtnCancel));
}

/********************/
/* CCopyMoveDlgBase */
/********************/
CCopyMoveDlgBase::CCopyMoveDlgBase(
	CD2DWControl* pParentControl,
	const std::shared_ptr<DialogProperty>& spDialogProp,
	const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
	const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs)//TOOD quit idl, change shellfile, folder
	:CSimpleFileOperationDlgBase(pParentControl, spDialogProp, spFilerGridViewProp, srcIDL, srcChildIDLs), m_destIDL(destIDL)
{
	//Items Source
	for (auto& childIDL : m_srcChildIDLs) {
		auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(
			shell::DesktopBindToShellFolder(m_srcIDL),
			m_srcIDL,
			childIDL);
		ItemsSource.emplace_back(
				spFile,
				RenameInfo{ spFile->GetPathNameWithoutExt(), spFile->GetPathExt() });
	}

	//FileGrid
	m_spFilerControl = std::make_unique<CFileOperationGridView>(
		this,
		spFilerGridViewProp,
		arg<"itemssource"_s>() = ItemsSource,
		arg<"hdrcol"_s>() = std::make_shared<CRowIndexColumn>(nullptr, spFilerGridViewProp->HeaderPropPtr),
		arg<"namecol"_s>() = std::make_shared<CFilePathNameColumn>(nullptr, L"Name"),
		arg<"frzcolcnt"_s>() = 1,
		arg<"columns"_s>() = std::vector<std::shared_ptr<CColumn>>{
			std::make_shared<CFilePathExtColumn>(nullptr, L"Ext"),
			std::make_shared<CFilePathRenameColumn>(nullptr, L"Rename"),
			std::make_shared<CFileReextColumn>(nullptr, L"Reext"),
			std::make_shared<CFileSizeColumn>(nullptr, spFilerGridViewProp->FileSizeArgsPtr),
			std::make_shared<CFileLastWriteColumn>(nullptr, spFilerGridViewProp->FileTimeArgsPtr)},
		arg<"namerow"_s>() = std::make_shared<CHeaderRow>(nullptr, spFilerGridViewProp->HeaderPropPtr),
		arg<"fltrow"_s>() = std::make_shared<CRow>(nullptr, spFilerGridViewProp->CellPropPtr),
		arg<"frzrowcnt"_s>() = 2);
}


/************/
/* CCopyDlg */
/************/
CCopyDlg::CCopyDlg(
	CD2DWControl* pParentControl, 
	const std::shared_ptr<DialogProperty>& spDialogProp,
	const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
	const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs)
	:CCopyMoveDlgBase(pParentControl, spDialogProp, spFilerGridViewProp, destIDL, srcIDL, srcChildIDLs)
{
	Title.set(L"Copy");
	m_spButtonDo->Command.subscribe([this]()->void
	{
		Copy();
	}, Dummy);

	m_spButtonDo->Content.set(L"Copy");
}

void CCopyDlg::Copy()
{
	std::vector<CIDL> noRenameIDLs;
	std::vector<std::pair<CIDL, std::wstring>> renameIDLs;

	for (auto& tup : *ItemsSource) {
		auto& spFile = tup.get<std::shared_ptr<CShellFile>>();
		auto& rename = tup.get<RenameInfo>();
		std::wstring newname = rename.Name + rename.Ext;
		if (spFile->GetPathName() == newname) {
			noRenameIDLs.push_back(spFile->GetAbsoluteIdl());
		} else {
			renameIDLs.emplace_back(spFile->GetAbsoluteIdl(), newname);
		}
	}

	auto fun = [this](const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& noRenameIDLs, const std::vector<std::pair<CIDL, std::wstring>>& renameIDLs)->void
	{
		CComPtr<IFileOperation> pFileOperation = nullptr;
		FAILED_RETURN(pFileOperation.CoCreateInstance(CLSID_FileOperation));

		CComPtr<IShellItem2> pDestShellItem;
		FAILED_RETURN(::SHCreateItemFromIDList(destIDL.ptr(), IID_IShellItem2, reinterpret_cast<LPVOID*>(&pDestShellItem)));

		if (!noRenameIDLs.empty()) {
			std::vector<LPITEMIDLIST> pidls;
			std::transform(std::begin(noRenameIDLs), std::end(noRenameIDLs), std::back_inserter(pidls), [](const CIDL& x) { return x.ptr(); });

			CComPtr<IShellItemArray> pItemAry = nullptr;
			FAILED_RETURN(SHCreateShellItemArrayFromIDLists(pidls.size(), (LPCITEMIDLIST*)(pidls.data()), &pItemAry));
			FAILED_RETURN(pFileOperation->CopyItems(pItemAry, pDestShellItem));
		}

		if (!renameIDLs.empty()) {
			for (auto& renamePair : renameIDLs) {
				CComPtr<IShellItem2> pSrcShellItem;
				FAILED_RETURN(::SHCreateItemFromIDList(renamePair.first.ptr(), IID_IShellItem2, reinterpret_cast<LPVOID*>(&pSrcShellItem)));
				FAILED_RETURN(pFileOperation->CopyItem(pSrcShellItem, pDestShellItem, renamePair.second.c_str(), nullptr));
			}
		}
		if (SUCCEEDED(pFileOperation->PerformOperations())) {
			GetWndPtr()->GetDispatcherPtr()->PostInvoke([this]() { OnClose(CloseEvent(GetWndPtr(), NULL, NULL)); });
		}

	};
	m_future = CThreadPool::GetInstance()->enqueue(
		fun,
		0,
		m_destIDL,
		m_srcIDL,
		noRenameIDLs,
		renameIDLs);
}

/************/
/* CMoveDlg */
/************/
CMoveDlg::CMoveDlg(
	CD2DWControl* pParentControl,
	const std::shared_ptr<DialogProperty>& spDialogProp,
	const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
	const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs)
	:CCopyMoveDlgBase(pParentControl, spDialogProp, spFilerGridViewProp, destIDL, srcIDL, srcChildIDLs)
{
	Title.set(L"Move");
	m_spButtonDo->Command.subscribe([this]()->void
	{
		Move();
	}, Dummy);

	m_spButtonDo->Content.set(L"Move");
}

void CMoveDlg::Move()
{
	std::vector<CIDL> noRenameIDLs;
	std::vector<std::pair<CIDL, std::wstring>> renameIDLs;

	for (auto iter = ItemsSource->cbegin(); iter != ItemsSource->cend(); ++ iter) {
		auto& spFile = iter->get<std::shared_ptr<CShellFile>>();
		auto& rename = iter->get<RenameInfo>();
		std::wstring newname = rename.Name + rename.Ext;
		if (spFile->GetPathName() == newname) {
			noRenameIDLs.push_back(spFile->GetAbsoluteIdl());
		} else {
			renameIDLs.emplace_back(spFile->GetAbsoluteIdl(), newname);
		}
	}

	auto fun = [this](const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& noRenameIDLs, const std::vector<std::pair<CIDL, std::wstring>>& renameIDLs)->void
	{
		CComPtr<IFileOperation> pFileOperation = nullptr;
		FAILED_RETURN(pFileOperation.CoCreateInstance(CLSID_FileOperation));

		CComPtr<IShellItem2> pDestShellItem;
		FAILED_RETURN(::SHCreateItemFromIDList(destIDL.ptr(), IID_IShellItem2, reinterpret_cast<LPVOID*>(&pDestShellItem)));

		if (!noRenameIDLs.empty()) {
			std::vector<LPITEMIDLIST> pidls;
			std::transform(std::begin(noRenameIDLs), std::end(noRenameIDLs), std::back_inserter(pidls), [](const CIDL& x) { return x.ptr(); });

			CComPtr<IShellItemArray> pItemAry = nullptr;
			FAILED_RETURN(SHCreateShellItemArrayFromIDLists(pidls.size(), (LPCITEMIDLIST*)(pidls.data()), &pItemAry));
			FAILED_RETURN(pFileOperation->MoveItems(pItemAry, pDestShellItem));
		}

		if (!renameIDLs.empty()) {
			for (auto& renamePair : renameIDLs) {
				CComPtr<IShellItem2> pSrcShellItem;
				FAILED_RETURN(::SHCreateItemFromIDList(renamePair.first.ptr(), IID_IShellItem2, reinterpret_cast<LPVOID*>(&pSrcShellItem)));
				FAILED_RETURN(pFileOperation->MoveItem(pSrcShellItem, pDestShellItem, renamePair.second.c_str(), nullptr));
			}
		}
		if (SUCCEEDED(pFileOperation->PerformOperations())) {
			GetWndPtr()->GetDispatcherPtr()->PostInvoke([this]() { OnClose(CloseEvent(GetWndPtr(), NULL, NULL)); });
		}

	};
	m_future = CThreadPool::GetInstance()->enqueue(
		fun,
		0,
		m_destIDL,
		m_srcIDL,
		noRenameIDLs,
		renameIDLs);
}

/**************/
/* CDeleteDlg */
/**************/
CDeleteDlg::CDeleteDlg(
	CD2DWControl* pParentControl,
	const std::shared_ptr<DialogProperty>& spDialogProp,
	const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
	const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs)
	:CSimpleFileOperationDlgBase(pParentControl, spDialogProp, spFilerGridViewProp, srcIDL, srcChildIDLs)
{
	Title.set(L"Delete");
	//Items Source
	for (auto& childIDL : m_srcChildIDLs) {
		auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(
			shell::DesktopBindToShellFolder(m_srcIDL),
			m_srcIDL,
			childIDL);
		ItemsSource.push_back(spFile);
	}

	//FileGrid
	m_spFilerControl = std::make_unique<CFileOperationGridView>(
		this,
		spFilerGridViewProp,
		arg<"itemssource"_s>() = ItemsSource,
		arg<"hdrcol"_s>() = std::make_shared<CRowIndexColumn>(nullptr, spFilerGridViewProp->HeaderPropPtr),
		arg<"namecol"_s>() = std::make_shared<CFilePathNameColumn>(nullptr, L"Name"),
		arg<"frzcolcnt"_s>() = 1,
		arg<"columns"_s>() = std::vector<std::shared_ptr<CColumn>>{
			std::make_shared<CFilePathExtColumn>(nullptr, L"Ext"),
			std::make_shared<CFileSizeColumn>(nullptr, spFilerGridViewProp->FileSizeArgsPtr),
			std::make_shared<CFileLastWriteColumn>(nullptr, spFilerGridViewProp->FileTimeArgsPtr) },
		arg<"namerow"_s>() = std::make_shared<CHeaderRow>(nullptr, spFilerGridViewProp->HeaderPropPtr),
		arg<"fltrow"_s>() = std::make_shared<CRow>(nullptr, spFilerGridViewProp->CellPropPtr),
		arg<"frzrowcnt"_s>() = 2);

	m_spButtonDo->Command.subscribe([this]()->void
	{
		Delete();
	}, Dummy);

	m_spButtonDo->Content.set(L"Delete");

}

void CDeleteDlg::Delete()
{
	std::vector<CIDL> delIDLs;

	for (auto iter = ItemsSource->cbegin(); iter != ItemsSource->cend(); ++iter) {
		auto& spFile = iter->get<std::shared_ptr<CShellFile>>();
		delIDLs.push_back(spFile->GetAbsoluteIdl());
	}

	auto fun = [this](const std::vector<CIDL>& delIDLs)->void
	{
		CComPtr<IFileOperation> pFileOperation = nullptr;
		FAILED_RETURN(pFileOperation.CoCreateInstance(CLSID_FileOperation));

		if (!delIDLs.empty()) {
			std::vector<LPITEMIDLIST> pidls;
			std::transform(std::begin(delIDLs), std::end(delIDLs), std::back_inserter(pidls), [](const CIDL& x) { return x.ptr(); });

			CComPtr<IShellItemArray> pItemAry = nullptr;
			FAILED_RETURN(SHCreateShellItemArrayFromIDLists(pidls.size(), (LPCITEMIDLIST*)(pidls.data()), &pItemAry));
			FAILED_RETURN(pFileOperation->DeleteItems(pItemAry));
		}
		if (SUCCEEDED(pFileOperation->PerformOperations())) {
			GetWndPtr()->GetDispatcherPtr()->PostInvoke([this]() { OnClose(CloseEvent(GetWndPtr(), NULL, NULL)); });
		}
	};

	m_future = CThreadPool::GetInstance()->enqueue(
		fun,
		0,
		delIDLs);
}

/********************/
/* CExeExtensionDlg */
/********************/

CExeExtensionDlg::CExeExtensionDlg(
	CD2DWControl* pParentControl,
	const std::shared_ptr<DialogProperty>& spDialogProp,
	const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
	const std::shared_ptr<TextBoxProperty>& spTextBoxProp,
	const std::shared_ptr<CShellFolder>& folder,
	const std::vector<std::shared_ptr<CShellFile>>& files,
	const ExeExtension& exeExtension)
	:CFileOperationDlgBase(pParentControl, spDialogProp, spFilerGridViewProp, CIDL(), std::vector<CIDL>()),
	m_spTextPath(std::make_shared<CTextBox>(this, spTextBoxProp, L"")),
	m_spTextParam(std::make_shared<CTextBox>(this, spTextBoxProp, L"")),
	m_exeExtension(exeExtension)
{
	m_spTextParam->Text.set(m_exeExtension.Parameter);

	m_spTextParam->Text.subscribe([this](auto notify)
	{
		m_exeExtension.Parameter = notify.all_items;
	}, Dummy);

	m_spTextPath->Text.set(m_exeExtension.Path);
	m_spTextPath->Text.subscribe([this](auto notify)
	{
		m_exeExtension.Path = notify.all_items;
	}, Dummy);

	Title.set(L"Exe");
	//Items Source
	for (auto& file : files) {
		ItemsSource.emplace_back(std::move(file));
	}

	//FileGrid
	m_spFilerControl = std::make_unique<CFileOperationGridView>(
		this,
		spFilerGridViewProp,
		arg<"itemssource"_s>() = ItemsSource,
		arg<"hdrcol"_s>() = std::make_shared<CRowIndexColumn>(nullptr, spFilerGridViewProp->HeaderPropPtr),
		arg<"namecol"_s>() = std::make_shared<CFilePathNameColumn>(nullptr, L"Name"),
		arg<"frzcolcnt"_s>() = 1,
		arg<"columns"_s>() = std::vector<std::shared_ptr<CColumn>>{
			std::make_shared<CFilePathExtColumn>(nullptr, L"Ext"),
			std::make_shared<CFileSizeColumn>(nullptr, spFilerGridViewProp->FileSizeArgsPtr),
			std::make_shared<CFileLastWriteColumn>(nullptr, spFilerGridViewProp->FileTimeArgsPtr) },
		arg<"namerow"_s>() = std::make_shared<CHeaderRow>(nullptr, spFilerGridViewProp->HeaderPropPtr),
		arg<"fltrow"_s>() = std::make_shared<CRow>(nullptr, spFilerGridViewProp->CellPropPtr),
		arg<"frzrowcnt"_s>() = 2);

	//TextBox
	m_spTextPath->SetHasBorder(true);
	m_spTextParam->SetHasBorder(true);

	m_spButtonDo->Command.subscribe([this]()->void
	{
		Execute();
	}, Dummy);

	m_spButtonDo->Content.set(L"Exe");

}

void CExeExtensionDlg::Execute()
{
try {
	std::vector<std::shared_ptr<CShellFile>> files = m_spFilerControl->GetAllFiles();
	std::vector<std::wstring> filePaths;
	for (const auto& file : files) {
		filePaths.emplace_back(L"\"" + file->GetPath() + L"\"");
	}

	std::wstring fileMultiPath = boost::join(filePaths, L" ");

	std::wstring parameter = m_exeExtension.Parameter;

	std::wstring singlePathUnQuo = filePaths[0];
	::PathUnquoteSpaces(::GetBuffer(singlePathUnQuo, MAX_PATH));
	::ReleaseBuffer(singlePathUnQuo);

	std::wstring singlePathWoExtUnQuo = singlePathUnQuo;
	::PathRemoveExtension(::GetBuffer(singlePathWoExtUnQuo, MAX_PATH));
	::ReleaseBuffer(singlePathWoExtUnQuo);

	boost::algorithm::replace_all(parameter, L"%SinglePath%", filePaths[0]);
	boost::algorithm::replace_all(parameter, L"%SinglePathUnQuo%", singlePathUnQuo);
	boost::algorithm::replace_all(parameter, L"%MultiPath%", fileMultiPath);
	boost::algorithm::replace_all(parameter, L"%DirectoryUnQuo%", ::PathFindDirectory(singlePathUnQuo.c_str()));
	boost::algorithm::replace_all(parameter, L"%SinglePathWoExtUnQuo%", singlePathWoExtUnQuo );	
							
	std::wstring cmdline = L"\"" + m_exeExtension.Path + L"\" " + parameter;

	HANDLE hRead, hWrite;
	SECURITY_ATTRIBUTES sa = { 0 };
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = 0;
	sa.bInheritHandle = TRUE;
	if (!::CreatePipe(&hRead, &hWrite, &sa, 0)) {
		return;
	}
	do {
		STARTUPINFO si = { 0 };
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESTDHANDLES;
		si.wShowWindow = SW_HIDE;
		si.hStdOutput = hWrite;
		si.hStdError = hWrite;

		PROCESS_INFORMATION pi = { 0 };
		DWORD len = 0;
		LOG_THIS_2("CreateProcess:", wstr2str(cmdline));

		if (!::CreateProcess(NULL, const_cast<LPWSTR>(cmdline.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) break;
		if (!::WaitForInputIdle(pi.hProcess, INFINITE) != 0)break;
		if (::WaitForSingleObject(pi.hProcess, INFINITE) != WAIT_OBJECT_0)break;

		DWORD exitCode = 0;
		if (!::GetExitCodeProcess(pi.hProcess, &exitCode))break;
		LOG_THIS_2("ExitCode:", exitCode);

		::CloseHandle(pi.hThread);
		::CloseHandle(pi.hProcess);

		if (!::PeekNamedPipe(hRead, NULL, 0, NULL, &len, NULL))break;
		std::string buff;
		if (len > 0 && ReadFile(hRead, (LPVOID)::GetBuffer(buff, len), len, &len, NULL)) {
			::ReleaseBuffer(buff);
			LOG_THIS_2("Output:", buff);
		}

	} while (0);

	::CloseHandle(hRead);
	::CloseHandle(hWrite);
	GetWndPtr()->GetDispatcherPtr()->PostInvoke([this]() { OnClose(CloseEvent(GetWndPtr(), NULL, NULL)); });

} catch (...) {
	throw;
}
}

std::tuple<CRectF, CRectF, CRectF, CRectF, CRectF> CExeExtensionDlg::GetRects()
{
	CRectF rc = GetRectInWnd();
	CRectF rcTitle = GetTitleRect();
	rc.top = rcTitle.bottom;
	CRectF rcBtnCancel(rc.right - 5.f - 50.f, rc.bottom - 25.f, rc.right - 5.f, rc.bottom - 5.f);
	CRectF rcBtnDo(rcBtnCancel.left - 5.f - 50.f, rc.bottom - 25.f, rcBtnCancel.left - 5.f, rc.bottom - 5.f);
	CRectF rcTextParam(rc.left + 5.f, rc.bottom - 95.f, rc.right - 5.f, rc.bottom - 30.f);
	CRectF rcTextPath(rc.left + 5.f, rc.bottom - 120, rc.right - 5.f, rc.bottom - 100.f);
	CRectF rcGrid(rc.left + 5.f, rc.top + 5.f, rc.right - 5.f, rc.bottom -  125.f);

	return { rcGrid, rcTextPath, rcTextParam, rcBtnDo, rcBtnCancel };
}


void CExeExtensionDlg::OnCreate(const CreateEvt& e)
{
	//Base
	CD2DWDialog::OnCreate(e);
	
	//Size
	auto [rcGrid, rcTextPath, rcTextParam, rcBtnDo, rcBtnCancel] = GetRects();
		
	//Create FilerControl
	m_spFilerControl->OnCreate(CreateEvt(GetWndPtr(), this, rcGrid));

	//Textbox
	m_spTextPath->OnCreate(CreateEvt(GetWndPtr(), this, rcTextPath));
	m_spTextParam->OnCreate(CreateEvt(GetWndPtr(), this, rcTextParam));

	//OK button
	m_spButtonDo->OnCreate(CreateEvt(GetWndPtr(), this, rcBtnDo));

	//Cancel button
	m_spButtonCancel->OnCreate(CreateEvt(GetWndPtr(), this, rcBtnCancel));

	//Focus
	GetWndPtr()->SetFocusToControl(m_spButtonDo);
}

void CExeExtensionDlg::OnRect(const RectEvent& e)
{
	CD2DWControl::OnRect(e);

	auto [rcGrid, rcTextPath, rcTextParam, rcBtnDo, rcBtnCancel] = GetRects();		
	m_spFilerControl->OnRect(RectEvent(GetWndPtr(), rcGrid));
	m_spTextPath->OnRect(RectEvent(GetWndPtr(), rcTextPath));
	m_spTextParam->OnRect(RectEvent(GetWndPtr(), rcTextParam));
	m_spButtonDo->OnRect(RectEvent(GetWndPtr(), rcBtnDo));
	m_spButtonCancel->OnRect(RectEvent(GetWndPtr(), rcBtnCancel));
}

/************************/
/* CPDFOperationDlgBase */
/************************/

CPDFOperationDlgBase::CPDFOperationDlgBase(
	CD2DWControl* pParentControl,
	const std::shared_ptr<DialogProperty>& spDialogProp,
	const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
	const std::shared_ptr<TextBoxProperty>& spTextBoxProp,
	const std::shared_ptr<CShellFolder>& folder,
	const std::vector<std::shared_ptr<CShellFile>>& files)
	:CFileOperationDlgBase(pParentControl, spDialogProp, spFilerGridViewProp, CIDL(), std::vector<CIDL>()),
	m_spParameter(std::make_shared<CTextBox>(this, spTextBoxProp, L""))
{
	//Items Source
	for (auto& file : files) {
		ItemsSource.emplace_back(std::move(file));
	}

	//FileGrid
	m_spFilerControl = std::make_unique<CFileOperationGridView>(
		this,
		spFilerGridViewProp,
		arg<"itemssource"_s>() = ItemsSource,
		arg<"hdrcol"_s>() = std::make_shared<CRowIndexColumn>(nullptr, spFilerGridViewProp->HeaderPropPtr),
		arg<"namecol"_s>() = std::make_shared<CFilePathNameColumn>(nullptr, L"Name"),
		arg<"frzcolcnt"_s>() = 1,
		arg<"columns"_s>() = std::vector<std::shared_ptr<CColumn>>{
			std::make_shared<CFilePathExtColumn>(nullptr, L"Ext"),
			std::make_shared<CFileSizeColumn>(nullptr, spFilerGridViewProp->FileSizeArgsPtr),
			std::make_shared<CFileLastWriteColumn>(nullptr, spFilerGridViewProp->FileTimeArgsPtr) },
		arg<"namerow"_s>() = std::make_shared<CHeaderRow>(nullptr, spFilerGridViewProp->HeaderPropPtr),
		arg<"fltrow"_s>() = std::make_shared<CRow>(nullptr, spFilerGridViewProp->CellPropPtr),
		arg<"frzrowcnt"_s>() = 2);
}

std::tuple<CRectF, CRectF, CRectF, CRectF> CPDFOperationDlgBase::GetRects()
{
	CRectF rc = GetRectInWnd();
	CRectF rcTitle = GetTitleRect();
	rc.top = rcTitle.bottom;
	CRectF rcBtnCancel(rc.right - 5.f - 50.f, rc.bottom - 25.f, rc.right - 5.f, rc.bottom - 5.f);
	CRectF rcBtnDo(rcBtnCancel.left - 5.f - 50.f, rc.bottom - 25.f, rcBtnCancel.left - 5.f, rc.bottom - 5.f);
	CRectF rcParam(rc.left + 5.f, rc.bottom - 95.f, rc.right - 5.f, rc.bottom - 30.f);
	CRectF rcGrid(rc.left + 5.f, rc.top + 5.f, rc.right - 5.f, rc.bottom - 100.f);

	return { rcGrid, rcParam, rcBtnDo, rcBtnCancel };
}


void CPDFOperationDlgBase::OnCreate(const CreateEvt& e)
{
	//Base
	CD2DWDialog::OnCreate(e);
	
	//Size
	auto [rcGrid, rcParam, rcBtnDo, rcBtnCancel] = GetRects();
		
	m_spFilerControl->OnCreate(CreateEvt(GetWndPtr(), this, rcGrid));
	m_spParameter->OnCreate(CreateEvt(GetWndPtr(), this, rcParam));
	m_spButtonDo->OnCreate(CreateEvt(GetWndPtr(), this, rcBtnDo));
	m_spButtonCancel->OnCreate(CreateEvt(GetWndPtr(), this, rcBtnCancel));
	GetWndPtr()->SetFocusToControl(m_spButtonDo);
}

void CPDFOperationDlgBase::OnRect(const RectEvent& e)
{
	CD2DWControl::OnRect(e);

	auto [rcGrid, rcParam, rcBtnDo, rcBtnCancel] = GetRects();		

	m_spFilerControl->OnRect(RectEvent(GetWndPtr(), rcGrid));
	m_spParameter->OnRect(RectEvent(GetWndPtr(), rcParam));
	m_spButtonDo->OnRect(RectEvent(GetWndPtr(), rcBtnDo));
	m_spButtonCancel->OnRect(RectEvent(GetWndPtr(), rcBtnCancel));
}

/****************/
/* CPDFSplitDlg */
/****************/
CPDFSplitDlg::CPDFSplitDlg(
	CD2DWControl* pParentControl,
	const std::shared_ptr<DialogProperty>& spDialogProp,
	const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
	const std::shared_ptr<TextBoxProperty>& spTextBoxProp,
	const std::shared_ptr<CShellFolder>& folder,
	const std::vector<std::shared_ptr<CShellFile>>& files)
	:CPDFOperationDlgBase(pParentControl, spDialogProp, spFilerGridViewProp, spTextBoxProp, folder, files)
{
	Title.set(L"PDF Split");
	m_spButtonDo->Content.set(L"Split");
	m_spButtonDo->Command.subscribe([this]()->void
	{
		std::vector<std::shared_ptr<CShellFile>> files = m_spFilerControl->GetAllFiles();
		for (auto& file : files) {
			CPDFDoc doc;
			doc.Open(file->GetPath(), L"");
			doc.SplitSave();
		}
		GetWndPtr()->GetDispatcherPtr()->PostInvoke([this]() { OnClose(CloseEvent(GetWndPtr(), NULL, NULL)); });
	}, Dummy);
}

/****************/
/* CPDFMergeDlg */
/****************/
CPDFMergeDlg::CPDFMergeDlg(
	CD2DWControl* pParentControl,
	const std::shared_ptr<DialogProperty>& spDialogProp,
	const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
	const std::shared_ptr<TextBoxProperty>& spTextBoxProp,
	const std::shared_ptr<CShellFolder>& folder,
	const std::vector<std::shared_ptr<CShellFile>>& files)
	:CPDFOperationDlgBase(pParentControl, spDialogProp, spFilerGridViewProp, spTextBoxProp, folder, files)
{
	Title.set(L"PDF Merge");

	m_spParameter->Text.set(files[0]->GetPathWithoutExt() + L"_merge.pdf");
	m_spButtonDo->Content.set(L"Merge");
	m_spButtonDo->Command.subscribe([this]()->void
	{
		std::vector<std::shared_ptr<CShellFile>> files = m_spFilerControl->GetAllFiles();
		CPDFDoc doc;
		doc.Create();
		int minVersion = 0;
		for (auto spFile : files) {
			CPDFDoc srcDoc;
			srcDoc.Open(spFile->GetPath(), L"");
			minVersion = std::min(minVersion, srcDoc.GetFileVersion());
			auto count = doc.GetPageCount();
			doc.ImportPages(srcDoc, NULL, count);		
		}
		doc.SaveAs(*m_spParameter->Text, minVersion, false);
		
		GetWndPtr()->GetDispatcherPtr()->PostInvoke([this]() { OnClose(CloseEvent(GetWndPtr(), NULL, NULL)); });
	}, Dummy);
}

/******************/
/* CPDFExtractDlg */
/******************/
CPDFExtractDlg::CPDFExtractDlg(
	CD2DWControl* pParentControl,
	const std::shared_ptr<DialogProperty>& spDialogProp,
	const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
	const std::shared_ptr<TextBoxProperty>& spTextBoxProp,
	const std::shared_ptr<CShellFolder>& folder,
	const std::vector<std::shared_ptr<CShellFile>>& files)
	:CPDFOperationDlgBase(pParentControl, spDialogProp, spFilerGridViewProp, spTextBoxProp, folder, files)
{
	Title.set(L"PDF Extract");

	m_spParameter->Text.set(L"");
	m_spButtonDo->Content.set(L"Extract");

	m_spButtonDo->Command.subscribe([this]()->void
	{
		std::vector<std::shared_ptr<CShellFile>> files = m_spFilerControl->GetAllFiles();
		for (auto& file : files) {
			CPDFDoc doc;
			doc.Open(file->GetPath(), L"");
			std::wstring param = *m_spParameter->Text;
			boost::algorithm::replace_all(param, L"first", L"1");
			boost::algorithm::replace_all(param, L"last", std::to_wstring(doc.GetPageCount()));
			CPDFDoc dst_doc(doc.Extract(param));
			dst_doc.SaveAs(std::format(L"{}_{}.pdf", file->GetPathWithoutExt(), *m_spParameter->Text), doc.GetFileVersion(), false);
		}

		GetWndPtr()->GetDispatcherPtr()->PostInvoke([this]() { OnClose(CloseEvent(GetWndPtr(), NULL, NULL)); });
	}, Dummy);
}

/*************************/
/* CPDFUnlockDlg */
/*************************/
CPDFUnlockDlg::CPDFUnlockDlg(
	CD2DWControl* pParentControl,
	const std::shared_ptr<DialogProperty>& spDialogProp,
	const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
	const std::shared_ptr<TextBoxProperty>& spTextBoxProp,
	const std::shared_ptr<CShellFolder>& folder,
	const std::vector<std::shared_ptr<CShellFile>>& files)
	:CPDFOperationDlgBase(pParentControl, spDialogProp, spFilerGridViewProp, spTextBoxProp, folder, files)
{
	Title.set(L"PDF Unlock");

	m_spParameter->Text.set(L"");
	m_spButtonDo->Content.set(L"Unlock");

	m_spButtonDo->Command.subscribe([this]()->void
	{
		std::vector<std::shared_ptr<CShellFile>> files = m_spFilerControl->GetAllFiles();
		for (auto& file : files) {
			CPDFDoc doc;
			doc.Open(file->GetPath(), *m_spParameter->Text);
			doc.SaveAs(std::format(L"{}{}.pdf", file->GetPathWithoutExt(), L"_unlock"),  doc.GetFileVersion(), true);
		}

		GetWndPtr()->GetDispatcherPtr()->PostInvoke([this]() { OnClose(CloseEvent(GetWndPtr(), NULL, NULL)); });
	}, Dummy);
}