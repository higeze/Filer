#include "FileOperationDlg.h"
#include "Textbox.h"
#include "TextBoxProperty.h"
#include "named_arguments.h"
#include "ShellFile.h"
#include "ShellFolder.h"
#include "scope_exit.h"
#include "future_then.h"
#include "Dispatcher.h"
#include "async_catch.h"
//TODOTODO
//RenameWnd

/********************/
/* CCopyMoveDlgBase */
/********************/
CCopyMoveDlgBase::CCopyMoveDlgBase(
	CD2DWControl* pParentControl,
	const std::shared_ptr<DialogProperty>& spDialogProp,
	const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
	const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs)
	:CSimpleFileOperationDlgBase(pParentControl, spDialogProp, spFilerGridViewProp, srcIDL, srcChildIDLs), m_destIDL(destIDL)
{
	//Items Source
	for (auto& childIDL : m_srcChildIDLs) {
		auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(
			shell::DesktopBindToShellFolder(m_srcIDL),
			m_srcIDL,
			childIDL);
		m_spItemsSource->push_back(
			std::make_tuple(
				spFile,
				RenameInfo{ spFile->GetPathNameWithoutExt(), spFile->GetPathExt() }));
	}

	//FileGrid
	m_spFilerControl = std::make_unique<CFileOperationGridView<std::shared_ptr<CShellFile>, RenameInfo>>(
		this,
		spFilerGridViewProp,
		m_spItemsSource,
		arg<"hdrcol"_s>() = std::make_shared<CRowIndexColumn>(nullptr),
		arg<"namecol"_s>() = std::make_shared<CFilePathNameColumn<std::shared_ptr<CShellFile>, RenameInfo>>(nullptr, L"Name"),
		arg<"frzcolcnt"_s>() = 1,
		arg<"columns"_s>() = std::vector<std::shared_ptr<CColumn>>{
			std::make_shared<CFilePathExtColumn<std::shared_ptr<CShellFile>, RenameInfo>>(nullptr, L"Ext"),
			std::make_shared<CFilePathRenameColumn<std::shared_ptr<CShellFile>, RenameInfo>>(nullptr, L"Rename"),
			std::make_shared<CFileReextColumn<std::shared_ptr<CShellFile>, RenameInfo>>(nullptr, L"Reext"),
			std::make_shared<CFileSizeColumn<std::shared_ptr<CShellFile>, RenameInfo>>(nullptr, spFilerGridViewProp->FileSizeArgsPtr),
			std::make_shared<CFileLastWriteColumn<std::shared_ptr<CShellFile>, RenameInfo>>(nullptr, spFilerGridViewProp->FileTimeArgsPtr)},
		arg<"namerow"_s>() = std::make_shared<CHeaderRow>(nullptr),
		arg<"fltrow"_s>() = std::make_shared<CRow>(nullptr),
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
	m_title.set(L"Copy");
	m_spButtonDo->GetCommand().Subscribe([this]()->void
	{
		Copy();
	});

	m_spButtonDo->GetContent().set(L"Copy");
}

void CCopyDlg::Copy()
{
	std::vector<CIDL> noRenameIDLs;
	std::vector<std::pair<CIDL, std::wstring>> renameIDLs;

	for (auto& tup : m_spItemsSource->get()) {
		auto& spFile = std::get<std::shared_ptr<CShellFile>>(tup);
		auto& rename = std::get<RenameInfo>(tup);
		std::wstring newname = rename.Name + rename.Ext;
		if (spFile->GetPathName() == newname) {
			noRenameIDLs.push_back(spFile->GetAbsoluteIdl());
		} else {
			renameIDLs.emplace_back(spFile->GetAbsoluteIdl(), newname);
		}
	}
	//auto test1 = async_action_nothrow(std::launch::async, [](int&&, int&&) { }, 10, 11);//works

	//auto test2 = async_action_nothrow(std::launch::async, [](const int&, const int&) { }, 10, 11);//works

	//int a = 29;
	//auto test3 = async_action_nothrow(std::launch::async, [](int&&, int&&) { }, std::forward<int>(a), std::forward<int>(a));//works

	//auto test4 = async_action_nothrow(std::launch::async, [](int&&, int&&) { }, 10, std::forward<int>(a));//works

	//auto test5 = async_action_nothrow(std::launch::async, [](const int&, const int&) { }, std::forward<int>(a), std::forward<int>(a));//works

	//auto test6 = async_action_nothrow(std::launch::async, [](const int&, const int&) { }, 10, std::forward<int>(a));//works

	//auto test7 = async_action_nothrow(std::launch::async, [](const int&, const int&) { }, std::move(a), std::move(a));//works

	//auto test8 = async_action_nothrow(std::launch::async, [](const int&, const int&) { }, 10, std::move(a));//works

	//auto test9 = async_action_nothrow(std::launch::async, [](int&&, const int&) { }, 10, std::ref(a));//works

	//auto test10 = async_action_nothrow(std::launch::async, std::forward<std::function<void(int&, const int)>>([]( int&, const int)->void { }), 10, a);// not works
	//auto fun = [](const int&, const int&)->void {};
	//auto test11 = std::async(std::launch::async, async_action_wrap<decltype(fun)&&, const int&, const int&>, fun, 10, a);

	auto fun = [](const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& noRenameIDLs, const std::vector<std::pair<CIDL, std::wstring>>& renameIDLs)->void
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
		SUCCEEDED(pFileOperation->PerformOperations());

	};
	m_future = std::async(
		std::launch::async, 
		async_action_wrap<decltype(fun), const CIDL&, const CIDL&, const std::vector<CIDL>&, const std::vector<std::pair<CIDL, std::wstring>>&>,
		fun,
		m_destIDL, 
		m_srcIDL, 
		noRenameIDLs, 
		renameIDLs)
	| then(
		[this]() 
		{
			GetWndPtr()->GetDispatcherPtr()->PostInvoke([this]() { OnClose(CloseEvent(GetWndPtr(), NULL, NULL)); });
		});
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
	m_title.set(L"Move");
	m_spButtonDo->GetCommand().Subscribe([this]()->void
	{
		Move();
	});

	m_spButtonDo->GetContent().set(L"Move");
}

