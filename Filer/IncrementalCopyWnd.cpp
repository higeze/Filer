//#include "IncrementalCopyWnd.h"
//#include "ProgressBar.h"
//#include "CheckableFileGrid.h"
//#include "IDL.h"
//#include "ThreadPool.h"
//#include "ShellFunction.h"
//#include "ShellFileFactory.h"
//#include "ResourceIDFactory.h"
//
//UINT CIncrementalCopyWnd::WM_INCREMENTMAX = ::RegisterWindowMessage(L"CIncrementalCopyWnd::WM_INCREMENTMAX");
//UINT CIncrementalCopyWnd::WM_INCREMENTVALUE = ::RegisterWindowMessage(L"CIncrementalCopyWnd::WM_INCREMENTVALUE");
//UINT CIncrementalCopyWnd::WM_ADDITEM = ::RegisterWindowMessage(L"CIncrementalCopyWnd::WM_ADDITEM");
//
//CIncrementalCopyWnd::CIncrementalCopyWnd(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
//	const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs)
//	:m_destIDL(destIDL), m_srcIDL(srcIDL), m_srcChildIDLs(srcChildIDLs),
//	CWnd(), 
//	m_pFileGrid(std::make_unique<CCheckableFileGrid>(spFilerGridViewProp)),
//	m_pProgressbar(std::make_unique<CProgressBar>(this, std::make_shared<ProgressProperty>()))
//
//{
//	m_rca
//		.lpszClassName(L"CIncrementalCopyWnd")
//		.style(CS_VREDRAW | CS_HREDRAW)
//		.hCursor(::LoadCursor(NULL, IDC_ARROW))
//		.hbrBackground((HBRUSH)GetStockObject(GRAY_BRUSH));
//
//	DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
//	if (m_isModal)dwStyle |= WS_POPUP;
//
//	m_cwa
//		.lpszWindowName(L"Incremental Copy")
//		.lpszClassName(L"CIncrementalCopyWnd")
//		.dwStyle(dwStyle);
//
//
//	AddMsgHandler(WM_CREATE, &CIncrementalCopyWnd::OnCreate, this);
//	AddMsgHandler(WM_SIZE, &CIncrementalCopyWnd::OnSize, this);
//	AddMsgHandler(WM_CLOSE, &CIncrementalCopyWnd::OnClose, this);
//	AddMsgHandler(WM_DESTROY, &CIncrementalCopyWnd::OnDestroy, this);
//	AddMsgHandler(WM_PAINT, &CIncrementalCopyWnd::OnPaint, this);
//	AddMsgHandler(WM_INCREMENTMAX, &CIncrementalCopyWnd::OnIncrementMax, this);
//	AddMsgHandler(WM_INCREMENTVALUE, &CIncrementalCopyWnd::OnIncrementValue, this);
//	AddMsgHandler(WM_ADDITEM, &CIncrementalCopyWnd::OnAddItem, this);
//
//	AddCmdIDHandler(CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"Copy"), &CIncrementalCopyWnd::OnCommandCopy, this);
//	AddCmdIDHandler(CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"Close"), &CIncrementalCopyWnd::OnCommandClose, this);
//}
//
//CIncrementalCopyWnd::~CIncrementalCopyWnd() = default;
//
//LRESULT CIncrementalCopyWnd::OnCommandCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
//{
//	CThreadPool::GetInstance()->enqueue([idlMap = m_idlMap]()->void {
//		CComPtr<IFileOperation> pFileOperation = nullptr;
//		if (FAILED(pFileOperation.CoCreateInstance(CLSID_FileOperation))) {
//			return;
//		}
//
//		if (!idlMap.empty()) {
//			for (auto& pair : idlMap) {
//				std::vector<LPITEMIDLIST> pidls;
//				std::transform(std::begin(pair.second), std::end(pair.second), std::back_inserter(pidls), [](const CIDL& x) {return x.ptr(); });
//
//				CComPtr<IShellItem2> pDestShellItem;
//				if (FAILED(::SHCreateItemFromIDList(pair.first.ptr(), IID_IShellItem2, reinterpret_cast<LPVOID*>(&pDestShellItem)))) {
//					return;
//				}
//				CComPtr<IShellItemArray> pItemAry = nullptr;
//				if (FAILED(SHCreateShellItemArrayFromIDLists(pidls.size(), (LPCITEMIDLIST*)(pidls.data()), &pItemAry))) {
//					return;
//				}
//				if (FAILED(pFileOperation->CopyItems(pItemAry, pDestShellItem))) {
//					return;
//				}
//			}
//			SUCCEEDED(pFileOperation->PerformOperations());
//		}
//	});
//
//	m_idlMap.clear();
//	m_buttonDo.EnableWindow(!m_idlMap.empty());
//
//	return 0;
//}
//
//
//LRESULT CIncrementalCopyWnd::OnCommandClose(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
//{
//	SendMessage(WM_CLOSE, NULL, NULL);
//	return 0;
//}
//
//LRESULT CIncrementalCopyWnd::OnCreate(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//{
//	//Direct2DWrite
//	m_pDirect = std::make_shared<CDirect2DWrite>(m_hWnd);
//
//	//Modal Window
//	if (m_isModal && GetParent()) {
//		::EnableWindow(GetParent(), FALSE);
//	}
//
//	//Size
//	CRect rc = GetClientRect();
//	CRect rcBtnOK, rcBtnCancel;
//	rcBtnOK.SetRect(rc.right - 170, rc.bottom - 25, rc.right - 170 + 52, rc.bottom - 25 + 22);
//	rcBtnCancel.SetRect(rc.right - 115, rc.bottom - 25, rc.right - 115 + 52, rc.bottom - 25 + 22);
//
//	CRectF rcProgress = m_pDirect->Pixels2Dips(CRect(rc.left + 5, rc.top + 5, rc.Width() - 5, rc.top + 30));
//	m_pProgressbar->SetRect(rcProgress);
//
//	CRect rcGrid(rc.left + 5, rc.top + 30 + 5, rc.Width() - 5, rc.bottom - 30);
//	m_pFileGrid->CreateWindowExArgument()
//		.dwStyle(WS_CHILD | WS_VISIBLE | m_pFileGrid->CreateWindowExArgument().dwStyle());
//
//	m_pFileGrid->Create(m_hWnd, rcGrid);
//
//	//OK button
//	m_buttonDo.CreateWindowExArgument()
//		.lpszClassName(WC_BUTTON)
//		.hMenu((HMENU)CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"Copy"))
//		.lpszWindowName(L"Copy")
//		.dwStyle(WS_CHILD | WS_VISIBLE | BP_PUSHBUTTON | WS_TABSTOP);
//
//	m_buttonDo.RegisterClassExArgument().lpszClassName(WC_BUTTON);
//	m_buttonDo.Create(m_hWnd, rcBtnOK);
//	m_buttonDo.EnableWindow(FALSE);
//
//	//Cancel button
//	m_buttonClose.CreateWindowExArgument()
//		.lpszClassName(WC_BUTTON)
//		.hMenu((HMENU)CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"Close"))
//		.lpszWindowName(L"Close")
//		.dwStyle(WS_CHILD | WS_VISIBLE | BP_PUSHBUTTON | WS_TABSTOP);
//
//	m_buttonClose.RegisterClassExArgument().lpszClassName(WC_BUTTON);
//	m_buttonClose.Create(m_hWnd, rcBtnCancel);
//	m_buttonClose.EnableWindow(FALSE);
//	
//	//Start comparison
//
//	CThreadPool::GetInstance()->enqueue([this]()->void {
//		std::function<void()> readMax = [this]()->void {
//			PostMessage(WM_INCREMENTMAX, NULL, NULL);
//		};
//		std::function<void()> readValue = [this]()->void {
//			PostMessage(WM_INCREMENTVALUE, NULL, NULL);
//		};
//		std::function<void(const CIDL&, const CIDL&)> find = [this](const CIDL& destIDL, const CIDL& srcIDL)->void {
//			auto iter = m_idlMap.find(destIDL);
//			if (iter != m_idlMap.end()) {
//				iter->second.push_back(srcIDL);
//			} else {
//				m_idlMap.insert(std::make_pair(destIDL, std::vector<CIDL>{srcIDL}));
//			}
//			//This should be Send Message to syncro
//			m_newIDL = srcIDL;
//			SendMessage(WM_ADDITEM, NULL, NULL);
//		};
//
//		GetProgressBarPtr()->SetMin(0);
//		GetProgressBarPtr()->SetMax(0);
//		GetProgressBarPtr()->SetValue(0);
//
//		auto fileCount = CThreadPool::GetInstance()->enqueue([srcIDL = m_srcIDL, srcChildIDLs = m_srcChildIDLs, readMax]()->void{
//			for (const auto& childIDL : srcChildIDLs) {
//				shell::CountFileOne(srcIDL, childIDL, readMax);
//			}
//		});
//
//		auto incremental = CThreadPool::GetInstance()->enqueue([srcParentIDL = m_srcIDL, srcChildIDLs = m_srcChildIDLs, destParentIDL = m_destIDL, readValue, find]()->void {
//			for (const auto& srcChildIDL : srcChildIDLs) {
//				shell::FindIncrementalOne(srcParentIDL, srcChildIDL, destParentIDL, readValue, find);
//			}
//		});
//
//		fileCount.get();
//		incremental.get();
//
//		if (!m_idlMap.empty()) {
//			m_buttonDo.EnableWindow(TRUE);
//			m_buttonDo.SetFocus();
//		}
//		m_buttonClose.EnableWindow(TRUE);
//	});
//
//
//
//	return 0;
//}
//
//
//LRESULT CIncrementalCopyWnd::OnClose(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//{
//	//Modal Window
//	if (m_isModal && GetParent()) {
//		::EnableWindow(GetParent(), TRUE);
//	}
//	//Foreground Owner window
//	if (HWND hWnd = GetWindow(m_hWnd, GW_OWNER); (GetWindowLongPtr(GWL_STYLE) & WS_OVERLAPPEDWINDOW) == WS_OVERLAPPEDWINDOW && hWnd != NULL) {
//		::SetForegroundWindow(hWnd);
//	}
//	//Destroy
//	m_pFileGrid->DestroyWindow();
//	DestroyWindow();
//
//	return 0;
//}
//
//LRESULT CIncrementalCopyWnd::OnDestroy(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//{
//	return 0;
//}
//
//LRESULT CIncrementalCopyWnd::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//{
//	CPaintDC dc(m_hWnd);
//	m_pDirect->BeginDraw();
//
//	m_pDirect->ClearSolid(BackgroundFill);
//	m_pProgressbar->OnPaint(PaintEvent(this));
//
//	m_pDirect->EndDraw();
//	return 0;
//}
//
//LRESULT CIncrementalCopyWnd::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//{
//	CRect rc = GetClientRect();
//	m_pDirect->GetHwndRenderTarget()->Resize(D2D1_SIZE_U{ (UINT)rc.Width(), (UINT)rc.Height() });
//
//	m_buttonDo.MoveWindow(rc.right - 115, rc.bottom - 25, 52, 22, TRUE);
//	m_buttonClose.MoveWindow(rc.right - 60, rc.bottom - 25, 52, 22, TRUE);
//	CRectF rcProgress = m_pDirect->Pixels2Dips(CRect(rc.left + 5, rc.top + 5, rc.Width() -5, rc.top + 30));
//	m_pProgressbar->SetRect(rcProgress);
//	CRect rcGrid(rc.left + 5, rc.top + 30 + 5, rc.Width() - 5, rc.bottom - 30);
//	m_pFileGrid->GetWndPtr()->MoveWindow(rcGrid, TRUE);
//
//	return 0;
//}
//
//LRESULT CIncrementalCopyWnd::OnAddItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//{
//	m_pFileGrid->AddItem(CShellFileFactory::GetInstance()->CreateShellFilePtr(
//		shell::DesktopBindToShellFolder(m_newIDL.CloneParentIDL()),
//		m_newIDL.CloneParentIDL(),
//		m_newIDL.CloneLastID()));
//	InvalidateRect(NULL, FALSE);
//	//m_periodicTimer.runperiodic([this](){InvalidateRect(NULL, FALSE); }, std::chrono::milliseconds(50));
//	return 0;
//}
//
//LRESULT CIncrementalCopyWnd::OnIncrementMax(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//{
//	m_pProgressbar->IncrementMax();
//	InvalidateRect(NULL, FALSE);
//	//m_periodicTimer.runperiodic([this](){InvalidateRect(NULL, FALSE); }, std::chrono::milliseconds(50));
//	return 0;
//}
//
//LRESULT CIncrementalCopyWnd::OnIncrementValue(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//{
//	m_pProgressbar->IncrementValue();
//	InvalidateRect(NULL, FALSE);
//	//m_periodicTimer.runperiodic([this](){InvalidateRect(NULL, FALSE); }, std::chrono::milliseconds(50));
//	return 0;
//}
//
//
