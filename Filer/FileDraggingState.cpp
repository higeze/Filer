#include "FileDraggingState.h"
#include "FilerGridView.h"
#include "FileDragger.h"

CSheetState* CFileDraggingState::FileDragging()
{
	static CFileDraggingState state;
	return &state;
}

CSheetState* CFileDraggingState::OnLButtonDown(CSheet* pSheet, MouseEventArgs& e)
{
	return static_cast<CFilerGridView*>(pSheet)->m_spFileDragger->OnDragLButtonDown(pSheet, e);
}
CSheetState* CFileDraggingState::OnLButtonUp(CSheet* pSheet, MouseEventArgs& e)
{
	return static_cast<CFilerGridView*>(pSheet)->m_spFileDragger->OnDragLButtonUp(pSheet, e);		
}
CSheetState* CFileDraggingState::OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e)
{
	return static_cast<CFilerGridView*>(pSheet)->m_spFileDragger->OnDragLButtonDblClk(pSheet, e);		
}
CSheetState* CFileDraggingState::OnLButtonDblClkTimeExceed(CSheet* pSheet, MouseEventArgs& e)
{
	return FileDragging();		
}
CSheetState* CFileDraggingState::OnRButtonDown(CSheet* pSheet, MouseEventArgs& e)
{
	return static_cast<CFilerGridView*>(pSheet)->m_spFileDragger->OnDragRButtonDown(pSheet, e);		
}
CSheetState* CFileDraggingState::OnMouseMove(CSheet* pSheet, MouseEventArgs& e)
{
	return static_cast<CFilerGridView*>(pSheet)->m_spFileDragger->OnDragMouseMove(pSheet, e);		
}
CSheetState* CFileDraggingState::OnMouseLeave(CSheet* pSheet, MouseEventArgs& e)
{
	return static_cast<CFilerGridView*>(pSheet)->m_spFileDragger->OnDragMouseLeave(pSheet, e);		
}
CSheetState* CFileDraggingState::OnSetCursor(CSheet* pSheet, SetCursorEventArgs& e)
{
	return static_cast<CFilerGridView*>(pSheet)->m_spFileDragger->OnDragSetCursor(pSheet, e);		
}