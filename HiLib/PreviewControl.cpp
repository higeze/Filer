#include "PreviewControl.h"
#include "PreviewControlProperty.h"
#include <propkey.h>

#include "D2DWWindow.h"
#include "Debug.h"
#include "ResourceIDFactory.h"
#include <functional>
#include <ranges>
#include "HiLibResource.h"
#include "PDFViewport.h"
#include "PdfViewStateMachine.h"
#include "TextBoxDialog.h"
#include "DialogProperty.h"
#include "TextBlock.h"
#include "TextBox.h"
#include "Button.h"
#include "ButtonProperty.h"
#include "strconv.h"
#include "Dispatcher.h"
#include "D2DImage.h"
#include "ImageDrawer.h"
#include "D2DWTypes.h"

#include "PreviewWnd.h"


/**************************/
/* Constructor/Destructor */
/**************************/

CPreviewControl::CPreviewControl(CD2DWControl* pParentControl, const std::shared_ptr<PreviewControlProperty>& pProp)
	:CD2DWControl(pParentControl),
	m_pWnd(std::make_unique<CPreviewWnd>()),
    m_pProp(pProp){}

CPreviewControl::~CPreviewControl() = default;

//const CComPtr<IPreviewHandlerFrame>& CPreviewControl::GetPreviewHandlerFramePtr() const
//{
//	if (!m_optPreviewHandlerFramePtr.has_value()) {
//		CComPtr<IPreviewHandlerFrame> pPreviewHandlerFrame;
//		CPreviewHandlerFrame::CreateInstance<CPreviewHandlerFrame>(&pPreviewHandlerFrame);
//		m_optPreviewHandlerFramePtr.emplace(pPreviewHandlerFrame);
//	}
//	return m_optPreviewHandlerFramePtr.value();
//};

void CPreviewControl::Open(const std::wstring& path)
{
	m_pWnd->Open(path);
	//Close();

	//if (!::PathFileExists(path.c_str())) { return; }

	//m_pFileIsInUse = CFileIsInUseImpl::CreateInstance(GetWndPtr()->m_hWnd, path.c_str(), FUT_DEFAULT, OF_CAP_DEFAULT);
	//GetWndPtr()->AddMsgHandler(CFileIsInUseImpl::WM_FILEINUSE_CLOSEFILE, [this](UINT, LPARAM, WPARAM, BOOL&)->LRESULT
	//{
	//	Close();
	//	return 0;
	//});

	//CComPtr<IShellItem> pItem1;
	//::SHCreateItemFromParsingName(path.c_str(), nullptr, IID_PPV_ARGS(&pItem1));
	//CComQIPtr<IShellItem2> pItem2(pItem1);

	//CComHeapPtr<wchar_t> ext;
	//FAILED_THROW(pItem2->GetString(PKEY_ItemType, &ext));

	//CComHeapPtr<wchar_t> parsingName;
	//FAILED_THROW(pItem2->GetDisplayName(SIGDN::SIGDN_DESKTOPABSOLUTEPARSING, &parsingName));

	//CComPtr<IQueryAssociations> assoc;
	//FAILED_THROW(pItem2->BindToHandler(NULL, BHID_AssociationArray, IID_PPV_ARGS(&assoc)));

	//WCHAR sclsid[48] = { 0 };
	//DWORD size = 48;
	//FAILED_THROW(assoc->GetString(ASSOCF_INIT_DEFAULTTOSTAR, ASSOCSTR_SHELLEXTENSION, L"{8895b1c6-b41f-4c1c-a562-0d564250836f}", sclsid, &size));

	//CLSID clsid;
	//SHCLSIDFromString(sclsid, &clsid);
	//FAILED_THROW(m_pPreviewHandler.CoCreateInstance(clsid, NULL, CLSCTX_LOCAL_SERVER));

	//CComPtr<IInitializeWithItem> iitem;
	//if (SUCCEEDED(m_pPreviewHandler->QueryInterface(&iitem)))
	//{
	//	FAILED_THROW(iitem->Initialize(pItem2, STGM_READ));
	//}
	//else
	//{
	//	CComPtr<IInitializeWithFile> ifile;
	//	if (SUCCEEDED(m_pPreviewHandler->QueryInterface(&ifile)))
	//	{
	//		FAILED_THROW(ifile->Initialize(parsingName, STGM_READ));
	//	}
	//	else
	//	{
	//		CComPtr<IInitializeWithStream> istream;
	//		FAILED_THROW(m_pPreviewHandler->QueryInterface(&istream));

	//		CComPtr<IStream> stream;
	//		FAILED_THROW(SHCreateStreamOnFile(parsingName, STGM_READ, &stream));
	//		FAILED_THROW(istream->Initialize(stream, STGM_READ));
	//	}
	//}

	//CComPtr<IObjectWithSite> site;
	//if (SUCCEEDED(m_pPreviewHandler->QueryInterface(&site)))
	//{
	//	site->SetSite(GetPreviewHandlerFramePtr());
	//}

	//CRectU rcu(CRectF2CRectU(GetRectInWnd()));
	//CRect rc(rcu.left, rcu.top, rcu.right, rcu.bottom);

	//FAILED_THROW(m_pPreviewHandler->SetWindow(GetWndPtr()->m_hWnd, &rc));
	//FAILED_THROW(m_pPreviewHandler->SetRect(&rc));
	//FAILED_THROW(m_pPreviewHandler->DoPreview());
}

