#pragma once
#include "Direct2DWrite.h"
#include "UIControl.h"

class CGridView;
struct ScrollProperty;

namespace d2dw
{
	class CScrollBase :public CUIControl
	{
	protected:
		bool m_visible = true;
		FLOAT m_page = 0.0f;
		std::pair<FLOAT, FLOAT> m_range = std::make_pair(0.0f, 0.0f);
		FLOAT m_pos = 0.0f;

		std::shared_ptr<ScrollProperty> m_spScrollProp;

		FLOAT m_startDrag;

		std::function<void(const wchar_t*)> m_onPropertyChanged;


	public:
		CScrollBase(CWindow* pWnd, const std::shared_ptr<ScrollProperty>& spScrollProp, std::function<void(const wchar_t*)> onPropertyChanged);
		virtual ~CScrollBase() = default;

		FLOAT GetStartDrag()const { return m_startDrag; }
		void SetStartDrag(const FLOAT startDrag) { m_startDrag = startDrag; }
		FLOAT GetScrollBandWidth() const;
		FLOAT GetScrollDelta() const;
		FLOAT GetScrollPage()const { return m_page; }
		std::pair<FLOAT, FLOAT> GetScrollRange()const { return m_range; }
		FLOAT GetScrollDistance()const { return m_range.second - m_range.first; }
		FLOAT GetScrollPos()const { return m_pos; }
		CSizeF GetSize()const { return GetRectInWnd().Size(); }
		bool GetVisible()const { return m_visible; }

		void SetScrollPos(const FLOAT pos);
		void SetScrollPage(const FLOAT page);
		void SetScrollRange(const FLOAT min, FLOAT max);
		//void SetRect(const CRectF& rect){ m_rect = rect; }
		//void SetRect(const FLOAT left, const FLOAT top, const FLOAT right, const FLOAT bottom) 
		//{
		//	m_rect.SetRect(left, top, right, bottom);
		//}
		void SetVisible(bool visible) { m_visible = visible; }
		virtual void OnPaint(const PaintEvent& e) override;
		virtual CRectF GetThumbRect()const = 0;
		virtual void OnPropertyChanged(const wchar_t* name) override
		{
			m_onPropertyChanged(name);
		}

	};

	class CVScroll :public CScrollBase
	{
	public:
		CVScroll(CWindow* pWnd, const std::shared_ptr<ScrollProperty>& spScrollProp, std::function<void(const wchar_t*)> onPropertyChanged)
			:CScrollBase(pWnd, spScrollProp, onPropertyChanged){}
		virtual ~CVScroll() = default;
		virtual CRectF GetThumbRect()const override;
	};

	class CHScroll :public CScrollBase
	{
	public:
		CHScroll(CWindow* pWnd, const std::shared_ptr<ScrollProperty>& spScrollProp, std::function<void(const wchar_t*)> onPropertyChanged)
			:CScrollBase(pWnd, spScrollProp, onPropertyChanged) {}
		virtual ~CHScroll() = default;
		virtual CRectF GetThumbRect()const override;
	};

}
