#include "FileOperationWnd.h"
#include "Textbox.h"
#include "TextBoxProperty.h"
#include "named_arguments.h"
#include "ShellFile.h"
#include "ShellFolder.h"
#include "scope_exit.h"
//TODOTODO
//RenameWnd

/********************/
/* CCopyMoveWndBase */
/********************/
CCopyMoveWndBase::CCopyMoveWndBase(
	const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
	const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs)
	:CSimpleFileOperationWndBase(spFilerGridViewProp, srcIDL, srcChildIDLs), m_destIDL(destIDL)
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
/* CCopyWnd */
/************/
CCopyWnd::CCopyWnd(const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
				   const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs)
	:CCopyMoveWndBase(spFilerGridViewProp, destIDL, srcIDL, srcChildIDLs)
{
	m_rca
		.lpszClassName(L"CCopyWnd");

	m_cwa
		.lpszWindowName(L"Copy")
		.lpszClassName(L"CCopyWnd")
		;//.hMenu((HMENU)CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"CCopyWnd"));

	m_spButtonDo->GetCommand().Subscribe([this]()->void
	{
		Copy();
	});

	m_spButtonDo->GetContent().set(L"Copy");
}

void CCopyWnd::Copy()
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

	m_future = std::async(std::launch::async, [](HWND hWnd, CIDL destIDL, CIDL srcIDL, std::vector<CIDL> noRenameIDLs, std::vector<std::pair<CIDL, std::wstring>> renameIDLs)->void
	{
		 auto exit = make_scope_exit([hWnd]() { ::SendMessage(hWnd, WM_CLOSE, NULL, NULL); });
		 CComPtr<IFileOperation> pFileOperation = nullptr;
		 FAILED_RETURN(pFileOperation.CoCreateInstance(CLSID_FileOperation));

		 CComPtr<IShellItem2> pDestShellItem;
		 FAILED_RETURN(::SHCreateItemFromIDList(destIDL.ptr(), IID_IShellItem2, reinterpret_cast<LPVOID*>(&pDestShellItem)));

		if (!noRenameIDLs.empty()) {
			std::vector<LPITEMIDLIST> pidls;
			std::transform(std::begin(noRenameIDLs), std::end(noRenameIDLs), std::back_inserter(pidls), [](const CIDL& x) {return x.ptr(); });

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

	}, m_hWnd, m_destIDL, m_srcIDL, noRenameIDLs, renameIDLs);
}

/************/
/* CMoveWnd */
/************/
CMoveWnd::CMoveWnd(const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
				   const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs)
	:CCopyMoveWndBase(spFilerGridViewProp, destIDL, srcIDL, srcChildIDLs)
{
	m_rca
		.lpszClassName(L"CMoveWnd");

	m_cwa
		.lpszWindowName(L"Move")
		.lpszClassName(L"CMoveWnd");

	m_spButtonDo->GetCommand().Subscribe([this]()->void
	{
		Move();
	});

	m_spButtonDo->GetContent().set(L"Move");
}

void CMoveWnd::Move()
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

	m_future = std::async(std::launch::async, [](HWND hWnd, CIDL destIDL, CIDL srcIDL, std::vector<CIDL> noRenameIDLs, std::vector<std::pair<CIDL, std::wstring>> renameIDLs)->void
	{
		auto exit = make_scope_exit([hWnd]() { ::SendMessage(hWnd, WM_CLOSE, NULL, NULL); });
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
		

	}, m_hWnd, m_destIDL, m_srcIDL, noRenameIDLs, renameIDLs);
}

