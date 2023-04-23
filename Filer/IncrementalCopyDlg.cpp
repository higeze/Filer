#include "IncrementalCopyDlg.h"
#include "D2DWWindow.h"
#include "Dispatcher.h"
#include "ProgressBar.h"
#include "Button.h"
#include "ButtonProperty.h"
#include "CheckableFileGrid.h"
#include "IDL.h"
#include "ShellFunction.h"
#include "ShellFileFactory.h"
#include "ResourceIDFactory.h"
#include "ThreadPool.h"

CIncrementalCopyDlg::CIncrementalCopyDlg(
	CD2DWControl* pParentControl,
	const std::shared_ptr<DialogProperty>& spDialogProp,
	const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp,
	const CIDL& destIDL, const CIDL& srcIDL, const std::vector<CIDL>& srcChildIDLs)
	:CD2DWDialog(pParentControl, spDialogProp), 
	m_destIDL(destIDL), m_srcIDL(srcIDL), m_srcChildIDLs(srcChildIDLs),
	m_spFileGrid(std::make_shared<CCheckableFileGrid>(this, spFilerGridViewProp)),
	m_spProgressbar(std::make_shared<CProgressBar>(this, std::make_shared<ProgressProperty>())),
	m_spButtonDo(std::make_shared<CButton>(this, std::make_shared<ButtonProperty>())),
	m_spButtonCancel(std::make_shared<CButton>(this, std::make_shared<ButtonProperty>())),
	m_spButtonClose(std::make_shared<CButton>(this, std::make_shared<ButtonProperty>()))
{
	m_title.set(L"Incremental Copy");
	m_spButtonDo->GetCommand().Subscribe([this]()->void
	{
		auto funDo = [idlMap = m_idlMap]()->void
		{
			CComPtr<IFileOperation> pFileOperation = nullptr;
			if (FAILED(pFileOperation.CoCreateInstance(CLSID_FileOperation))) {
				return;
			}

			if (!idlMap.empty()) {
				for (auto& pair : idlMap) {
					std::vector<LPITEMIDLIST> pidls;
					std::transform(std::begin(pair.second), std::end(pair.second), std::back_inserter(pidls), [](const CIDL& x) { return x.ptr(); });

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
		};
		m_doFuture = CThreadPool::GetInstance()->enqueue(
			funDo, 0);

		m_idlMap.clear();
		m_spButtonDo->GetIsEnabled().set(!m_idlMap.empty());
	});
	m_spButtonDo->GetContent().set(L"Copy");

	m_spButtonClose->GetCommand().Subscribe([this]()->void
	{
		GetWndPtr()->GetDispatcherPtr()->PostInvoke([this]() { OnClose(CloseEvent(GetWndPtr(), NULL, NULL)); });
	});
	m_spButtonClose->GetContent().set(L"Close");
	m_spButtonCancel->GetContent().set(L"Cancel");

}

CIncrementalCopyDlg::~CIncrementalCopyDlg() = default;


std::tuple<CRectF, CRectF, CRectF, CRectF, CRectF> CIncrementalCopyDlg::GetRects()
{
	CRectF rc = GetRectInWnd();
	CRectF rcTitle = GetTitleRect();
	rc.top = rcTitle.bottom;
	CRectF rcProgress(rc.left + 5.f, rc.top + 5.f, rc.right - 5.f, rc.top + 30.f);
	CRectF rcGrid(rc.left + 5.f, rcProgress.bottom + 5.f, rc.right - 5.f, rc.bottom - 30.f);
	CRectF rcBtnClose(rc.right - 5.f - 50.f, rc.bottom - 25.f, rc.right - 5.f, rc.bottom - 5.f);
	CRectF rcBtnCancel(rcBtnClose.left - 5.f - 50.f, rc.bottom - 25.f, rcBtnClose.left - 5.f, rc.bottom - 5.f);
	CRectF rcBtnDo(rcBtnCancel.left - 5.f - 50.f, rc.bottom - 25.f, rcBtnCancel.left - 5.f, rc.bottom - 5.f);
	return { rcProgress, rcGrid, rcBtnDo, rcBtnCancel, rcBtnClose };
}


void CIncrementalCopyDlg::OnCreate(const CreateEvt& e)
{
	//Dlg
	CD2DWDialog::OnCreate(e);
	//Size
	auto [rcProgress, rcGrid, rcBtnDo, rcBtnCancel, rcBtnClose] = GetRects();
	m_spProgressbar->OnCreate(CreateEvt(GetWndPtr(), this, rcProgress));
	m_spFileGrid->OnCreate(CreateEvt(GetWndPtr(), this, rcGrid));
	m_spButtonDo->OnCreate(CreateEvt(GetWndPtr(), this, rcBtnDo));
	m_spButtonCancel->OnCreate(CreateEvt(GetWndPtr(), this, rcBtnCancel));
	m_spButtonClose->OnCreate(CreateEvt(GetWndPtr(), this, rcBtnClose));


	m_spButtonDo->GetIsEnabled().set(false);

	m_spButtonCancel->GetIsEnabled().set(false);
	m_spButtonClose->GetIsEnabled().set(true);

	auto fun = [this]()->void
	{
		std::function<void()> readMax = [this]()->void
		{
			GetWndPtr()->GetDispatcherPtr()->PostInvoke([this] { OnIncrementMax(); });
		};
		std::function<void()> readValue = [this]()->void
		{
			GetWndPtr()->GetDispatcherPtr()->PostInvoke([this] { OnIncrementValue(); });
		};
		std::function<void(const CIDL&, const CIDL&)> find = [this](const CIDL& destIDL, const CIDL& srcIDL)->void
		{
			auto iter = m_idlMap.find(destIDL);
			if (iter != m_idlMap.end()) {
				iter->second.push_back(srcIDL);
			} else {
				m_idlMap.insert(std::make_pair(destIDL, std::vector<CIDL>{srcIDL}));
			}
			//This should be Send Message to syncro
			GetWndPtr()->GetDispatcherPtr()->PostInvoke([this, srcIDL] { OnAddItem(srcIDL); });
		};

		GetProgressBarPtr()->SetMin(0);
		GetProgressBarPtr()->SetMax(0);
		GetProgressBarPtr()->SetValue(0);

		auto funCount = [srcIDL = m_srcIDL, srcChildIDLs = m_srcChildIDLs, readMax]()->void
		{
			for (const auto& childIDL : srcChildIDLs) {
				shell::CountFileOne(srcIDL, childIDL, readMax);
			}
		};
		auto countFuture = CThreadPool::GetInstance()->enqueue(
			funCount, 0);

		auto funIncr = [srcParentIDL = m_srcIDL, srcChildIDLs = m_srcChildIDLs, destParentIDL = m_destIDL, readValue, find]()->void
		{
			for (const auto& srcChildIDL : srcChildIDLs) {
				shell::FindIncrementalOne(srcParentIDL, srcChildIDL, destParentIDL, readValue, find);
			}
		};
		auto incrFuture = CThreadPool::GetInstance()->enqueue(
			funIncr, 0);

		countFuture.get();
		incrFuture.get();

		if (!m_idlMap.empty()) {
			m_spButtonDo->GetIsEnabled().set(true);
			SetFocusedControlPtr(m_spButtonDo);
		}
	};
	
	//Start comparison
	m_compFuture = CThreadPool::GetInstance()->enqueue(
		fun, 0);
}

void CIncrementalCopyDlg::OnRect(const RectEvent& e)
{
	CD2DWDialog::OnRect(e);

	auto [rcProgress, rcGrid, rcBtnDo, rcBtnCancel, rcBtnClose] = GetRects();
	m_spProgressbar->OnRect(RectEvent(GetWndPtr(), rcProgress));
	m_spFileGrid->OnRect(RectEvent(GetWndPtr(), rcGrid));
	m_spButtonDo->OnRect(RectEvent(GetWndPtr(), rcBtnDo));
	m_spButtonCancel->OnRect(RectEvent(GetWndPtr(), rcBtnCancel));
	m_spButtonClose->OnRect(RectEvent(GetWndPtr(), rcBtnClose));
}

void CIncrementalCopyDlg::OnAddItem(const CIDL& newIdl )
{
	m_spFileGrid->AddItem(CShellFileFactory::GetInstance()->CreateShellFilePtr(
		shell::DesktopBindToShellFolder(newIdl.CloneParentIDL()),
		newIdl.CloneParentIDL(),
		newIdl.CloneLastID()));
	GetWndPtr()->InvalidateRect(NULL, FALSE);
	//m_periodicTimer.runperiodic([this](){InvalidateRect(NULL, FALSE); }, std::chrono::milliseconds(50));
}

void CIncrementalCopyDlg::OnIncrementMax()
{
	m_spProgressbar->IncrementMax();
	GetWndPtr()->InvalidateRect(NULL, FALSE);
	//m_periodicTimer.runperiodic([this](){InvalidateRect(NULL, FALSE); }, std::chrono::milliseconds(50));
}

void CIncrementalCopyDlg::OnIncrementValue()
{
	m_spProgressbar->IncrementValue();
	GetWndPtr()->InvalidateRect(NULL, FALSE);
	//m_periodicTimer.runperiodic([this](){InvalidateRect(NULL, FALSE); }, std::chrono::milliseconds(50));
}


