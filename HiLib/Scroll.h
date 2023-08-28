#pragma once
#include "Direct2DWrite.h"
#include "D2DWControl.h"
#include <sigslot/signal.hpp>
#include "reactive_property.h"

struct ScrollProperty;

	class CScrollBase :public CD2DWControl
	{
	public:
		sigslot::signal<> ScrollChanged;
		reactive_property_ptr<FLOAT> Position;
	protected:
		Visibility m_visibility = Visibility::Auto;
		FLOAT m_page = 0.0f;
		std::pair<FLOAT, FLOAT> m_range = std::make_pair(0.0f, 0.0f);

		std::shared_ptr<ScrollProperty> m_spScrollProp;

		FLOAT m_startDrag;
	public:
		CScrollBase(CD2DWControl* pParentControl, const std::shared_ptr<ScrollProperty>& spScrollProp/*, std::function<void(const wchar_t*)> onPropertyChanged*/);
		virtual ~CScrollBase() = default;

		FLOAT GetStartDrag()const { return m_startDrag; }
		void SetStartDrag(const FLOAT startDrag) { m_startDrag = startDrag; }
		FLOAT GetScrollBandWidth() const;
		FLOAT GetScrollDelta() const;
		FLOAT GetScrollPage()const { return m_page; }
		std::pair<FLOAT, FLOAT> GetScrollRange()const { return m_range; }
		FLOAT GetScrollDistance()const { return m_range.second - m_range.first; }
		FLOAT GetScrollPos()const { return *Position; }
		CSizeF GetSize()const { return GetRectInWnd().Size(); }

		void SetScrollPos(const FLOAT pos);
		void SetScrollPage(const FLOAT page);
		void SetScrollRange(const FLOAT min, FLOAT max);
		void SetScrollRangePage(const FLOAT min, const FLOAT max, const FLOAT page);

		Visibility GetVisibility()const { return m_visibility; }
		void SetVisibility(const Visibility& value) { m_visibility = value; }
		bool GetIsVisible()const;

		virtual void OnPaint(const PaintEvent& e) override;
		virtual void OnSetCursor(const SetCursorEvent& e) override;
		virtual CRectF GetThumbRangeRect()const;
		virtual CRectF GetThumbRect()const = 0;
		virtual void Clear();
		//virtual void OnPropertyChanged(const wchar_t* name) override
		//{
		//	m_onPropertyChanged(name);
		//}
	protected:
		virtual void PaintBackground(const PaintEvent& e);
		virtual void PaintThumb(const PaintEvent& e);
		virtual void PaintForeground(const PaintEvent& e) {}

	};

	class CVScroll :public CScrollBase
	{
	public:
		CVScroll(CD2DWControl* pParentControl, const std::shared_ptr<ScrollProperty>& spScrollProp/*, std::function<void(const wchar_t*)> onPropertyChanged*/)
			:CScrollBase(pParentControl, spScrollProp/*, onPropertyChanged*/){}
		virtual ~CVScroll() = default;
		virtual CRectF GetThumbRect()const override;
	};

	class CHScroll :public CScrollBase
	{
	public:
		CHScroll(CD2DWControl* pParentControl, const std::shared_ptr<ScrollProperty>& spScrollProp/*, std::function<void(const wchar_t*)> onPropertyChanged*/)
			:CScrollBase(pParentControl, spScrollProp/*, onPropertyChanged*/) {}
		virtual ~CHScroll() = default;
		virtual CRectF GetThumbRect()const override;
	};

