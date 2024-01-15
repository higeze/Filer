#include "DropTarget.h"
#include "IDL.h"
#include "MyString.h"
#include "ShellFolder.h"
#include "D2DWWindow.h"
#include "D2DWControl.h"

#define INITGUID
#include <objbase.h>

#define USES_IID_IMessage
#include <initguid.h> //this is needed,
#include <mapiguid.h> //then this
#include <mapix.h>
#include <mapitags.h>
#include <mapidefs.h>
#include <mapiutil.h>
#include <imessage.h>
#include <fcntl.h>

#define SETFormatEtc(fe, cf, asp, td, med, li)   \
    {\
    (fe).cfFormat=cf;\
    (fe).dwAspect=asp;\
    (fe).ptd=td;\
    (fe).tymed=med;\
    (fe).lindex=li;\
    }

#define SETDefFormatEtc(fe, cf, med)   \
    {\
    (fe).cfFormat=cf;\
    (fe).dwAspect=DVASPECT_CONTENT;\
    (fe).ptd=NULL;\
    (fe).tymed=med;\
    (fe).lindex=-1;\
    }

// {00020D0B-0000-0000-C000-000000000046}
DEFINE_GUID(CLSID_MailMessage,
	0x00020D0B,
	0x0000, 0x0000, 0xC0, 0x00, 0x0, 0x00, 0x0, 0x00, 0x00, 0x46);


CDropTarget::CDropTarget(CD2DWControl* pControl):CUnknown<IDropTarget>(), m_pControl(pControl)
{
	::CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pDropTargetHelper));
}

CDropTarget::~CDropTarget()
{
	if (m_pDropTargetHelper != nullptr){
		m_pDropTargetHelper->Release();
	}
}

STDMETHODIMP CDropTarget::DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	std::vector<FORMATETC> formats;
	CComPtr<IEnumFORMATETC> pEnumFormatEtc;
	if (SUCCEEDED(pDataObj->EnumFormatEtc(DATADIR::DATADIR_GET, &pEnumFormatEtc))) {
		FORMATETC rgelt[100];
		ULONG celtFetched = 0UL;
		if (SUCCEEDED(pEnumFormatEtc->Next(100, rgelt, &celtFetched))) {
			for (size_t i = 0; i < celtFetched; ++i) {
				formats.push_back(rgelt[i]);
			}
		}
	}

	m_bSupportFormat = IsDroppable(formats);
	if (m_bSupportFormat && m_pDropTargetHelper != NULL) {
		HRESULT hr = m_pDropTargetHelper->DragEnter(m_pControl->GetWndPtr()->m_hWnd, pDataObj, (LPPOINT)&pt, *pdwEffect);
	}
	*pdwEffect = m_bSupportFormat ? *pdwEffect = GetEffectFromKeyState(grfKeyState) : DROPEFFECT_NONE;

	m_bRButton = grfKeyState & MK_RBUTTON;

	return S_OK; 
}

STDMETHODIMP CDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	if (m_bSupportFormat && m_pDropTargetHelper != NULL) {
		HRESULT hr = m_pDropTargetHelper->DragOver((LPPOINT)&pt, *pdwEffect);
	}

	*pdwEffect = m_bSupportFormat ? *pdwEffect = GetEffectFromKeyState(grfKeyState) : DROPEFFECT_NONE;

	return S_OK;
}

STDMETHODIMP CDropTarget::DragLeave()
{
	if (m_pDropTargetHelper != NULL) {
		m_pDropTargetHelper->DragLeave();
	}

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
			InitializeMenuItem(hmenuPopup, const_cast<LPWSTR>(L"Move here"), DROPEFFECT_MOVE);
		if (dwEffect & DROPEFFECT_COPY)
			InitializeMenuItem(hmenuPopup, const_cast<LPWSTR>(L"Copy here"), DROPEFFECT_COPY);
		if (dwEffect & DROPEFFECT_LINK)
			InitializeMenuItem(hmenuPopup, const_cast<LPWSTR>(L"Link here"), DROPEFFECT_LINK);

		InitializeMenuItem(hmenuPopup, NULL, 100);
		InitializeMenuItem(hmenuPopup, const_cast<LPWSTR>(L"Cancel"), 0);

		nId = TrackPopupMenu(hmenuPopup, TPM_RETURNCMD, pt.x, pt.y, 0, m_pControl->GetWndPtr()->m_hWnd, NULL);
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
		if (grfKeyState & MK_SHIFT) {
			dwEffect = DROPEFFECT_LINK;
		} else {
			dwEffect = DROPEFFECT_MOVE;
		}
	} else {
		dwEffect = DROPEFFECT_COPY;
	}

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

