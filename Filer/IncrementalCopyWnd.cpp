#include "IncrementalCopyWnd.h"
#include "ProgressBar.h"
#include "CheckableFileGrid.h"
#include "IDL.h"
#include "ThreadPool.h"
#include "ShellFunction.h"
#include "ShellFileFactory.h"
#include "ResourceIDFactory.h"

UINT CIncrementalCopyWnd::WM_ADDITEM = ::RegisterWindowMessage(L"CIncrementalCopyWnd::WM_ADDITEM");

CIncrementalCopyWnd::CIncrementalCopyWnd(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
	const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs)
	:m_destIDL(destIDL), m_srcIDL(srcIDL), m_srcChildIDLs(srcChildIDLs),
	CWnd(), 
	m_pFileGrid(std::make_unique<CCheckableFileGrid>(spFilerGridViewProp)),
	m_pProgressbar(std::make_unique<d2dw::CProgressBar>(this, std::make_shared<ProgressProperty>()))

{
	m_rca
		.lpszClassName(L"CIncrementalCopyWnd")
		.style(CS_VREDRAW | CS_HREDRAW)
		.hCursor(::LoadCursor(NULL, IDC_ARROW))
		.hbrBackground((HBRUSH)GetStockObject(GRAY_BRUSH));

	DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	if (m_isModal)dwStyle |= WS_POPUP;

	m_cwa
		.lpszWindowName(L"IncrementalCopyWnd")
		.lpszClassName(L"CIncrementalCopyWnd")
		.dwStyle(dwStyle);


	AddMsgHandler(WM_CREATE, &CIncrementalCopyWnd::OnCreate, this);
	AddMsgHandler(WM_SIZE, &CIncrementalCopyWnd::OnSize, this);
	AddMsgHandler(WM_CLOSE, &CIncrementalCopyWnd::OnClose, this);
	AddMsgHandler(WM_DESTROY, &CIncrementalCopyWnd::OnDestroy, this);
	AddMsgHandler(WM_PAINT, &CIncrementalCopyWnd::OnPaint, this);
	AddMsgHandler(WM_ADDITEM, &CIncrementalCopyWnd::OnAddItem, this);

	AddCmdIDHandler(CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"Copy"), &CIncrementalCopyWnd::OnCommandCopy, this);
	AddCmdIDHandler(CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"Close"), &CIncrementalCopyWnd::OnCommandClose, this);
}

CIncrementalCopyWnd::~CIncrementalCopyWnd() = default;

LRESULT CIncrementalCopyWnd::OnCommandCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CThreadPool::GetInstance()->enqueue([idlMap = m_idlMap]()->void {
		CComPtr<IFileOperation> pFileOperation = nullptr;
		if (FAILED(pFileOperation.CoCreateInstance(CLSID_FileOperation))) {
			return;
		}

		if (!idlMap.empty()) {
			for (auto& pair : idlMap) {
				std::vector<LPITEMIDLIST> pidls;
				std::transform(std::begin(pair.second), std::end(pair.second), std::back_inserter(pidls), [](const CIDL& x) {return x.ptr(); });

				CComPtr<IShellItem2> pDestShellItem;
				if (FAILED(::SHCreateItemFromIDList(pair.first.ptr(), IID_IShellItem2, reinterpret_cast<LPVOID*>(&pDestShellItem)))) {
					return;
				}
				CComPtr<IShellItemArray> pItemAry = nullptr;
				if (FAILED(SHCreateShellItemArrayFromIDLists(pidls.size(), (LPCITEMIDLIST*)(pidls.data()), &pItemAry))) {
					return;
				}
				if (FAILED(pFileOperation->CopyItems(pItemAry, pDestShellItem))) {
					return;
				}
			}
			SUCCEEDED(pFileOperation->PerformOperations());
		}
	});

	m_idlMap.clear();
	m_buttonCopy.EnableWindow(!m_idlMap.empty());

	return 0;
}


LRESULT CIncrementalCopyWnd::OnCommandClose(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	SendMessage(WM_CLOSE, NULL, NULL);
	return 0;
}

