#pragma
#include "UIElement.h"
#include <boost/sml.hpp>

template<class T, class R, class... Ts>
auto call(R(T::* f)(Ts...))
{
	return [f](T* self, Ts... args) {return (self->*f)(args...); };
}

//template<class R, class... Ts>
//auto call_impl(R(*f)(Ts...))
//{
//	return [f](Ts... args) {return f(args...); };
//}
//
//template<class T, class R, class... Ts>
//auto call_impl(R(T::* f)(Ts...))
//{
//	return [f](Ts... args) {return f(T,args...); };
//}
//
//template<class T, class R, class... Ts>
//auto call_impl(T* self, R(T::* f)(Ts...))
//{
//	return [self, f](Ts... args) {return self->*f(args...); };
//}
//
//template<class T, class R, class... Ts>
//auto call_impl(const T* self, R(T::* f)(Ts...)const)
//{
//	return [self, f](Ts... args) {return self->*f(args...); };
//}
//
//template<class T, class R, class... Ts>
//auto call_impl(const T* self, R(T::* f)(Ts...))
//{
//	return [self, f](Ts... args) {return self->*f(args...); };
//}
//
//auto call = [](auto... args) {return call_impl(args...); };

//Pre-declare
//class CGridView;
//namespace sml = boost::sml;
//
//struct CGridViewStateMachine
//{
//	CGridViewStateMachine(CGridView* pGrid);
//	~CGridViewStateMachine();
//
//	struct Impl;
//	std::unique_ptr<Impl> pImpl;
//
//	void ProcessEvent(const LButtonDownEvent& e);
//	void ProcessEvent(const LButtonUpEvent& e);
//	void ProcessEvent(const LButtonClkEvent& e);
//	void ProcessEvent(const LButtonSnglClkEvent& e);
//	void ProcessEvent(const LButtonDblClkEvent& e);
//	void ProcessEvent(const LButtonBeginDragEvent& e);
//	void ProcessEvent(const RButtonDownEvent& e);
//	void ProcessEvent(const ContextMenuEvent& e);
//	void ProcessEvent(const MouseMoveEvent& e);
//	void ProcessEvent(const MouseLeaveEvent& e);
//	void ProcessEvent(const SetCursorEvent& e);
//	void ProcessEvent(const KeyDownEvent& e);
//	void ProcessEvent(const CharEvent& e);
//	void ProcessEvent(const BeginEditEvent& e);
//};
