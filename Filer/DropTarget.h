#pragma once
#include "Unknown.h"
#include <mapix.h>
class CIDL;

class CDropTarget : public CUnknown<IDropTarget>
{
private:
	HWND              m_hWnd;
	BOOL              m_bSupportFormat;
	BOOL              m_bRButton;
	IDropTargetHelper *m_pDropTargetHelper;
public:
	CDropTarget(HWND hwnd);
	virtual ~CDropTarget();
	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject) override;
	
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
	static HRESULT StreamToFile(IStream* stream, const std::wstring& filePath);
	static HRESULT MessageToFile(LPMESSAGE pMessage, const std::wstring& dirPath);

};