void CMoveDlg::Move()
{
	std::vector<CIDL> noRenameIDLs;
	std::vector<std::pair<CIDL, std::wstring>> renameIDLs;

	for (auto iter = m_spItemsSource->cbegin(); iter != m_spItemsSource->cend(); ++ iter) {
		auto& spFile = std::get<std::shared_ptr<CShellFile>>(*iter);
		auto& rename = std::get<RenameInfo>(*iter);
		std::wstring newname = rename.Name + rename.Ext;
		if (spFile->GetPathName() == newname) {
			noRenameIDLs.push_back(spFile->GetAbsoluteIdl());
		} else {
			renameIDLs.emplace_back(spFile->GetAbsoluteIdl(), newname);
		}
	}

	auto fun = [](const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& noRenameIDLs, const std::vector<std::pair<CIDL, std::wstring>>& renameIDLs)->void
	{
		int* p = NULL;
		*p = 1;

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
		SUCCEEDED(pFileOperation->PerformOperations());

	};

	m_future = std::async(
		std::launch::async,
		async_action_wrap<decltype(fun), CIDL, CIDL, std::vector<CIDL>, std::vector<std::pair<CIDL, std::wstring>>>,
		fun,
		m_destIDL,
		m_srcIDL,
		noRenameIDLs,
		renameIDLs)
	| then(
		[this]() 
		{
			GetWndPtr()->GetDispatcherPtr()->PostInvoke([this]() { OnClose(CloseEvent(GetWndPtr(), NULL, NULL)); });
		});

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
	m_title.set(L"Delete");
	//Items Source
	for (auto& childIDL : m_srcChildIDLs) {
		auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(
			shell::DesktopBindToShellFolder(m_srcIDL),
			m_srcIDL,
			childIDL);
		m_spItemsSource->push_back(
			std::make_tuple(spFile));
	}

	//FileGrid
	m_spFilerControl = std::make_unique<CFileOperationGridView<std::shared_ptr<CShellFile>>>(
		this,
		spFilerGridViewProp,
		m_spItemsSource,
		arg<"hdrcol"_s>() = std::make_shared<CRowIndexColumn>(nullptr),
		arg<"namecol"_s>() = std::make_shared<CFilePathNameColumn<std::shared_ptr<CShellFile>>>(nullptr, L"Name"),
		arg<"frzcolcnt"_s>() = 1,
		arg<"columns"_s>() = std::vector<std::shared_ptr<CColumn>>{
			std::make_shared<CFilePathExtColumn<std::shared_ptr<CShellFile>>>(nullptr, L"Ext"),
			std::make_shared<CFileSizeColumn<std::shared_ptr<CShellFile>>>(nullptr, spFilerGridViewProp->FileSizeArgsPtr),
			std::make_shared<CFileLastWriteColumn<std::shared_ptr<CShellFile>>>(nullptr, spFilerGridViewProp->FileTimeArgsPtr) },
		arg<"namerow"_s>() = std::make_shared<CHeaderRow>(nullptr),
		arg<"fltrow"_s>() = std::make_shared<CRow>(nullptr),
		arg<"frzrowcnt"_s>() = 2);

	m_spButtonDo->GetCommand().Subscribe([this]()->void
	{
		Delete();
	});

	m_spButtonDo->GetContent().set(L"Delete");

}

