#pragma once
#include "Unknown.h"
#include "MyString.h"
#include <mapix.h>
class CIDL;
class CShellFolder;

template<typename TRect>
inline std::basic_string<TRect> GetMakeSureTempDirectory()
{
	//create temp directory if not exist
	TRect tempDir[MAX_PATH];
	::GetModuleFileNameA(NULL, tempDir, MAX_PATH);
	::PathRemoveFileSpecA(tempDir);
	::PathCombineA(tempDir, tempDir, "Temp");

	if (!::PathFileExistsA(tempDir)) {
		::CreateDirectoryA(tempDir, NULL);
	}
	return tempDir;
}

template<>
inline std::basic_string<wchar_t> GetMakeSureTempDirectory()
{
	//create temp directory if not exist
	wchar_t tempDir[MAX_PATH];
	::GetModuleFileName(NULL, tempDir, MAX_PATH);
	::PathRemoveFileSpec(tempDir);
	::PathCombine(tempDir, tempDir, L"Temp");

	if (!::PathFileExists(tempDir)) {
		::CreateDirectory(tempDir, NULL);
	}
	return tempDir;
}

template<typename TRect>
std::basic_string<TRect> SanitizeFileName(const std::basic_string<TRect>& fileIn)
{
	std::basic_string<TRect> fileOut(fileIn);
	for (size_t i = 0; i < fileOut.size(); i++) {
		switch (fileOut[i]) {
			case TCHAR_CAST(TRect, '^'):
			case TCHAR_CAST(TRect, '&'):
			case TCHAR_CAST(TRect, '*'):
			case TCHAR_CAST(TRect, '-'):
			case TCHAR_CAST(TRect, '+'):
			case TCHAR_CAST(TRect, '='):
			case TCHAR_CAST(TRect, '['):
			case TCHAR_CAST(TRect, ']'):
			case TCHAR_CAST(TRect, '\\'):
			case TCHAR_CAST(TRect, '|'):
			case TCHAR_CAST(TRect, ';'):
			case TCHAR_CAST(TRect, ':'):
			case TCHAR_CAST(TRect, '\"'):
			case TCHAR_CAST(TRect, ','):
			case TCHAR_CAST(TRect, '<'):
			case TCHAR_CAST(TRect, '>'):
			case TCHAR_CAST(TRect, '/'):
			case TCHAR_CAST(TRect, '?'):
				fileOut[i] = TCHAR_CAST(TRect, '_');
				break;
			default:
				;
		}
	}
	return fileOut;
}

class CD2DWControl;

class CDropTarget : public CUnknown<IDropTarget>
{
private:
	CD2DWControl*     m_pControl;
	BOOL              m_bSupportFormat;
	BOOL              m_bRButton;
	IDropTargetHelper *m_pDropTargetHelper;
public:
	CDropTarget(CD2DWControl* pControl);
	virtual ~CDropTarget();
	
	STDMETHODIMP DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) override;
	STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) override;
	STDMETHODIMP DragLeave() override;
	STDMETHODIMP Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)override;

	DWORD GetEffectFromKeyState(DWORD grfKeyState);
	void InitializeMenuItem(HMENU hmenu, LPTSTR lpszItemName, int nId);

	//signal
	//boost::signals2::signal<void(std::string, std::vector<CIDLPtr>)> Dropped;
	std::function<bool(const std::vector<FORMATETC>&)> IsDroppable;
	std::function<void(IDataObject*, DWORD)> Dropped;

public:
	/******************/
	/* Helper function*/
	/******************/
	static HRESULT CopyStream(const CComPtr<IFileOperation>& pFileOperation, const std::shared_ptr<CShellFolder>& pDestFolder, IStream* stream, const std::wstring& fileName);
	static HRESULT CopyMessage(const CComPtr<IFileOperation>& pFileOperation, const std::shared_ptr<CShellFolder>& pDestFolder, LPMESSAGE pSrcMessage);

};

