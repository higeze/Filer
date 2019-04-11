#pragma once
#include "Direct2DWrite.h"
#include "UIElement.h"

class CGridView;

namespace d2dw
{
	class CScrollBase :public CUIElement
	{
	protected:
		CGridView* m_pGrid;
		bool m_visible = true;
		FLOAT m_page = 0.0f;
		std::pair<FLOAT, FLOAT> m_range = std::make_pair(0.0f, 0.0f);
		FLOAT m_pos = 0.0f;
		CRectF m_rect;

		SolidFill m_backgroundFill = SolidFill(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 0.4f);
		SolidFill m_foregroundFill = SolidFill(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 0.9f);

	public:
		CScrollBase(CGridView* pGrid);
		virtual ~CScrollBase() = default;

		FLOAT GetScrollPage()const { return m_page; }
		std::pair<FLOAT, FLOAT> GetScrollRange()const { return m_range; }
		FLOAT GetScrollDistance()const { return m_range.second - m_range.first; }
		FLOAT GetScrollPos()const { return m_pos; }
		CRectF GetRect()const { return m_rect; }
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
		virtual void OnPaint(const PaintEvent& e);
		virtual CRectF GetThumbRect()const = 0;

		template <class Archive>
		void serialize(Archive& ar)
		{
			ar("BackgroundFill", m_backgroundFill);
			ar("ForegroundFill", m_foregroundFill);
		}

	};

	class CVScroll :public CScrollBase
	{
	public:
		CVScroll(CGridView* pGrid):CScrollBase(pGrid){}
		virtual ~CVScroll() = default;
		virtual CRectF GetThumbRect()const override;
		virtual void OnPropertyChanged(const wchar_t* name) override;
	};

	class CHScroll :public CScrollBase
	{
	public:
		CHScroll(CGridView* pGrid) :CScrollBase(pGrid) {}
		virtual ~CHScroll() = default;
		virtual CRectF GetThumbRect()const override;
		virtual void OnPropertyChanged(const wchar_t* name) override;
	};

}