/**************/
/* CDeleteWnd */
/**************/
CDeleteWnd::CDeleteWnd(const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
				   const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs)
	:CSimpleFileOperationWndBase(spFilerGridViewProp, srcIDL, srcChildIDLs)
{
	m_rca
		.lpszClassName(L"CDeleteWnd");

	m_cwa
		.lpszWindowName(L"Delete")
		.lpszClassName(L"CDeleteWnd");

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

void CDeleteWnd::Delete()
{
	std::vector<CIDL> delIDLs;

	for (auto iter = m_spItemsSource->cbegin(); iter != m_spItemsSource->cend(); ++iter) {
		auto& spFile = std::get<std::shared_ptr<CShellFile>>(*iter);
		delIDLs.push_back(spFile->GetAbsoluteIdl());
	}

	m_future = std::async(std::launch::async, [](HWND hWnd, std::vector<CIDL> delIDLs)->void
	{
		auto exit = make_scope_exit([hWnd]() { ::SendMessage(hWnd, WM_CLOSE, NULL, NULL); });

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
	}, m_hWnd, delIDLs);
}

/********************/
/* CExeExtensionWnd */
/********************/

CExeExtensionWnd::CExeExtensionWnd(
		const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
		const std::shared_ptr<TextBoxProperty>& spTextBoxProp,
		const std::shared_ptr<CShellFolder>& folder,
		const std::vector<std::shared_ptr<CShellFile>>& files,
		ExeExtension& exeExtension)
	:CFileOperationWndBase(spFilerGridViewProp, CIDL(), std::vector<CIDL>()),
	m_spTextPath(std::make_shared<CTextBox>(this, spTextBoxProp, L"")),
	m_spTextParam(std::make_shared<CTextBox>(this, spTextBoxProp, L"")),
	m_exeExtension(exeExtension),
	m_pBindingPath(std::make_unique<CBinding>(m_exeExtension.Path, m_spTextPath->GetText())),
	m_pBindingParam(std::make_unique<CBinding>(m_exeExtension.Parameter, m_spTextParam->GetText()))

{
	m_rca
		.lpszClassName(L"CExeExtensionWnd");

	m_cwa
		.lpszWindowName(L"Execute")
		.lpszClassName(L"CExeExtensionWnd");

	//Items Source
	for (auto& file : files) {
		m_spItemsSource->push_back(
			std::make_tuple(file));
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

void CExeExtensionWnd::Execute()
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
	::SendMessage(m_hWnd, WM_CLOSE, NULL, NULL);

} catch (...) {
	throw;
}
}

std::tuple<CRectF, CRectF, CRectF, CRectF, CRectF> CExeExtensionWnd::GetRects()
{
	CRectF rc = GetRectInWnd();
	CRectF rcBtnCancel(rc.right - 5.f - 50.f, rc.bottom - 25.f, rc.right - 5.f, rc.bottom - 5.f);
	CRectF rcBtnDo(rcBtnCancel.left - 5.f - 50.f, rc.bottom - 25.f, rcBtnCancel.left - 5.f, rc.bottom - 5.f);
	CRectF rcTextParam(rc.left + 5.f, rc.bottom - 95.f, rc.right - 5.f, rc.bottom - 30.f);
	CRectF rcTextPath(rc.left + 5.f, rc.bottom - 120, rc.right - 5.f, rc.bottom - 100.f);
	CRectF rcGrid(rc.left + 5.f, rc.top + 5.f, rc.Width() - 5.f, rc.bottom -  125.f);

	return { rcGrid, rcTextPath, rcTextParam, rcBtnDo, rcBtnCancel };
}


void CExeExtensionWnd::OnCreate(const CreateEvt& e)
{
	//Modal Window
	if (m_isModal && GetParent()) {
		::EnableWindow(GetParent(), FALSE);
	}

	//Size
	auto [rcGrid, rcTextPath, rcTextParam, rcBtnDo, rcBtnCancel] = GetRects();
		
	//Create FilerControl
	m_spFilerControl->OnCreate(CreateEvt(this, this, rcGrid));

	//Textbox
	m_spTextPath->OnCreate(CreateEvt(this, this, rcTextPath));
	m_spTextParam->OnCreate(CreateEvt(this, this, rcTextParam));

	//OK button
	m_spButtonDo->OnCreate(CreateEvt(this, this, rcBtnDo));

	//Cancel button
	m_spButtonCancel->OnCreate(CreateEvt(this, this, rcBtnCancel));

	//Focus
	SetFocusedControlPtr(m_spButtonDo);
}

void CExeExtensionWnd::OnRect(const RectEvent& e)
{
	CD2DWWindow::OnRect(e);

	auto [rcGrid, rcTextPath, rcTextParam, rcBtnDo, rcBtnCancel] = GetRects();		
	m_spFilerControl->OnRect(RectEvent(this, rcGrid));
	m_spTextPath->OnRect(RectEvent(this, rcTextPath));
	m_spTextParam->OnRect(RectEvent(this, rcTextParam));
	m_spButtonDo->OnRect(RectEvent(this, rcBtnDo));
	m_spButtonCancel->OnRect(RectEvent(this, rcBtnCancel));
}