LRESULT CIncrementalCopyWnd::OnCreate(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
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

	d2dw::CRectF rcProgress = d2dw::CRectF(rc.left + 5, rc.top + 5, rc.Width() - 5, rc.top + 30);
	m_pProgressbar->SetRect(rcProgress);

	CRect rcGrid(rc.left + 5, rc.top + 30 + 5, rc.Width() - 5, rc.bottom - 30);
	m_pFileGrid->CreateWindowExArgument()
		.dwStyle(WS_CHILD | WS_VISIBLE | m_pFileGrid->CreateWindowExArgument().dwStyle());

	m_pFileGrid->Create(m_hWnd, rcGrid);

	//OK button
	m_buttonCopy.CreateWindowExArgument()
		.lpszClassName(WC_BUTTON)
		.hMenu((HMENU)CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"Copy"))
		.lpszWindowName(L"Copy")
		.dwStyle(WS_CHILD | WS_VISIBLE | BP_PUSHBUTTON);

	m_buttonCopy.RegisterClassExArgument().lpszClassName(WC_BUTTON);
	m_buttonCopy.Create(m_hWnd, rcBtnOK);
	m_buttonCopy.EnableWindow(FALSE);

	//Cancel button
	m_buttonClose.CreateWindowExArgument()
		.lpszClassName(WC_BUTTON)
		.hMenu((HMENU)CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"Close"))
		.lpszWindowName(L"Close")
		.dwStyle(WS_CHILD | WS_VISIBLE | BP_PUSHBUTTON);

	m_buttonClose.RegisterClassExArgument().lpszClassName(WC_BUTTON);
	m_buttonClose.Create(m_hWnd, rcBtnCancel);
	m_buttonClose.EnableWindow(FALSE);
	
	//Start comparison

	CThreadPool::GetInstance()->enqueue([this]()->void {
		std::function<void(int)> readMax = [this](int count)->void {
			GetProgressBarPtr()->AddMax(count);
			InvalidateRect(NULL, FALSE);
		};
		std::function<void(int, const CIDL&, const CIDL&)> readValue = [this](int count, const CIDL& destIDL, const CIDL& srcIDL)->void {
			GetProgressBarPtr()->AddValue(count);
			if (destIDL && srcIDL) {
				AddItem(destIDL, srcIDL);
			}
			InvalidateRect(NULL, FALSE);
		};

		GetProgressBarPtr()->SetMin(0);
		GetProgressBarPtr()->SetMax(0);
		GetProgressBarPtr()->SetValue(0);

		auto fileCount = CThreadPool::GetInstance()->enqueue([srcIDL = m_srcIDL, srcChildIDLs = m_srcChildIDLs, readMax]()->void{
			for (const auto& childIDL : srcChildIDLs) {
				shell::GetFileCount(srcIDL, childIDL, readMax);
			}
		});

		auto incremental = CThreadPool::GetInstance()->enqueue([srcIDL = m_srcIDL, srcChildIDLs = m_srcChildIDLs, destIDL = m_destIDL, readValue]()->void {
			shell::GetIncrementalIDLs(srcIDL, srcChildIDLs, destIDL, readValue);
		});

		fileCount.get();
		incremental.get();

		m_buttonCopy.EnableWindow(!m_idlMap.empty());
		m_buttonClose.EnableWindow(TRUE);
	});

	return 0;
}


LRESULT CIncrementalCopyWnd::OnClose(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//Modal Window
	if (m_isModal && GetParent()) {
		::EnableWindow(GetParent(), TRUE);
	}
	//Destroy
	DestroyWindow();

	//Focus parent wnd
	::SetFocus(::GetParent(m_hWnd));
	return 0;
}

LRESULT CIncrementalCopyWnd::OnDestroy(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

void CIncrementalCopyWnd::OnFinalMessage(HWND hWnd)
{
	delete this;
}


LRESULT CIncrementalCopyWnd::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CPaintDC dc(m_hWnd);
	m_pDirect->BeginDraw();

	m_pDirect->ClearSolid(BackgroundFill);
	PaintEvent e(this, *m_pDirect);
	m_pProgressbar->OnPaint(e);

	m_pDirect->EndDraw();
	return 0;
}

LRESULT CIncrementalCopyWnd::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CRect rc = GetClientRect();
	m_pDirect->GetHwndRenderTarget()->Resize(D2D1_SIZE_U{ (UINT)rc.Width(), (UINT)rc.Height() });

	m_buttonCopy.MoveWindow(rc.right - 115, rc.bottom - 25, 52, 22, TRUE);
	m_buttonClose.MoveWindow(rc.right - 60, rc.bottom - 25, 52, 22, TRUE);
	d2dw::CRectF rcProgress = d2dw::CRectF(rc.left + 5, rc.top + 5, rc.Width() -5, rc.top + 30);
	m_pProgressbar->SetRect(rcProgress);
	CRect rcGrid(rc.left + 5, rc.top + 30 + 5, rc.Width() - 5, rc.bottom - 30);
	m_pFileGrid->MoveWindow(rcGrid, TRUE);

	return 0;
}

LRESULT CIncrementalCopyWnd::OnAddItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pFileGrid->AddItem(CShellFileFactory::GetInstance()->CreateShellFilePtr(
		shell::DesktopBindToShellFolder(m_newIDL.CloneParentIDL()),
		m_newIDL.CloneParentIDL(),
		m_newIDL.CloneLastID()));
	return 0;
}

void CIncrementalCopyWnd::AddItem(const CIDL& destIDL, const CIDL srcIDL)
{
	auto iter = m_idlMap.find(destIDL);
	if (iter != m_idlMap.end()) {
		iter->second.push_back(srcIDL);
	} else {
		m_idlMap.insert(std::make_pair(destIDL, std::vector<CIDL>{srcIDL}));
	}
	m_newIDL = srcIDL;
	SendMessage(WM_ADDITEM, NULL, NULL);
}