/******************/
/* Helper function*/
/******************/
HRESULT CDropTarget::CopyStream(const CComPtr<IFileOperation>& pFileOperation, const std::shared_ptr<CShellFolder>& pDestFolder, IStream* pSrcStream, const std::wstring& fileName)
{
	HRESULT       hr = S_OK;
	unsigned char buffer[1024];
	unsigned long bytes_read = 0;
	int           bytes_written = 0;
	int* pFileHandle = new int();	//handle to a file

	std::wstring filePath;
	::PathCombine(GetBuffer(filePath, MAX_PATH), GetMakeSureTempDirectory<wchar_t>().c_str(), fileName.c_str());
	::ReleaseBuffer(filePath);

	errno_t errorNo = _wsopen_s(pFileHandle, filePath.c_str(), O_RDWR | O_BINARY | O_CREAT, SH_DENYNO, S_IREAD | S_IWRITE);
	if ((*pFileHandle) != -1) {
		do {
			hr = pSrcStream->Read(buffer, 1024, &bytes_read);
			if (bytes_read)
				bytes_written = _write((*pFileHandle), buffer, bytes_read);
		} while (S_OK == hr && bytes_read == 1024);
		_close((*pFileHandle));
		if (bytes_written == 0)
			_tunlink(filePath.c_str());
	} else {
		unsigned long  error;
		if ((error = GetLastError()) == 0L)
			error = _doserrno;
		hr = HRESULT_FROM_WIN32(errno);
	}

	// copy temp to dest
	CComPtr<IShellItem2> pSrcShellItem;
	CComPtr<IShellItem2> pDestShellItem;

	if (SUCCEEDED(::SHCreateItemFromIDList(pDestFolder->GetAbsoluteIdl().ptr(), IID_IShellItem2, reinterpret_cast<LPVOID*>(&pDestShellItem))) &&
		SUCCEEDED(::SHCreateItemFromParsingName(filePath.c_str(), nullptr, IID_IShellItem2, reinterpret_cast<LPVOID*>(&pSrcShellItem))) &&
		SUCCEEDED(pFileOperation->MoveItems(pSrcShellItem, pDestShellItem))) {
		return S_OK;
	} else {
		return S_FALSE;
	}
}

std::wstring SanitizeFileName(const std::wstring& dir, const std::wstring& file)
{
	const int slash_size = 1;
	const int nullterm_size = 1;
	if (dir.size() + slash_size + nullterm_size > MAX_PATH) {
		return std::wstring();
	}
	std::wstring file_sanitized = file;
	int pcs = ::PathCleanupSpec(dir.c_str(), ::GetBuffer(file_sanitized, MAX_PATH));
	::ReleaseBuffer(file_sanitized);

	if (pcs & PCS_FATAL){
		if(pcs & PCS_PATHTOOLONG) {
			std::wstring  file_wo_ext = file_sanitized;
			::PathRemoveExtensionW(::GetBuffer(file_wo_ext, MAX_PATH));
			::ReleaseBuffer(file_wo_ext);
			std::wstring ext = ::PathFindExtensionW(file_sanitized.c_str());
			std::wstring file_wo_ext_truncated = file_wo_ext.substr(0, MAX_PATH - dir.size() - slash_size - ext.size() - nullterm_size);
			return file_wo_ext_truncated + ext;
		} else {
			return std::wstring();
		}
	} else {
		return file_sanitized;
	}
}

