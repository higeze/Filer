#include "FileOperationWnd.h"
#include "named_arguments.h"
//TODOTODO
//RenameWnd

/********************/
/* CCopyMoveWndBase */
/********************/
CCopyMoveWndBase::CCopyMoveWndBase(
	const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
	const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs)
	:CFileOperationWndBase(spFilerGridViewProp, srcIDL, srcChildIDLs), m_destIDL(destIDL)
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
	m_spFilerControl = std::make_unique<CFilerBindGridView<std::shared_ptr<CShellFile>, RenameInfo>>(
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

	CThreadPool::GetInstance()->enqueue([](HWND hWnd, CIDL destIDL, CIDL srcIDL, std::vector<CIDL> noRenameIDLs, std::vector<std::pair<CIDL, std::wstring>> renameIDLs)->void {

		CComPtr<IFileOperation> pFileOperation = nullptr;
		if (FAILED(pFileOperation.CoCreateInstance(CLSID_FileOperation))) {
			return;
		}

		CComPtr<IShellItem2> pDestShellItem;
		if (FAILED(::SHCreateItemFromIDList(destIDL.ptr(), IID_IShellItem2, reinterpret_cast<LPVOID*>(&pDestShellItem)))) {
			return;
		}

		if (!noRenameIDLs.empty()) {
			std::vector<LPITEMIDLIST> pidls;
			std::transform(std::begin(noRenameIDLs), std::end(noRenameIDLs), std::back_inserter(pidls), [](const CIDL& x) {return x.ptr(); });

			CComPtr<IShellItemArray> pItemAry = nullptr;
			if (FAILED(SHCreateShellItemArrayFromIDLists(pidls.size(), (LPCITEMIDLIST*)(pidls.data()), &pItemAry))) {
				return;
			}
			if (FAILED(pFileOperation->CopyItems(pItemAry, pDestShellItem))) {
				return;
			}
		}

		if (!renameIDLs.empty()) {
			for (auto& renamePair : renameIDLs) {
				CComPtr<IShellItem2> pSrcShellItem;
				if (FAILED(::SHCreateItemFromIDList(renamePair.first.ptr(), IID_IShellItem2, reinterpret_cast<LPVOID*>(&pSrcShellItem)))) {
					return;
				}
				if (FAILED(pFileOperation->CopyItem(pSrcShellItem, pDestShellItem, renamePair.second.c_str(), nullptr))) {
					return;
				}
			}

		}
		SUCCEEDED(pFileOperation->PerformOperations());
		::SendMessage(hWnd, WM_CLOSE, NULL, NULL);

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

	CThreadPool::GetInstance()->enqueue([](HWND hWnd, CIDL destIDL, CIDL srcIDL, std::vector<CIDL> noRenameIDLs, std::vector<std::pair<CIDL, std::wstring>> renameIDLs)->void {

		CComPtr<IFileOperation> pFileOperation = nullptr;
		if (FAILED(pFileOperation.CoCreateInstance(CLSID_FileOperation))) {
			return;
		}

		CComPtr<IShellItem2> pDestShellItem;
		if (FAILED(::SHCreateItemFromIDList(destIDL.ptr(), IID_IShellItem2, reinterpret_cast<LPVOID*>(&pDestShellItem)))) {
			return;
		}

		if (!noRenameIDLs.empty()) {
			std::vector<LPITEMIDLIST> pidls;
			std::transform(std::begin(noRenameIDLs), std::end(noRenameIDLs), std::back_inserter(pidls), [](const CIDL& x) {return x.ptr(); });

			CComPtr<IShellItemArray> pItemAry = nullptr;
			if (FAILED(SHCreateShellItemArrayFromIDLists(pidls.size(), (LPCITEMIDLIST*)(pidls.data()), &pItemAry))) {
				return;
			}
			if (FAILED(pFileOperation->MoveItems(pItemAry, pDestShellItem))) {
				return;
			}
		}

		if (!renameIDLs.empty()) {
			for (auto& renamePair : renameIDLs) {
				CComPtr<IShellItem2> pSrcShellItem;
				if (FAILED(::SHCreateItemFromIDList(renamePair.first.ptr(), IID_IShellItem2, reinterpret_cast<LPVOID*>(&pSrcShellItem)))) {
					return;
				}
				if (FAILED(pFileOperation->MoveItem(pSrcShellItem, pDestShellItem, renamePair.second.c_str(), nullptr))) {
					return;
				}
			}

		}
		SUCCEEDED(pFileOperation->PerformOperations());
		::SendMessage(hWnd, WM_CLOSE, NULL, NULL);

	}, m_hWnd, m_destIDL, m_srcIDL, noRenameIDLs, renameIDLs);
}

/**************/
/* CDeleteWnd */
/**************/
CDeleteWnd::CDeleteWnd(const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
				   const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs)
	:CFileOperationWndBase(spFilerGridViewProp, srcIDL, srcChildIDLs)
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
	m_spFilerControl = std::make_unique<CFilerBindGridView<std::shared_ptr<CShellFile>>>(
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

	CThreadPool::GetInstance()->enqueue([](HWND hWnd, std::vector<CIDL> delIDLs)->void {

		CComPtr<IFileOperation> pFileOperation = nullptr;
		if (FAILED(pFileOperation.CoCreateInstance(CLSID_FileOperation))) {
			return;
		}

		if (!delIDLs.empty()) {
			std::vector<LPITEMIDLIST> pidls;
			std::transform(std::begin(delIDLs), std::end(delIDLs), std::back_inserter(pidls), [](const CIDL& x) {return x.ptr(); });

			CComPtr<IShellItemArray> pItemAry = nullptr;
			if (FAILED(SHCreateShellItemArrayFromIDLists(pidls.size(), (LPCITEMIDLIST*)(pidls.data()), &pItemAry))) {
				return;
			}
			if (FAILED(pFileOperation->DeleteItems(pItemAry))) {
				return;
			}
		}

		SUCCEEDED(pFileOperation->PerformOperations());
		::SendMessage(hWnd, WM_CLOSE, NULL, NULL);

	}, m_hWnd, delIDLs);
}