void CDeleteDlg::Delete()
{
	std::vector<CIDL> delIDLs;

	for (auto iter = m_spItemsSource->cbegin(); iter != m_spItemsSource->cend(); ++iter) {
		auto& spFile = std::get<std::shared_ptr<CShellFile>>(*iter);
		delIDLs.push_back(spFile->GetAbsoluteIdl());
	}

	auto fun = [](const std::vector<CIDL>& delIDLs)->void
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

		SUCCEEDED(pFileOperation->PerformOperations());
	};

	m_future = std::async(
		std::launch::async,
		async_action_wrap<decltype(fun), std::vector<CIDL>>,
		fun,
		delIDLs)
	| then(
		[this]() 
		{
			GetWndPtr()->GetDispatcherPtr()->PostInvoke([this]() { OnClose(CloseEvent(GetWndPtr(), NULL, NULL)); });
		});

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
	ExeExtension& exeExtension)
	:CFileOperationDlgBase(pParentControl, spDialogProp, spFilerGridViewProp, CIDL(), std::vector<CIDL>()),
	m_spTextPath(std::make_shared<CTextBox>(this, spTextBoxProp, L"")),
	m_spTextParam(std::make_shared<CTextBox>(this, spTextBoxProp, L"")),
	m_exeExtension(exeExtension),
	m_pBindingPath(std::make_unique<CBinding>(m_exeExtension.Path, m_spTextPath->GetText())),
	m_pBindingParam(std::make_unique<CBinding>(m_exeExtension.Parameter, m_spTextParam->GetText()))

{
	m_title.set(L"Exe");
	//Items Source
	for (auto& file : files) {
		m_spItemsSource->push_back(std::make_tuple(file));
	}

	//FileGrid
	m_spFilerControl = std::make_unique<CFileOperationGridView<std::shared_ptr<CShellFile>>>(
		this,
		spFilerGridViewProp,
		m_spItemsSource,
		arg<"hdrcol"_s>() = std::make_shared<CRowIndexColumn>(nullptr),
		arg<"namecol"_s>() = std::make_shared<CFilePathNameColumn<std::shared_ptr<CShellFile>>>(nullptr, L"Name"),
		arg<"frzcolcnt"_s>() = 1,
		arg<"columns"_s>() = std::vector<std::shared_ptr<CColumn>>{
			std::make_shared<CFilePathExtColumn<std::shared_ptr<CShellFile>>>(nullptr, L"Ext"),
			std::make_shared<CFileSizeColumn<std::shared_ptr<CShellFile>>>(nullptr, spFilerGridViewProp->FileSizeArgsPtr),
			std::make_shared<CFileLastWriteColumn<std::shared_ptr<CShellFile>>>(nullptr, spFilerGridViewProp->FileTimeArgsPtr) },
		arg<"namerow"_s>() = std::make_shared<CHeaderRow>(nullptr),
		arg<"fltrow"_s>() = std::make_shared<CRow>(nullptr),
		arg<"frzrowcnt"_s>() = 2);

	//TextBox
	m_spTextPath->SetHasBorder(true);
	m_spTextParam->SetHasBorder(true);

	m_spButtonDo->GetCommand().Subscribe([this]()->void
	{
		Execute();
	});

	m_spButtonDo->GetContent().set(L"Exe");

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
	SetFocusedControlPtr(m_spButtonDo);
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