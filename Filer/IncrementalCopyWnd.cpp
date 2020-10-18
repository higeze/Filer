#include "IncrementalCopyWnd.h"
#include "ProgressBar.h"
#include "Button.h"
#include "CheckableFileGrid.h"
#include "IDL.h"
#include "ThreadPool.h"
#include "ShellFunction.h"
#include "ShellFileFactory.h"
#include "ResourceIDFactory.h"
#include "Dispatcher.h"

CIncrementalCopyWnd::CIncrementalCopyWnd(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
	const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs)
	:CD2DWWindow(), 
	m_destIDL(destIDL), m_srcIDL(srcIDL), m_srcChildIDLs(srcChildIDLs),
	m_spFileGrid(std::make_shared<CCheckableFileGrid>(this, spFilerGridViewProp)),
	m_spProgressbar(std::make_shared<CProgressBar>(this, std::make_shared<ProgressProperty>())),
	m_spButtonDo(std::make_shared<CButton>(this, std::make_shared<ButtonProperty>())),
	m_spButtonCancel(std::make_shared<CButton>(this, std::make_shared<ButtonProperty>())),
	m_spButtonClose(std::make_shared<CButton>(this, std::make_shared<ButtonProperty>()))
{
	m_rca
		.lpszClassName(L"CIncrementalCopyWnd")
		.style(CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS)
		.hCursor(::LoadCursor(NULL, IDC_ARROW))
		.hbrBackground((HBRUSH)GetStockObject(GRAY_BRUSH));

	DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	if (m_isModal)dwStyle |= WS_POPUP;

	m_cwa
		.lpszWindowName(L"Incremental Copy")
		.lpszClassName(L"CIncrementalCopyWnd")
		.dwStyle(dwStyle);

	m_spButtonDo->GetCommand().Subscribe([this]()->void
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
		m_spButtonDo->GetIsEnabled().set(!m_idlMap.empty());
	});
	m_spButtonDo->GetContent().set(L"Copy");

	m_spButtonClose->GetCommand().Subscribe([this]()->void
	{
		SendMessage(WM_CLOSE, NULL, NULL);
	});
	m_spButtonClose->GetContent().set(L"Close");
	m_spButtonCancel->GetContent().set(L"Cancel");

}

CIncrementalCopyWnd::~CIncrementalCopyWnd() = default;


std::tuple<CRectF, CRectF, CRectF, CRectF, CRectF> CIncrementalCopyWnd::GetRects()
{
	CRectF rc = GetRectInWnd();
	CRectF rcProgress(rc.left + 5.f, rc.top + 5.f, rc.right - 5.f, rc.top + 30.f);
	CRectF rcGrid(rc.left + 5.f, rcProgress.bottom + 5.f, rc.right - 5.f, rc.bottom - 30.f);
	CRectF rcBtnClose(rc.right - 5.f - 50.f, rc.bottom - 25.f, rc.right - 5.f, rc.bottom - 5.f);
	CRectF rcBtnCancel(rcBtnClose.left - 5.f - 50.f, rc.bottom - 25.f, rcBtnClose.left - 5.f, rc.bottom - 5.f);
	CRectF rcBtnDo(rcBtnCancel.left - 5.f - 50.f, rc.bottom - 25.f, rcBtnCancel.left - 5.f, rc.bottom - 5.f);
	return { rcProgress, rcGrid, rcBtnDo, rcBtnCancel, rcBtnClose };
}


