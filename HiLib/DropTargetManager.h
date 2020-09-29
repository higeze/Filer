#pragma once
#include <unordered_map>
#include "Unknown.h"
#include "Direct2DWrite.h"
#include <mapix.h>


class CD2DWWindow;
class CD2DWControl;

class CDropTargetManager : public CUnknown<IDropTarget>
{
public:
	using map_type = std::unordered_map<CD2DWControl*, IDropTarget*>;
private:
	CD2DWWindow* m_pWnd;
	CD2DWControl* m_pCurControl;
	map_type m_map;
	CComPtr<IDataObject> m_pDataObj;
public:
	CDropTargetManager(CD2DWWindow* pWnd):m_pWnd(pWnd), CUnknown<IDropTarget>(), m_pCurControl(nullptr){}

	STDMETHODIMP DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) override;
	STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) override;
	STDMETHODIMP DragLeave() override;
	STDMETHODIMP Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)override;

	map_type::const_iterator FindPtInRectControl(const CPointF& point);
	void RegisterDragDrop(CD2DWControl* pControl, IDropTarget* pTarget) { m_map.emplace(pControl, pTarget); }
};
