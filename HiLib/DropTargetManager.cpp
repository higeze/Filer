#include "DropTargetManager.h"
#include "D2DWWindow.h"
#include "D2DWControl.h"

STDMETHODIMP CDropTargetManager::DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	m_pDataObj.Attach(pDataObj);
	m_pDataObj.p->AddRef();

	auto iter = FindPtInRectControl(m_pWnd->GetDirectPtr()->Pixels2Dips(m_pWnd->GetWndPtr()->ScreenToClient(CPoint(pt.x, pt.y)).value()));
	if (iter != m_map.end()) {
		m_pCurControl = iter->first;
		return iter->second->DragEnter(pDataObj, grfKeyState, pt, pdwEffect);
	} else {
		*pdwEffect = DROPEFFECT_NONE;
		return S_OK;
	}
}
STDMETHODIMP CDropTargetManager::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	auto iter = FindPtInRectControl(m_pWnd->GetDirectPtr()->Pixels2Dips(m_pWnd->GetWndPtr()->ScreenToClient(CPoint(pt.x, pt.y)).value()));
	if (iter != m_map.end()) {
		if (m_pCurControl != iter->first) {
			if (m_pCurControl) {
				m_map[m_pCurControl]->DragLeave();
			}
			m_pCurControl = iter->first;
			iter->second->DragEnter(m_pDataObj, grfKeyState, pt, pdwEffect);
		}
		return iter->second->DragOver(grfKeyState, pt, pdwEffect);
	} else {
		if (m_pCurControl) {
			m_map[m_pCurControl]->DragLeave();
			m_pCurControl = nullptr;
		}
		*pdwEffect = DROPEFFECT_NONE;
		return S_OK;
	}
}
STDMETHODIMP CDropTargetManager::DragLeave()
{
	if (m_pCurControl) {	
		m_map[m_pCurControl]->DragLeave();
	}

	m_pCurControl = nullptr;
	m_pDataObj.Release();
	m_pDataObj.Detach();
	return S_OK;
}
STDMETHODIMP CDropTargetManager::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	auto iter = FindPtInRectControl(m_pWnd->GetDirectPtr()->Pixels2Dips(m_pWnd->GetWndPtr()->ScreenToClient(CPoint(pt.x, pt.y)).value()));
	HRESULT ret;
	if (iter != m_map.end()) {
		m_pCurControl = iter->first;
		ret =  iter->second->Drop(pDataObj, grfKeyState, pt, pdwEffect);
	} else {
		ret =  S_FALSE;
	}
	m_pCurControl = nullptr;
	m_pDataObj.Release();
	m_pDataObj.Detach();
	return ret;
}

CDropTargetManager::map_type::const_iterator CDropTargetManager::FindPtInRectControl(const CPointF& point)
{
	return std::find_if(m_map.begin(), m_map.end(),
		[point](const std::pair<CD2DWControl*, IDropTarget*>& pair)->bool
		{
			return pair.first->GetRectInWnd().PtInRect(point);
		});
}


