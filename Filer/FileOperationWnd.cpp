#include "FileOperationWnd.h"

//TODOTODO
//RenameWnd

/********************/
/* CCopyMoveWndBase */
/********************/
CCopyMoveWndBase::CCopyMoveWndBase(
	std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
	const std::wstring& buttonText,
	const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs)
	:CFileOperationWndBase(spFilerGridViewProp, buttonText, srcIDL, srcChildIDLs), m_destIDL(destIDL) {}

void CCopyMoveWndBase::InitializeFileGrid()
{
	//Insert rows
	m_pFileGrid->SetNameHeaderRowPtr(std::make_shared<CHeaderRow>(m_pFileGrid.get()));
	m_pFileGrid->SetFilterRowPtr(std::make_shared<CRow>(m_pFileGrid.get()));
	m_pFileGrid->PushRows(
		m_pFileGrid->GetNameHeaderRowPtr(),
		m_pFileGrid->GetFilterRowPtr());

	m_pFileGrid->SetFrozenCount<RowTag>(2);

	//Insert columns
	m_pFileGrid->SetFileNameColumnPtr(std::make_shared<CFilePathNameColumn<std::shared_ptr<CShellFile>, RenameInfo>>(m_pFileGrid.get(), L"Name"));
	m_pFileGrid->PushColumns(
		std::make_shared<CRowIndexColumn>(m_pFileGrid.get()),
		m_pFileGrid->GetFileNameColumnPtr(),
		std::make_shared<CFilePathExtColumn<std::shared_ptr<CShellFile>, RenameInfo>>(m_pFileGrid.get(), L"Ext"),
		std::make_shared<CFilePathRenameColumn<std::shared_ptr<CShellFile>, RenameInfo>>(m_pFileGrid.get(), L"Rename"),
		std::make_shared<CFileReextColumn<std::shared_ptr<CShellFile>, RenameInfo>>(m_pFileGrid.get(), L"Reext"),
		std::make_shared<CFileSizeColumn<std::shared_ptr<CShellFile>, RenameInfo>>(m_pFileGrid.get(), m_pFileGrid->GetFilerGridViewPropPtr()->FileSizeArgsPtr),
		std::make_shared<CFileLastWriteColumn<std::shared_ptr<CShellFile>, RenameInfo>>(m_pFileGrid.get(), m_pFileGrid->GetFilerGridViewPropPtr()->FileTimeArgsPtr));

	m_pFileGrid->SetFrozenCount<ColTag>(1);

	//Insert rows
	for (auto& childIDL : m_srcChildIDLs) {
		auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(
			shell::DesktopBindToShellFolder(m_srcIDL),
			m_srcIDL,
			childIDL);
		m_spItemsSource->notify_push_back(
			std::make_tuple(
				spFile,
				RenameInfo{ spFile->GetPathNameWithoutExt(), spFile->GetPathExt() }));
	}
}


/************/
/* CCopyWnd */
/************/
CCopyWnd::CCopyWnd(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
				   const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs)
	:CCopyMoveWndBase(spFilerGridViewProp, L"Copy", destIDL, srcIDL, srcChildIDLs)
{
	m_rca
		.lpszClassName(L"CCopyWnd");

	m_cwa
		.lpszWindowName(L"Copy")
		.lpszClassName(L"CCopyWnd");
}

LRESULT CCopyWnd::OnCommandDo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::vector<CIDL> noRenameIDLs;
	std::vector<std::pair<CIDL, std::wstring>> renameIDLs;

	for (auto& tup : *m_spItemsSource) {
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

	return 0;
}

/************/
/* CMoveWnd */
/************/
CMoveWnd::CMoveWnd(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
				   const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs)
	:CCopyMoveWndBase(spFilerGridViewProp, L"Move", destIDL, srcIDL, srcChildIDLs)
{
	m_rca
		.lpszClassName(L"CMoveWnd");

	m_cwa
		.lpszWindowName(L"Move")
		.lpszClassName(L"CMoveWnd");
}

LRESULT CMoveWnd::OnCommandDo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::vector<CIDL> noRenameIDLs;
	std::vector<std::pair<CIDL, std::wstring>> renameIDLs;

	for (auto& tup : *m_spItemsSource) {
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

	return 0;
}

/**************/
/* CDeleteWnd */
/**************/
CDeleteWnd::CDeleteWnd(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
				   const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs)
	:CFileOperationWndBase(spFilerGridViewProp, L"Delete", srcIDL, srcChildIDLs)
{
	m_rca
		.lpszClassName(L"CDeleteWnd");

	m_cwa
		.lpszWindowName(L"Delete")
		.lpszClassName(L"CDeleteWnd");
}

void CDeleteWnd::InitializeFileGrid()
{
	//Insert rows
	m_pFileGrid->SetNameHeaderRowPtr(std::make_shared<CHeaderRow>(m_pFileGrid.get()));
	m_pFileGrid->SetFilterRowPtr(std::make_shared<CRow>(m_pFileGrid.get()));
	m_pFileGrid->PushRows(
		m_pFileGrid->GetNameHeaderRowPtr(),
		m_pFileGrid->GetFilterRowPtr());

	m_pFileGrid->SetFrozenCount<RowTag>(2);

	//Insert columns
	m_pFileGrid->SetFileNameColumnPtr(std::make_shared<CFilePathNameColumn<std::shared_ptr<CShellFile>>>(m_pFileGrid.get(), L"Name"));
	m_pFileGrid->PushColumns(
		std::make_shared<CRowIndexColumn>(m_pFileGrid.get()),
		m_pFileGrid->GetFileNameColumnPtr(),
		std::make_shared<CFilePathExtColumn<std::shared_ptr<CShellFile>>>(m_pFileGrid.get(), L"Ext"),
		std::make_shared<CFileSizeColumn<std::shared_ptr<CShellFile>>>(m_pFileGrid.get(), m_pFileGrid->GetFilerGridViewPropPtr()->FileSizeArgsPtr),
		std::make_shared<CFileLastWriteColumn<std::shared_ptr<CShellFile>>>(m_pFileGrid.get(), m_pFileGrid->GetFilerGridViewPropPtr()->FileTimeArgsPtr));

	m_pFileGrid->SetFrozenCount<ColTag>(1);

	//Insert rows
	for (auto& childIDL : m_srcChildIDLs) {
		auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(
			shell::DesktopBindToShellFolder(m_srcIDL),
			m_srcIDL,
			childIDL);
		m_spItemsSource->notify_push_back(
			std::make_tuple(spFile));
	}
}

LRESULT CDeleteWnd::OnCommandDo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::vector<CIDL> delIDLs;

	for (auto& tup : *m_spItemsSource) {
		auto& spFile = std::get<std::shared_ptr<CShellFile>>(tup);
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

	return 0;
}







