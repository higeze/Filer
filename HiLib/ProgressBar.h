#pragma once
#include "Direct2DWrite.h"
#include "UIElement.h"
#include "GridViewProperty.h"

class CGridView;

namespace d2dw
{
	class CProgressBar :public CUIElement
	{
	protected:
		CWnd* m_pWnd;

		CRectF m_rect;
		
		FLOAT m_min = 0.0f;
		FLOAT m_max = 100.0f;
		FLOAT m_value = 0.0f;

		std::shared_ptr<ProgressProperty> m_spProgressProp;
	public:
		CProgressBar(CWnd* pWnd, const std::shared_ptr<ProgressProperty>& spProgressProp);
		virtual ~CProgressBar() = default;

		FLOAT GetMax()const { return m_max; }
		void AddMax(FLOAT i)
		{
			m_max += i;
		}
		void SetMax(FLOAT max) 
		{
			if (m_max != max) {
				m_max = max;
				m_value = (std::min)(m_max, m_value);
			}
		}
		FLOAT GetMin()const { return m_min; }
		void SetMin(FLOAT min)
		{
			if (m_min != min) {
				m_min = min;
				m_value = (std::max)(m_min, m_value);
			}
		}
		FLOAT GetValue()const { return m_value; }
		void AddValue(FLOAT i)
		{
			m_value += i;
		}
		void SetValue(FLOAT value)
		{
			if (m_value != value) {
				m_value = std::clamp(value, m_min, m_max);
			}
		}

		CRectF GetRect()const { return m_rect; }
		void SetRect(const CRectF& rect) { m_rect = rect; }
		void SetRect(const FLOAT left, const FLOAT top, const FLOAT right, const FLOAT bottom)
		{
			m_rect.SetRect(left, top, right, bottom);
		}
		CSizeF GetSize()const { return CSizeF(m_rect.Width(), m_rect.Height()); }
		virtual void OnPaint(const PaintEvent& e);
	};
}