void CIncrementalCopyWnd::OnCreate(const CreateEvt& e)
{
	//Modal Window
	if (m_isModal && GetParent()) {
		::EnableWindow(GetParent(), FALSE);
	}

	//Size
	auto [rcProgress, rcGrid, rcBtnDo, rcBtnCancel, rcBtnClose] = GetRects();
	m_spProgressbar->OnCreate(CreateEvt(this, rcProgress));
	m_spFileGrid->OnCreate(CreateEvt(this, rcGrid));
	m_spButtonDo->OnCreate(CreateEvt(this, rcBtnDo));
	m_spButtonCancel->OnCreate(CreateEvt(this, rcBtnCancel));
	m_spButtonClose->OnCreate(CreateEvt(this, rcBtnClose));


	m_spButtonDo->GetIsEnabled().set(false);
	m_spButtonCancel->GetIsEnabled().set(false);
	m_spButtonClose->GetIsEnabled().set(true);
	
	//Start comparison
	CThreadPool::GetInstance()->enqueue([this]()->void {
		std::function<void()> readMax = [this]()->void {
			GetDispatcherPtr()->PostInvoke([this] { OnIncrementMax(); });
		};
		std::function<void()> readValue = [this]()->void {
			GetDispatcherPtr()->PostInvoke([this] { OnIncrementValue(); });
		};
		std::function<void(const CIDL&, const CIDL&)> find = [this](const CIDL& destIDL, const CIDL& srcIDL)->void {
			auto iter = m_idlMap.find(destIDL);
			if (iter != m_idlMap.end()) {
				iter->second.push_back(srcIDL);
			} else {
				m_idlMap.insert(std::make_pair(destIDL, std::vector<CIDL>{srcIDL}));
			}
			//This should be Send Message to syncro
			GetDispatcherPtr()->PostInvoke([this, srcIDL] { OnAddItem(srcIDL); });
		};

		GetProgressBarPtr()->SetMin(0);
		GetProgressBarPtr()->SetMax(0);
		GetProgressBarPtr()->SetValue(0);

		auto fileCount = CThreadPool::GetInstance()->enqueue([srcIDL = m_srcIDL, srcChildIDLs = m_srcChildIDLs, readMax]()->void{
			for (const auto& childIDL : srcChildIDLs) {
				shell::CountFileOne(srcIDL, childIDL, readMax);
			}
		});

		auto incremental = CThreadPool::GetInstance()->enqueue([srcParentIDL = m_srcIDL, srcChildIDLs = m_srcChildIDLs, destParentIDL = m_destIDL, readValue, find]()->void {
			for (const auto& srcChildIDL : srcChildIDLs) {
				shell::FindIncrementalOne(srcParentIDL, srcChildIDL, destParentIDL, readValue, find);
			}
		});

		fileCount.get();
		incremental.get();

		if (!m_idlMap.empty()) {
			m_spButtonDo->GetIsEnabled().set(true);
			SetFocusedControlPtr(m_spButtonDo);
		}
	});
}

void CIncrementalCopyWnd::OnClose(const CloseEvent& e)
{
	CD2DWWindow::OnClose(e);

	//Modal Window
	if (m_isModal && GetParent()) {
		::EnableWindow(GetParent(), TRUE);
	}
	//Foreground Owner window
	if (HWND hWnd = GetWindow(m_hWnd, GW_OWNER); (GetWindowLongPtr(GWL_STYLE) & WS_OVERLAPPEDWINDOW) == WS_OVERLAPPEDWINDOW && hWnd != NULL) {
		::SetForegroundWindow(hWnd);
	}
	DestroyWindow();
}

void CIncrementalCopyWnd::OnRect(const RectEvent& e)
{
	CD2DWWindow::OnRect(e);

	auto [rcProgress, rcGrid, rcBtnDo, rcBtnCancel, rcBtnClose] = GetRects();
	m_spProgressbar->OnRect(RectEvent(this, rcProgress));
	m_spFileGrid->OnRect(RectEvent(this, rcGrid));
	m_spButtonDo->OnRect(RectEvent(this, rcBtnDo));
	m_spButtonCancel->OnRect(RectEvent(this, rcBtnCancel));
	m_spButtonClose->OnRect(RectEvent(this, rcBtnClose));
}

void CIncrementalCopyWnd::OnAddItem(const CIDL& newIdl )
{
	m_spFileGrid->AddItem(CShellFileFactory::GetInstance()->CreateShellFilePtr(
		shell::DesktopBindToShellFolder(newIdl.CloneParentIDL()),
		newIdl.CloneParentIDL(),
		newIdl.CloneLastID()));
	InvalidateRect(NULL, FALSE);
	//m_periodicTimer.runperiodic([this](){InvalidateRect(NULL, FALSE); }, std::chrono::milliseconds(50));
}

void CIncrementalCopyWnd::OnIncrementMax()
{
	m_spProgressbar->IncrementMax();
	InvalidateRect(NULL, FALSE);
	//m_periodicTimer.runperiodic([this](){InvalidateRect(NULL, FALSE); }, std::chrono::milliseconds(50));
}

void CIncrementalCopyWnd::OnIncrementValue()
{
	m_spProgressbar->IncrementValue();
	InvalidateRect(NULL, FALSE);
	//m_periodicTimer.runperiodic([this](){InvalidateRect(NULL, FALSE); }, std::chrono::milliseconds(50));
}


