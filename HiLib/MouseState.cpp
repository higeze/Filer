#include "MouseState.h"
#include "Sheet.h"
#include "RowColumn.h"
#include "Column.h"
#include "Cell.h"
#include "IMouseObserver.h"
#include "Tracker.h"
#include "Dragger.h"
#include "GridView.h"

bool IMouseState::m_isDblClkTimeExceed;

IMouseState* IMouseState::ChangeState(CSheet* pSheet, IMouseState* pMouseState, MouseEventArgs& e)
{
	Exit(pSheet, e);
	pMouseState->Entry(pSheet, e);
	return pMouseState;
}

IMouseState* IMouseState::KeepState()
{
	return this;
}

void IMouseState::Entry(CSheet* pSheet, MouseEventArgs& e) 
{
	std::cout << "IMouseState::Entry" << std::endl; 
	m_isDblClkTimeExceed = false;
}

IMouseState* IMouseState::OnMouseLeave(CSheet* pSheet, MouseEventArgs& e)
{
	if (auto p = dynamic_cast<CGridView*>(pSheet)) {
		p->GetTimerPtr()->cancel();
	}
	pSheet->OnMouseLeave(e);
	return ChangeState(pSheet, CDefaultMouseState::State(), e);
}

IMouseState* CDefaultMouseState::State()
{
	std::cout << "CDefaultMouseState::State" << std::endl;
	static CDefaultMouseState state;
	return &state;
}
IMouseState* CDefaultMouseState::OnLButtonDown(CSheet* pSheet, MouseEventArgs& e)
{
	pSheet->OnLButtonDown(e);
	return ChangeState(pSheet, CDownedMouseState::State(), e);
}
IMouseState* CDefaultMouseState::OnLButtonUp(CSheet* pSheet, MouseEventArgs& e)
{
	return KeepState();
}
IMouseState* CDefaultMouseState::OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e)
{
	return KeepState();
}
IMouseState* CDefaultMouseState::OnLButtonDblClkTimeExceed(CSheet* pSheet, MouseEventArgs& e)
{
	m_isDblClkTimeExceed = true;
	return KeepState();
}

void CDownedMouseState::Entry(CSheet* pSheet, MouseEventArgs& e)
{
	std::cout << "CDownedMouseState::Entry" << std::endl;

	if (auto p = dynamic_cast<CGridView*>(pSheet)) {
		boost::asio::deadline_timer* pTimer = p->GetTimerPtr();
		pTimer->expires_from_now(boost::posix_time::milliseconds(::GetDoubleClickTime()));
		HWND hWnd = pSheet->GetGridPtr()->m_hWnd;
		pTimer->async_wait([hWnd, e](const boost::system::error_code& error)->void {

			if (error == boost::asio::error::operation_aborted) {
				std::cout << "timer canceled" << std::endl;
			}
			else {
				std::cout << "timer editcell" << std::endl;
				::PostMessage(hWnd, WM_LBUTTONDBLCLKTIMEXCEED, NULL, MAKELPARAM(e.Point.x, e.Point.y));
			}
		});
	}
	m_isDblClkTimeExceed = false;
}

IMouseState* CDownedMouseState::State()
{
	std::cout << "CDownedMouseState::State" << std::endl;
	static CDownedMouseState state;
	return &state;
}
IMouseState* CDownedMouseState::OnLButtonDown(CSheet* pSheet, MouseEventArgs& e)
{
	return KeepState();
}
IMouseState* CDownedMouseState::OnLButtonUp(CSheet* pSheet, MouseEventArgs& e)
{
	pSheet->OnLButtonUp(e);
	if (m_isDblClkTimeExceed) {
		std::cout << "CUppedMouseState::LButtonSnglClk" << std::endl;
		pSheet->OnLButtonSnglClk(e);
		if (auto p = dynamic_cast<CGridView*>(pSheet)) {
			p->GetTimerPtr()->cancel();
		}
		return ChangeState(pSheet, CDefaultMouseState::State(), e);
	}else{
		return ChangeState(pSheet, CUppedMouseState::State(), e);
	}
}
IMouseState* CDownedMouseState::OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e)
{
	return KeepState();
}
IMouseState* CDownedMouseState::OnLButtonDblClkTimeExceed(CSheet* pSheet, MouseEventArgs& e)
{
	m_isDblClkTimeExceed = true;
	return ChangeState(pSheet, CDragMouseState::State(), e);
}

IMouseState* CUppedMouseState::State()
{
	std::cout << "CUppedMouseState::State" << std::endl;
	static CUppedMouseState state;
	return &state;
}

void CUppedMouseState::Entry(CSheet* pSheet, MouseEventArgs& e)
{
	std::cout << "CUppedMouseState::Entry" << std::endl;
}
IMouseState* CUppedMouseState::OnLButtonDown(CSheet* pSheet, MouseEventArgs& e)
{
	pSheet->OnLButtonDown(e);
	return KeepState();
}
IMouseState* CUppedMouseState::OnLButtonUp(CSheet* pSheet, MouseEventArgs& e)
{
	return KeepState();
}
IMouseState* CUppedMouseState::OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e)
{
	pSheet->OnLButtonDblClk(e);
	return ChangeState(pSheet, CDblClkedMouseState::State(), e);
}
IMouseState* CUppedMouseState::OnLButtonDblClkTimeExceed(CSheet* pSheet, MouseEventArgs& e)
{
	m_isDblClkTimeExceed = true;
	std::cout << "CUppedMouseState::LButtonSnglClk" << std::endl;
	pSheet->OnLButtonSnglClk(e);
	if (auto p = dynamic_cast<CGridView*>(pSheet)) {
		p->GetTimerPtr()->cancel();
	}
	return ChangeState(pSheet, CDefaultMouseState::State(), e);
}

IMouseState* CDblClkedMouseState::State()
{
	std::cout << "CDblClkedMouseState::State" << std::endl;
	static CDblClkedMouseState state;
	return &state;
}
IMouseState* CDblClkedMouseState::OnLButtonDown(CSheet* pSheet, MouseEventArgs& e)
{
	return KeepState();
}
IMouseState* CDblClkedMouseState::OnLButtonUp(CSheet* pSheet, MouseEventArgs& e)
{
	return ChangeState(pSheet, CDefaultMouseState::State(), e);
}
IMouseState* CDblClkedMouseState::OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e)
{
	return KeepState();
}
IMouseState* CDblClkedMouseState::OnLButtonDblClkTimeExceed(CSheet* pSheet, MouseEventArgs& e)
{
	return KeepState();
}

void CDragMouseState::Entry(CSheet* pSheet, MouseEventArgs& e)
{
	std::cout << "CDragMouseState::Entry" << std::endl;
	pSheet->OnLButtonBeginDrag(e);
}


IMouseState* CDragMouseState::State()
{
	std::cout << "CDragMouseState::State" << std::endl;
	static CDragMouseState state;
	return &state;
}
IMouseState* CDragMouseState::OnLButtonDown(CSheet* pSheet, MouseEventArgs& e)
{
	return KeepState();
}
IMouseState* CDragMouseState::OnLButtonUp(CSheet* pSheet, MouseEventArgs& e)
{
	pSheet->OnLButtonEndDrag(e);
	return ChangeState(pSheet, CDefaultMouseState::State(), e);
}
IMouseState* CDragMouseState::OnLButtonDblClk(CSheet* pSheet, MouseEventArgs& e)
{
	return KeepState();
}
IMouseState* CDragMouseState::OnLButtonDblClkTimeExceed(CSheet* pSheet, MouseEventArgs& e)
{
	return KeepState();
}
