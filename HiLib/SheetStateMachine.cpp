#include "SheetStateMachine.h"
#include "Cursorer.h"
#include "Dragger.h"
#include "Tracker.h"
#include "Celler.h"
#include "SheetEventArgs.h"
#include "Sheet.h"
#include "Cell.h"
#include "Scroll.h"
#include "Textbox.h"
#include "TextCell.h"

#include <boost\msm\front\state_machine_def.hpp>
#include <boost\msm\back\state_machine.hpp>
#include <boost\mpl\vector.hpp>


//CSheetStateMachine::CSheetStateMachine(CSheet* pSheet) :pImpl(new Impl(this, pSheet))
//{
//	pImpl->m_machine.start();
//}

CSheetStateMachine::~CSheetStateMachine() = default;

//void CSheetStateMachine::LButtonDown(const LButtonDownEvent& e) { pImpl->m_machine.process_event(e); }
//void CSheetStateMachine::LButtonUp(const LButtonUpEvent& e) { pImpl->m_machine.process_event(e); }
//void CSheetStateMachine::LButtonClk(const LButtonClkEvent& e) { pImpl->m_machine.process_event(e); }
//void CSheetStateMachine::LButtonSnglClk(const LButtonSnglClkEvent& e) { pImpl->m_machine.process_event(e); }
//void CSheetStateMachine::LButtonDblClk(const LButtonDblClkEvent& e) { pImpl->m_machine.process_event(e); }
//void CSheetStateMachine::RButtonDown(const RButtonDownEvent& e) { pImpl->m_machine.process_event(e); }
//void CSheetStateMachine::MouseMove(const MouseMoveEvent& e) { pImpl->m_machine.process_event(e); }
//void CSheetStateMachine::MouseLeave(const MouseLeaveEvent& e) { pImpl->m_machine.process_event(e); }
//void CSheetStateMachine::LButtonBeginDrag(const LButtonBeginDragEvent& e) { pImpl->m_machine.process_event(e); }
//void CSheetStateMachine::SetCursor(const SetCursorEvent& e) { pImpl->m_machine.process_event(e); }
//void CSheetStateMachine::ContextMenu(const ContextMenuEvent& e) { pImpl->m_machine.process_event(e); }
//void CSheetStateMachine::KeyDown(const KeyDownEvent& e) { pImpl->m_machine.process_event(e); }
//void CSheetStateMachine::Char(const CharEvent& e) { pImpl->m_machine.process_event(e); }
//void CSheetStateMachine::BeginEdit(const BeginEditEvent& e) { pImpl->m_machine.process_event(e); }

