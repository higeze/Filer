#pragma
#include "UIElement.h"
#include <boost/sml.hpp>

template<class R, class... Ts>
auto call_impl(R(*f)(Ts...))
{
	return [f](Ts... args) {return f(args...); };
}

template<class T, class R, class... Ts>
auto call_impl(T* self, R(T::* f)(Ts...))
{
	return [self, f](Ts... args) {return self->*f(args...); };
}

template<class T, class R, class... Ts>
auto call_impl(const T* self, R(T::* f)(Ts...)const)
{
	return [self, f](Ts... args) {return self->*f(args...); };
}

template<class T, class R, class... Ts>
auto call_impl(const T* self, R(T::* f)(Ts...))
{
	return [self, f](Ts... args) {return self->*f(args...); };
}

auto call = [](auto... args) {return call_impl(args...); };

//Pre-declare
class CGridView;
namespace sml = boost::sml;

struct CGridViewStateMachine
{
	CGridViewStateMachine(CGridView* pGrid);
	~CGridViewStateMachine();

	struct Impl;
	std::unique_ptr<Impl> pImpl;

	//void LButtonDown(const LButtonDownEvent& e);
	//void LButtonUp(const LButtonUpEvent& e);
	//void LButtonClk(const LButtonClkEvent& e);
	//void LButtonSnglClk(const LButtonSnglClkEvent& e);
	//void LButtonDblClk(const LButtonDblClkEvent& e);
	//void LButtonBeginDrag(const LButtonBeginDragEvent& e);
	//void RButtonDown(const RButtonDownEvent& e);
	//void ContextMenu(const ContextMenuEvent& e);
	//void MouseMove(const MouseMoveEvent& e);
	//void MouseLeave(const MouseLeaveEvent& e);
	//void SetCursor(const SetCursorEvent& e);
	//void KeyDown(const KeyDownEvent& e);
	//void Char(const CharEvent& e);
	//void BeginEdit(const BeginEditEvent& e);
};
