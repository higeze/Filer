#pragma once
#include "Direct2DWrite.h"
#include "UIElement.h"

class CGridView;

namespace d2dw
{
	class CScroll :public CUIElement
	{
	private:
		CGridView* m_pGrid;
		bool m_visible = true;
		FLOAT m_page = 0.0f;
		std::pair<FLOAT, FLOAT> m_range = std::make_pair(0.0f, 0.0f);
		FLOAT m_pos = 0.0f;
		CRectF m_rect;

		SolidFill m_backgroundFill = SolidFill(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 0.6f);
		SolidFill m_foregroundFill = SolidFill(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 0.9f);

	public:
		CScroll(CGridView* pGrid);
		virtual ~CScroll();

		//boost::signals2::signal<void(CScroll*)> SignalScroll;

		//struct StateMachine;
		//std::unique_ptr<StateMachine> m_pStateMachine;

		FLOAT GetScrollPage()const { return m_page; }
		std::pair<FLOAT, FLOAT> GetScrollRange()const { return m_range; }
		FLOAT GetScrollPos()const { return m_pos; }
		CRectF GetRect()const { return m_rect; }
		CRectF GetThumbRect()const{return 
			CRectF(
			m_rect.left,
			(std::max)(m_rect.top + m_rect.Height() * GetScrollPos() / (GetScrollRange().second - GetScrollRange().first), m_rect.top),
			m_rect.right,
			(std::min)(m_rect.top + m_rect.Height() * (GetScrollPos() + GetScrollPage()) / (GetScrollRange().second - GetScrollRange().first), m_rect.bottom));
		}
		CSizeF GetSize()const { return CSizeF(m_rect.Width(), m_rect.Height()); }
		bool GetVisible()const { return m_visible; }

		void SetScrollPage(const FLOAT page) { m_page = page; }
		void SetScrollRange(const FLOAT min, FLOAT max) { m_range.first = min; m_range.second = max; }
		void SetScrollPos(const FLOAT pos);
		void SetRect(const CRectF& rect){ m_rect = rect; }
		void SetRect(const FLOAT left, const FLOAT top, const FLOAT right, const FLOAT bottom) 
		{
			m_rect.SetRect(left, top, right, bottom);
		}
		void SetVisible(bool visible) { m_visible = visible; }
//		void ResetStateMachine();

		void OnPropertyChanged(const wchar_t* name) override;

		virtual void OnPaint(const PaintEvent& e);

		//virtual void OnLButtonDown(const LButtonDownEvent& e);
		//virtual void OnLButtonUp(const LButtonUpEvent& e);
		//virtual void OnMouseMove(const MouseMoveEvent& e);
		//virtual void OnMouseWheel(const MouseWheelEvent& e);






	};
}