void CPreviewControl::Open()
{
	m_pWnd->Open();
	//std::wstring path;
	//OPENFILENAME ofn = { 0 };
	//ofn.lStructSize = sizeof(OPENFILENAME);
	//ofn.hwndOwner = NULL;// GetWndPtr()->m_hWnd;
	////ofn.lpstrFilter = L"Text file(*.txt)\0*.txt\0\0";
	//ofn.lpstrFile = ::GetBuffer(path, MAX_PATH);
	//ofn.nMaxFile = MAX_PATH;
	//ofn.lpstrTitle = L"Open";
	//ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	////ofn.lpstrDefExt = L"txt";

	//if (!GetOpenFileName(&ofn)) {
	//	DWORD errCode = CommDlgExtendedError();
	//	if (errCode) {
	//		throw std::exception(FILE_LINE_FUNC);
	//	}
	//} else {
	//	::ReleaseBuffer(path);
	//	Open(path);
	//}
}

void CPreviewControl::Close()
{
	m_pWnd->Close();
	m_pWnd->ShowWindow(SW_HIDE);
	//if (m_pFileIsInUse) {
	//	GetWndPtr()->RemoveMsgHandler(CFileIsInUseImpl::WM_FILEINUSE_CLOSEFILE);
	//	m_pFileIsInUse.Release();
	//}

	//if (m_pPreviewHandler) {
	//	FAILED_THROW(m_pPreviewHandler->Unload());
	//	m_pPreviewHandler.Release();
	//}
}

/****************/
/* EventHandler */
/****************/

void CPreviewControl::OnCreate(const CreateEvt& e)
{
	CD2DWControl::OnCreate(e);
	m_pWnd->CreateWindowExArgument().dwStyle(m_pWnd->CreateWindowExArgument().dwStyle() | WS_CHILD);
	m_pWnd->Create(GetWndPtr()->m_hWnd, e.Rect);
	m_pWnd->ShowWindow(SW_HIDE);
	//Path.Subscribe([](const NotifyStringChangedEventArgs& arg) {


	//};
}

void CPreviewControl::OnRect(const RectEvent& e)
{
	//if (e.Rect != GetRectInWnd()) {
		CD2DWControl::OnRect(e);
		CRect rc(e.Rect.left, e.Rect.top, e.Rect.right, e.Rect.bottom);
		m_pWnd->MoveWindow(rc, TRUE);
		//if (m_pPreviewHandler) {
		//	CRectU rcu(CRectF2CRectU(e.Rect));
		//	CRect rc(rcu.left, rcu.top, rcu.right, rcu.bottom);
		//	FAILED_THROW(m_pPreviewHandler->SetRect(&rc));
		//}
	//}
}

void CPreviewControl::OnClose(const CloseEvent& e)
{
	CD2DWControl::OnClose(e);
}

void CPreviewControl::OnDestroy(const DestroyEvent& e)
{
	CD2DWControl::OnDestroy(e);
}

void CPreviewControl::OnSetFocus(const SetFocusEvent& e)
{
	CD2DWControl::OnSetFocus(e);
	//if (m_pPreviewHandler) {
	//	FAILED_THROW(m_pPreviewHandler->SetFocus());
	//}
}

void CPreviewControl::OnEnable(const EnableEvent& e)
{
	if (e.Enable) {
		m_pWnd->ShowWindow(SW_SHOW);
	} else {
		Close();
	}
}

void CPreviewControl::OnWndKillFocus(const KillFocusEvent& e)
{
}

void CPreviewControl::OnKeyDown(const KeyDownEvent& e)
{
	bool shift = (::GetKeyState(VK_SHIFT) & 0x80) != 0;
	bool ctrl = (::GetKeyState(VK_CONTROL) & 0x80) != 0;

	switch (e.Char) {
		case 'O':
			if (ctrl) {
				Open();
			}
			break;
		default:
			CD2DWControl::OnKeyDown(e);
			break;
	}
}