HRESULT CDropTarget::CopyMessage(const CComPtr<IFileOperation>& pFileOperation, const std::shared_ptr<CShellFolder>& pDestFolder, LPMESSAGE pSrcMessage)
{
	HRESULT hRes = S_OK;
	LPSPropValue pSubject = NULL;
	LPSTORAGE pStorage = NULL;
	LPMSGSESS pMsgSession = NULL;
	LPMESSAGE pIMsg = NULL;
	SizedSPropTagArray(7, excludeTags);

	// get subject line of message to copy. This will be used as the new file name.
	HrGetOneProp(pSrcMessage, PR_SUBJECT, &pSubject);
	std::wstring title = pSubject != nullptr ? pSubject->Value.lpszW : L"NoTitle";

	//get file name
	std::wstring longerDir = pDestFolder->GetPath().size() > GetMakeSureTempDirectory<wchar_t>().size() ?
		pDestFolder->GetPath() : GetMakeSureTempDirectory<wchar_t>();
	std::wstring fileName = SanitizeFileName(longerDir, title + L".msg");
	if (fileName.empty()) {
		return S_FALSE;
	}

	// get temp file path
	std::wstring tempPath;
	::PathCombineW(GetBuffer(tempPath, _MAX_PATH), GetMakeSureTempDirectory<wchar_t>().c_str(), fileName.c_str());
	::ReleaseBuffer(tempPath);

	// get memory allocation function
	LPMALLOC pMalloc = MAPIGetDefaultMalloc();

	// create compound file
	hRes = ::StgCreateDocfile(tempPath.c_str(),
		STGM_READWRITE |
		STGM_TRANSACTED |
		STGM_CREATE, 0, &pStorage);

	// Open an IMessage session.
	hRes = ::OpenIMsgSession(pMalloc, 0, &pMsgSession);

	// Open an IMessage interface on an IStorage object
	hRes = ::OpenIMsgOnIStg(pMsgSession,
		MAPIAllocateBuffer,
		MAPIAllocateMore,
		MAPIFreeBuffer,
		pMalloc,
		NULL,
		pStorage,
		NULL, 0, 0, &pIMsg);

	// write the CLSID to the IStorage instance - pStorage. This will
	// only work with clients that support this compound document type
	// as the storage medium. If the client does not support
	// CLSID_MailMessage as the compound document, you will have to use
	// the CLSID that it does support.
	hRes = WriteClassStg(pStorage, CLSID_MailMessage);

	// Specify properties to exclude in the copy operation. These are
	// the properties that Exchange excludes to save bits and time.
	// Should not be necessary to exclude these, but speeds the process
	// when a lot of messages are being copied.
	excludeTags.cValues = 7;
	excludeTags.aulPropTag[0] = PR_ACCESS;
	excludeTags.aulPropTag[1] = PR_BODY;
	excludeTags.aulPropTag[2] = PR_RTF_SYNC_BODY_COUNT;
	excludeTags.aulPropTag[3] = PR_RTF_SYNC_BODY_CRC;
	excludeTags.aulPropTag[4] = PR_RTF_SYNC_BODY_TAG;
	excludeTags.aulPropTag[5] = PR_RTF_SYNC_PREFIX_COUNT;
	excludeTags.aulPropTag[6] = PR_RTF_SYNC_TRAILING_COUNT;

	// copy message properties to IMessage object opened on top of
	// IStorage.
	hRes = pSrcMessage->CopyTo(0, NULL,
		(LPSPropTagArray)&excludeTags,
		NULL, NULL,
		(LPIID)&IID_IMessage,
		pIMsg, 0, NULL);

	// save changes to IMessage object.
	pIMsg->SaveChanges(KEEP_OPEN_READWRITE);

	// save changes in storage of new doc file
	hRes = pStorage->Commit(STGC_DEFAULT);

	// free objects and clean up memory
	pStorage->Release();
	pIMsg->Release();
	CloseIMsgSession(pMsgSession);

	pStorage = NULL;
	pIMsg = NULL;
	pMsgSession = NULL;

	// copy temp to dest
	CComPtr<IShellItem2> pSrcShellItem;
	CComPtr<IShellItem2> pDestShellItem;
	
	if (SUCCEEDED(::SHCreateItemFromIDList(pDestFolder->GetAbsoluteIdl().ptr(), IID_IShellItem2, reinterpret_cast<LPVOID*>(&pDestShellItem))) &&
		SUCCEEDED(::SHCreateItemFromParsingName(tempPath.c_str(), nullptr, IID_IShellItem2, reinterpret_cast<LPVOID*>(&pSrcShellItem))) &&
		SUCCEEDED(pFileOperation->MoveItems(pSrcShellItem, pDestShellItem))){
		return  S_OK;
	} else {
		return  S_FALSE;
	}
}
