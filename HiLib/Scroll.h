#pragma once
#include "Direct2DWrite.h"
#include "UIElement.h"

namespace d2dw
{
	class CScroll :public CUIElement
	{
	private:
		bool m_visible = true;
		FLOAT m_page = 0.0f;
		FLOAT m_range = 0.0f;
		FLOAT m_position = 0.0f;
		CRectF m_rect;

	public:
		FLOAT GetScrollPage()const { return m_page; }
		FLOAT GetScrollRange()const { return m_range; }
		FLOAT GetScrollPos()const { return m_position; }
		void SetScrollPage();
		void SetScrollRange();
		void SetScrollPos();

		virtual void OnPaint(const PaintEvent& e);





	};
}
