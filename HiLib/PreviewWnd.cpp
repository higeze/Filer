#pragma once
#include "PreviewWnd.h"
#include <propkey.h>


CPreviewWnd::CPreviewWnd() 
{
	m_rca
		.lpszClassName(L"CPreviewWnd")
		.style(CS_VREDRAW | CS_HREDRAW)
		.hCursor(::LoadCursor(NULL, IDC_ARROW))
		.hbrBackground((HBRUSH)GetStockObject(GRAY_BRUSH));

	m_cwa
		.lpszWindowName(L"PreviewWnd")
		.lpszClassName(L"CPreviewWnd")
		.dwStyle(WS_CLIPSIBLINGS | WS_CLIPCHILDREN)
		.dwExStyle(WS_EX_TOOLWINDOW);

	//AddMsgHandler(WM_CREATE, &CSearchWnd::OnCreate, this);
	AddMsgHandler(WM_SIZE, &CPreviewWnd::OnSize, this);
	//AddMsgHandler(WM_CLOSE, &CSearchWnd::OnClose, this);
	//AddMsgHandler(WM_DESTROY, &CSearchWnd::OnDestroy, this);
	//AddMsgHandler(WM_PAINT, &CSearchWnd::OnPaint, this);
	AddMsgHandler(WM_ERASEBKGND, [this](UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)-> LRESULT
		{
			bHandled = TRUE;
			return 1;
		});
}
CPreviewWnd::~CPreviewWnd() = default;

const CComPtr<IPreviewHandlerFrame>& CPreviewWnd::GetPreviewHandlerFramePtr() const
{
	if (!m_optPreviewHandlerFramePtr.has_value()) {
		CComPtr<IPreviewHandlerFrame> pPreviewHandlerFrame;
		CPreviewHandlerFrame::CreateInstance<CPreviewHandlerFrame>(&pPreviewHandlerFrame);
		m_optPreviewHandlerFramePtr.emplace(pPreviewHandlerFrame);
	}
	return m_optPreviewHandlerFramePtr.value();
};

void CPreviewWnd::Open(const std::wstring& path)
{
	if (m_path != path && ::PathFileExists(path.c_str())) {

		Close();

		//m_pFileIsInUse = CFileIsInUseImpl::CreateInstance(m_hWnd, path.c_str(), FUT_DEFAULT, OF_CAP_DEFAULT);
		//AddMsgHandler(CFileIsInUseImpl::WM_FILEINUSE_CLOSEFILE, [this](UINT, LPARAM, WPARAM, BOOL&)->LRESULT {
		//	Close();
		//	return 0;
		//});

		CComPtr<IShellItem> pItem1;
		::SHCreateItemFromParsingName(path.c_str(), nullptr, IID_PPV_ARGS(&pItem1));
		CComQIPtr<IShellItem2> pItem2(pItem1);

		CComHeapPtr<wchar_t> ext;
		FAILED_THROW(pItem2->GetString(PKEY_ItemType, &ext));

		CComHeapPtr<wchar_t> parsingName;
		FAILED_THROW(pItem2->GetDisplayName(SIGDN::SIGDN_DESKTOPABSOLUTEPARSING, &parsingName));

		CComPtr<IQueryAssociations> assoc;
		FAILED_THROW(pItem2->BindToHandler(NULL, BHID_AssociationArray, IID_PPV_ARGS(&assoc)));

		WCHAR sclsid[48] = {0};
		DWORD size = 48;
		FAILED_THROW(assoc->GetString(ASSOCF_INIT_DEFAULTTOSTAR, ASSOCSTR_SHELLEXTENSION, L"{8895b1c6-b41f-4c1c-a562-0d564250836f}", sclsid, &size));

		CLSID clsid;
		SHCLSIDFromString(sclsid, &clsid);
		FAILED_THROW(m_pPreviewHandler.CoCreateInstance(clsid, NULL, CLSCTX_LOCAL_SERVER));

		CComPtr<IInitializeWithItem> iitem;
		if (SUCCEEDED(m_pPreviewHandler->QueryInterface(&iitem))) {
			FAILED_THROW(iitem->Initialize(pItem2, STGM_READ));
		} else {
			CComPtr<IInitializeWithFile> ifile;
			if (SUCCEEDED(m_pPreviewHandler->QueryInterface(&ifile))) {
				FAILED_THROW(ifile->Initialize(parsingName, STGM_READ));
			} else {
				CComPtr<IInitializeWithStream> istream;
				FAILED_THROW(m_pPreviewHandler->QueryInterface(&istream));

				CComPtr<IStream> stream;
				FAILED_THROW(SHCreateStreamOnFile(parsingName, STGM_READ, &stream));
				FAILED_THROW(istream->Initialize(stream, STGM_READ));
			}
		}

		CComPtr<IObjectWithSite> site;
		if (SUCCEEDED(m_pPreviewHandler->QueryInterface(&site))) {
			site->SetSite(GetPreviewHandlerFramePtr());
		}

		CRect rc(GetClientRect());

		FAILED_THROW(m_pPreviewHandler->SetWindow(m_hWnd, &rc));
		FAILED_THROW(m_pPreviewHandler->SetRect(&rc));
		FAILED_THROW(m_pPreviewHandler->DoPreview());
		m_path = path;
	}
}

void CPreviewWnd::Open()
{
	std::wstring path;
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;// GetWndPtr()->m_hWnd;
	//ofn.lpstrFilter = L"Text file(*.txt)\0*.txt\0\0";
	ofn.lpstrFile = ::GetBuffer(path, MAX_PATH);
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = L"Open";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	//ofn.lpstrDefExt = L"txt";

	if (!GetOpenFileName(&ofn)) {
		DWORD errCode = CommDlgExtendedError();
		if (errCode) {
			throw std::exception(FILE_LINE_FUNC);
		}
	} else {
		::ReleaseBuffer(path);
		Open(path);
	}
}

void CPreviewWnd::Close()
{
	//if (m_pFileIsInUse) {
	//	RemoveMsgHandler(CFileIsInUseImpl::WM_FILEINUSE_CLOSEFILE);
	//	m_pFileIsInUse.Release();
	//}

	if (m_pPreviewHandler) {
		FAILED_THROW(m_pPreviewHandler->Unload());
		m_pPreviewHandler.Release();
	}
}

//LRESULT CPreviewWnd::OnCreate(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//LRESULT CPreviewWnd::OnClose(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//LRESULT CPreviewWnd::OnDestroy(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//LRESULT CPreviewWnd::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
LRESULT CPreviewWnd::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_pPreviewHandler) {
		CRect rc(GetClientRect());
		FAILED_THROW(m_pPreviewHandler->SetRect(&rc));
	}
	return 0;
}
