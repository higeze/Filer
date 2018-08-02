#include "DropTarget.h"
#include "IDL.h"

CDropTarget::CDropTarget(HWND hWnd):CUnknown<IDropTarget>(), m_hWnd(hWnd)
{
	::CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pDropTargetHelper));
}

CDropTarget::~CDropTarget()
{
	if (m_pDropTargetHelper != nullptr){
		m_pDropTargetHelper->Release();
	}
}

STDMETHODIMP CDropTarget::QueryInterface(REFIID riid, void **ppvObject)
{
	*ppvObject = NULL;

	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDropTarget)){
		*ppvObject = static_cast<IDropTarget *>(this);
	}else{
		return E_NOINTERFACE;
	}

	AddRef();

	return S_OK;
}

STDMETHODIMP CDropTarget::DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	HRESULT   hr;
	FORMATETC formatetc;
	
	*pdwEffect = GetEffectFromKeyState(grfKeyState);
	
	if (m_pDropTargetHelper != NULL)
		m_pDropTargetHelper->DragEnter(m_hWnd, pDataObj, (LPPOINT)&pt, *pdwEffect);

	formatetc.cfFormat = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_SHELLIDLIST);
	formatetc.ptd      = NULL;
	formatetc.dwAspect = DVASPECT_CONTENT;
	formatetc.lindex   = -1;
	formatetc.tymed    = TYMED_HGLOBAL;

	hr = pDataObj->QueryGetData(&formatetc);
	if (FAILED(hr)) {
		m_bSupportFormat = FALSE;
		*pdwEffect = DROPEFFECT_NONE;
	}
	else
		m_bSupportFormat = TRUE;

	m_bRButton = grfKeyState & MK_RBUTTON;

	return S_OK; 
}

STDMETHODIMP CDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	*pdwEffect = GetEffectFromKeyState(grfKeyState);
	
	if (m_pDropTargetHelper != NULL)
		m_pDropTargetHelper->DragOver((LPPOINT)&pt, *pdwEffect);

	if (!m_bSupportFormat)
		*pdwEffect = DROPEFFECT_NONE;

	return S_OK;
}

STDMETHODIMP CDropTarget::DragLeave()
{
	if (m_pDropTargetHelper != NULL)
		m_pDropTargetHelper->DragLeave();

	return S_OK;
}

STDMETHODIMP CDropTarget::Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{

	DWORD dwEffect = *pdwEffect;
	
	*pdwEffect = GetEffectFromKeyState(grfKeyState);

	if (m_pDropTargetHelper != NULL){
		m_pDropTargetHelper->Drop(pDataObj, (LPPOINT)&pt, *pdwEffect);
	}

	//FORMATETC formatetc = {0};
	//formatetc.cfFormat = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_SHELLIDLIST);
	//formatetc.ptd      = NULL;
	//formatetc.dwAspect = DVASPECT_CONTENT;
	//formatetc.lindex   = -1;
	//formatetc.tymed    = TYMED_HGLOBAL;
	//
	//STGMEDIUM medium;
	//HRESULT hr = pDataObj->GetData(&formatetc, &medium);
	//if (FAILED(hr)) {
	//	*pdwEffect = DROPEFFECT_NONE;
	//	return E_FAIL;
	//}

	if (m_bRButton) {
		int   nId;
		HMENU hmenuPopup;

		hmenuPopup = CreatePopupMenu();
		
		if (dwEffect & DROPEFFECT_MOVE)
			InitializeMenuItem(hmenuPopup, TEXT("Move here"), DROPEFFECT_MOVE);
		if (dwEffect & DROPEFFECT_COPY)
			InitializeMenuItem(hmenuPopup, TEXT("Copy here"), DROPEFFECT_COPY);
		if (dwEffect & DROPEFFECT_LINK)
			InitializeMenuItem(hmenuPopup, TEXT("Link here"), DROPEFFECT_LINK);

		InitializeMenuItem(hmenuPopup, NULL, 100);
		InitializeMenuItem(hmenuPopup, TEXT("Cancel"), 0);

		nId = TrackPopupMenu(hmenuPopup, TPM_RETURNCMD, pt.x, pt.y, 0, m_hWnd, NULL);
		if (nId == 0) {
			DestroyMenu(hmenuPopup);
			*pdwEffect = DROPEFFECT_NONE;
			return E_FAIL;
		}

		*pdwEffect = nId;
		DestroyMenu(hmenuPopup);
	}

	Dropped(pDataObj, *pdwEffect);
	
	return S_OK;
}

DWORD CDropTarget::GetEffectFromKeyState(DWORD grfKeyState)
{
	DWORD dwEffect;

	if (grfKeyState & MK_CONTROL) {
		if (grfKeyState & MK_SHIFT)
			dwEffect = DROPEFFECT_LINK;
		else
			dwEffect = DROPEFFECT_COPY;
	}
	else
		dwEffect = DROPEFFECT_MOVE;

	return dwEffect;
}

void CDropTarget::InitializeMenuItem(HMENU hmenu, LPTSTR lpszItemName, int nId)
{
	MENUITEMINFO mii;
	
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask  = MIIM_ID | MIIM_TYPE;
	mii.wID    = nId;

	if (lpszItemName != NULL) {
		mii.fType      = MFT_STRING;
		mii.dwTypeData = lpszItemName;
	}
	else
		mii.fType = MFT_SEPARATOR;

	InsertMenuItem(hmenu, nId, FALSE, &mii);
}