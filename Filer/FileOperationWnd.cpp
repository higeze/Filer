#include "FileOperationWnd.h"
#include "IDL.h"
#include "ThreadPool.h"
#include "ShellFunction.h"
#include "ShellFileFactory.h"
#include "ResourceIDFactory.h"
#include "ShellFile.h"
#include "ThreadPool.h"

//TODOTODO
//Rename
//srcIDL, srcChildIDLs

//Replace
//RenameFileGrid

//Delete
//srcIDL, srcChildIDLs

CFileOperationWndBase::CFileOperationWndBase(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
											 const std::wstring& buttonText,
											 const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs)
	:CWnd(), 
	m_buttonText(buttonText), m_destIDL(destIDL), m_srcIDL(srcIDL), m_srcChildIDLs(srcChildIDLs),
	m_spItemsSource(std::make_shared< observable_vector<std::tuple<std::shared_ptr<CShellFile>, RenameInfo>>>())
{
	m_pFileGrid = std::make_unique<CFilerBindGridView<std::shared_ptr<CShellFile>, RenameInfo>>(
		spFilerGridViewProp,
		m_spItemsSource);
	m_rca
		.lpszClassName(L"CFileOperationWnd")
		.style(CS_VREDRAW | CS_HREDRAW)
		.hCursor(::LoadCursor(NULL, IDC_ARROW))
		.hbrBackground((HBRUSH)GetStockObject(GRAY_BRUSH));

	DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	if (m_isModal)dwStyle |= WS_POPUP;

	m_cwa
		.lpszWindowName(L"FileOperation")
		.lpszClassName(L"CFileOperationWnd")
		.dwStyle(dwStyle);


	AddMsgHandler(WM_CREATE, &CFileOperationWndBase::OnCreate, this);
	AddMsgHandler(WM_SIZE, &CFileOperationWndBase::OnSize, this);
	AddMsgHandler(WM_CLOSE, &CFileOperationWndBase::OnClose, this);
	AddMsgHandler(WM_DESTROY, &CFileOperationWndBase::OnDestroy, this);
	AddMsgHandler(WM_PAINT, &CFileOperationWndBase::OnPaint, this);

	AddCmdIDHandler(CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"Do"), &CFileOperationWndBase::OnCommandDo, this);
	AddCmdIDHandler(CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"Cancel"), &CFileOperationWndBase::OnCommandCancel, this);
}

CFileOperationWndBase::~CFileOperationWndBase() = default;

LRESULT CFileOperationWndBase::OnCommandCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	SendMessage(WM_CLOSE, NULL, NULL);
	return 0;
}

LRESULT CFileOperationWndBase::OnCreate(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//Direct2DWrite
	m_pDirect = std::make_shared<d2dw::CDirect2DWrite>(m_hWnd);

	//Modal Window
	if (m_isModal && GetParent()) {
		::EnableWindow(GetParent(), FALSE);
	}

	//Size
	CRect rc = GetClientRect();
	CRect rcBtnOK, rcBtnCancel;
	rcBtnOK.SetRect(rc.right - 170, rc.bottom - 25, rc.right - 170 + 52, rc.bottom - 25 + 22);
	rcBtnCancel.SetRect(rc.right - 115, rc.bottom - 25, rc.right - 115 + 52, rc.bottom - 25 + 22);

	CRect rcGrid(rc.left + 5, rc.top + 5, rc.Width() - 5, rc.bottom - 30);
	//FileGrid
	m_pFileGrid->CreateWindowExArgument()
		.dwStyle(WS_CHILD | WS_VISIBLE | WS_TABSTOP | m_pFileGrid->CreateWindowExArgument().dwStyle());

	//OK button
	m_buttonDo.CreateWindowExArgument()
		.lpszClassName(WC_BUTTON)
		.hMenu((HMENU)CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"Do"))
		.lpszWindowName(m_buttonText.c_str())
		.dwStyle(WS_CHILD | WS_VISIBLE | WS_TABSTOP | BP_PUSHBUTTON);

	m_buttonDo.RegisterClassExArgument().lpszClassName(WC_BUTTON);
	m_buttonDo.AddMsgHandler(WM_KEYDOWN, [this](UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)->LRESULT {
		switch ((UINT)wParam) {
		case VK_RETURN:
			OnCommandDo(0, 0, 0, bHandled);
			break;
		default:
			break;
		}
		return 0;
	});

	//Cancel button
	m_buttonCancel.CreateWindowExArgument()
		.lpszClassName(WC_BUTTON)
		.hMenu((HMENU)CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"Cancel"))
		.lpszWindowName(L"Cancel")
		.dwStyle(WS_CHILD | WS_VISIBLE | WS_TABSTOP);

	m_buttonCancel.RegisterClassExArgument().lpszClassName(WC_BUTTON);

	//Create on order of tabstop
	m_buttonDo.Create(m_hWnd, rcBtnOK);
	m_buttonDo.SubclassWindow(m_buttonDo.m_hWnd);
	m_buttonCancel.Create(m_hWnd, rcBtnCancel);
	m_pFileGrid->Create(m_hWnd, rcGrid);
	
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
	for(auto& childIDL : m_srcChildIDLs){
		auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(
			shell::DesktopBindToShellFolder(m_srcIDL),
			m_srcIDL,
			childIDL);
		m_spItemsSource->notify_push_back(
			std::make_tuple(
				spFile,
				RenameInfo{spFile->GetPathNameWithoutExt(), spFile->GetPathExt()}));
	}

	m_pFileGrid->PostUpdate(Updates::All);
	m_pFileGrid->SubmitUpdate();

	m_buttonDo.SetFocus();
	
	return 0;
}


LRESULT CFileOperationWndBase::OnClose(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//Modal Window
	if (m_isModal && GetParent()) {
		::EnableWindow(GetParent(), TRUE);
	}
	//Foreground Owner window
	if (HWND hWnd = GetWindow(m_hWnd, GW_OWNER); (GetWindowLongPtr(GWL_STYLE) & WS_OVERLAPPEDWINDOW) == WS_OVERLAPPEDWINDOW && hWnd != NULL) {
		::SetForegroundWindow(hWnd);
	}
	//Destroy
	m_pFileGrid->DestroyWindow();
	DestroyWindow();

	return 0;
}

LRESULT CFileOperationWndBase::OnDestroy(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CFileOperationWndBase::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CPaintDC dc(m_hWnd);
	m_pDirect->BeginDraw();

	m_pDirect->ClearSolid(BackgroundFill);

	m_pDirect->EndDraw();
	return 0;
}

LRESULT CFileOperationWndBase::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CRect rc = GetClientRect();
	m_pDirect->GetHwndRenderTarget()->Resize(D2D1_SIZE_U{ (UINT)rc.Width(), (UINT)rc.Height() });

	m_buttonDo.MoveWindow(rc.right - 115, rc.bottom - 25, 52, 22, TRUE);
	m_buttonCancel.MoveWindow(rc.right - 60, rc.bottom - 25, 52, 22, TRUE);
	CRect rcGrid(rc.left + 5, rc.top + 5, rc.Width() - 5, rc.bottom - 30);
	m_pFileGrid->MoveWindow(rcGrid, TRUE);

	return 0;
}

/************/
/* CCopyWnd */
/************/
CCopyWnd::CCopyWnd(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
				   const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs)
	:CFileOperationWndBase(spFilerGridViewProp, L"Copy", destIDL, srcIDL, srcChildIDLs)
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
	:CFileOperationWndBase(spFilerGridViewProp, L"Move", destIDL, srcIDL, srcChildIDLs)
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



