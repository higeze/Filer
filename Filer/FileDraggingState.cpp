#include "FileDraggingState.h"
#include "FilerGridView.h"
#include "FileDragger.h"

ISheetState* CFileDraggingState::FileDragging()
{
	static CFileDraggingState state;
	return &state;
}

ISheetState* CFileDraggingState::OnLButtonDown(CSheet* pSheet, MouseEventArgs& e)
{
	return static_cast<CFilerGridView*>(pSheet)->m_spFileDragger->OnDragLButtonDown(pSheet, e);
}
ISheetState* CFileDraggingState::OnLButtonUp(CSheet* pSheet, MouseEventArgs& e)
{
	return static_cast<CFilerGridView*>(pSheet)->m_spFileDragger->OnDragLButtonUp(pSheet, e);		
}
ISheetState* CFileDraggingState::OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e)
{
	return static_cast<CFilerGridView*>(pSheet)->m_spFileDragger->OnDragLButtonDblClk(pSheet, e);		
}
ISheetState* CFileDraggingState::OnLButtonDblClkTimeExceed(CSheet* pSheet, MouseEventArgs& e)
{
	return FileDragging();		
}
ISheetState* CFileDraggingState::OnRButtonDown(CSheet* pSheet, MouseEventArgs& e)
{
	return static_cast<CFilerGridView*>(pSheet)->m_spFileDragger->OnDragRButtonDown(pSheet, e);		
}
ISheetState* CFileDraggingState::OnMouseMove(CSheet* pSheet, MouseEventArgs& e)
{
	return static_cast<CFilerGridView*>(pSheet)->m_spFileDragger->OnDragMouseMove(pSheet, e);		
}
ISheetState* CFileDraggingState::OnMouseLeave(CSheet* pSheet, MouseEventArgs& e)
{
	return static_cast<CFilerGridView*>(pSheet)->m_spFileDragger->OnDragMouseLeave(pSheet, e);		
}
ISheetState* CFileDraggingState::OnSetCursor(CSheet* pSheet, SetCursorEventArgs& e)
{
	return static_cast<CFilerGridView*>(pSheet)->m_spFileDragger->OnDragSetCursor(pSheet, e);		
}