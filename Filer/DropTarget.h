#pragma once
#include "Unknown.h"
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
		boost::signals2::signal<void(IDataObject*, DWORD)> Dropped;
};