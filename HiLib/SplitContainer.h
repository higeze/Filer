#pragma once
#include "D2DWControl.h"
#include "reactive_property.h"
#include "JsonSerializer.h"
#include "Splitter.h"

class CSplitter;

class CDockPanel : public CD2DWControl
{
protected:
	//std::unordered_map<std::shared_ptr<CD2DWControl>, std::shared_ptr<CSplitter>> m_ctrl_split_map;
public:
	virtual const CRectF& GetMargin() const override
	{ 
		static const CRectF value(0.f, 0.f, 0.f, 0.f); return value; 
	}
	virtual const CRectF& GetPadding() const override
	{ 
		static const CRectF value(0.f, 0.f, 0.f, 0.f); return value; 
	}

public:
	CDockPanel(CD2DWControl* pParentControl = nullptr)
		:CD2DWControl(pParentControl){}
	virtual ~CDockPanel() = default;

	//Event
	virtual void OnCreate(const CreateEvt& e) override;
	//MeasureArrange
	virtual CSizeF MeasureOverride(const CSizeF& availableSize) override;
	virtual void ArrangeOverride(const CRectF& rc) override;
	//Add
	void Add() {}
	void Add(std::pair<std::shared_ptr<CD2DWControl>, DockEnum>&& p)
	{
		p.first->Dock.set(p.second);
		AddChildControlPtr(p.first);
	}
	//template<class _Head>
	//void Add(_Head&& child)
	//{
	//	switch (*child->Dock) {
	//		case DockEnum::Left:
	//		case DockEnum::Right:
	//		{
	//			auto splitter = std::make_shared<CVerticalSplitter>(this);
	//			splitter->Value.subscribe([this](auto value) { Arrange(ArrangedRect()); }, shared_from_this());
	//			splitter->Dock.set(DockEnum::Vertical);
	//			m_ctrl_split_map.emplace(child, splitter);
	//			break;
	//		}
	//		case DockEnum::Top:
	//		case DockEnum::Bottom:
	//		{
	//			auto splitter = std::make_shared<CHorizontalSplitter>(this);
	//			splitter->Value.subscribe([this](auto value) { Arrange(ArrangedRect()); }, shared_from_this());
	//			splitter->Dock.set(DockEnum::Horizontal);
	//			m_ctrl_split_map.emplace(child, splitter);
	//			break;
	//		}
	//		default:
	//			m_ctrl_split_map.emplace(child, nullptr);
	//			break;
	//	}
	//}

	template<class _Head, class... _Tail>
	void Add(_Head&& head, _Tail&&... tail)
	{
		Add(std::forward<_Head>(head));
		Add(std::forward<_Tail>(tail)...);
	}

	//virtual void OnPaint(const PaintEvent& e) override;
	//virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) override;
	//virtual void OnLButtonEndDrag(const LButtonEndDragEvent& e) override;

	//virtual CRectF GetSplitterRect() const = 0;

	//NLOHMANN_DEFINE_TYPE_INTRUSIVE_NOTHROW(CSplitContainer,
	//	Maximum,
	//	Minimum,
	//	Value)
public:
	friend void to_json(json& j, const CDockPanel& o)
	{
		to_json(j, static_cast<const CD2DWControl&>(o));

		j["Children"] = o.m_childControls;
	}

	friend void from_json(const json& j, CDockPanel& o)
	{
		from_json(j, static_cast<CD2DWControl&>(o));

		json_safe_from(j, "Children", o.m_childControls);
	}

};

class CSplitContainer : public CD2DWControl
{
protected:
	std::shared_ptr<CD2DWControl> m_one;
	std::shared_ptr<CD2DWControl> m_two;
	std::shared_ptr<CSplitter> m_splitter;

public:
	virtual const CRectF& GetMargin() const override
	{ 
		static const CRectF value(0.f, 0.f, 0.f, 0.f); return value; 
	}
	virtual const CRectF& GetPadding() const override
	{ 
		static const CRectF value(0.f, 0.f, 0.f, 0.f); return value; 
	}

public:
	CSplitContainer(CD2DWControl* pParentControl = nullptr)
		:CD2DWControl(pParentControl){}
	virtual ~CSplitContainer() = default;

	//Event
	virtual void OnCreate(const CreateEvt& e) override;
	//virtual void OnPaint(const PaintEvent& e) override;
	//virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) override;
	//virtual void OnLButtonEndDrag(const LButtonEndDragEvent& e) override;

	//virtual CRectF GetSplitterRect() const = 0;

	//NLOHMANN_DEFINE_TYPE_INTRUSIVE_NOTHROW(CSplitContainer,
	//	Maximum,
	//	Minimum,
	//	Value)
};

class CVerticalSplitContainer:public CSplitContainer
{
public:
	CVerticalSplitContainer(CD2DWControl* pParentControl = nullptr);
	virtual ~CVerticalSplitContainer() = default;

	const std::shared_ptr<CD2DWControl>& GetLeft() const { return m_one; }
	const std::shared_ptr<CD2DWControl>& GetRight() const { return m_two; }
	void SetLeft(const std::shared_ptr<CD2DWControl>& left) { m_one = left; }
	void SetRight(const std::shared_ptr<CD2DWControl>& right) { m_two = right; }

	//CSplitContainer
	//virtual CRectF GetSplitterRect() const override;


	//MeasureArrange
	virtual CSizeF MeasureOverride(const CSizeF& availableSize) override;
	virtual void ArrangeOverride(const CRectF& rc) override;


	//Event
	//virtual void OnMouseMove(const MouseMoveEvent& e) override;
	//virtual void OnSetCursor(const SetCursorEvent& e) override;
};

class CHorizontalSplitContainer:public CSplitContainer
{
public:
	CHorizontalSplitContainer(CD2DWControl* pParentControl = nullptr);
	virtual ~CHorizontalSplitContainer() = default;

	const std::shared_ptr<CD2DWControl>& GetTop() const { return m_one; }
	const std::shared_ptr<CD2DWControl>& GetBottom() const { return m_two; }
	void SetTop(const std::shared_ptr<CD2DWControl>& top) { m_one = top; }
	void SetBottom(const std::shared_ptr<CD2DWControl>& bottom) { m_two = bottom; }


	//CSplitContainer
	//virtual CRectF GetSplitterRect() const override;


	//MeasureArrange
	virtual CSizeF MeasureOverride(const CSizeF& availableSize) override;
	virtual void ArrangeOverride(const CRectF& rc) override;


	//Event
	//virtual void OnMouseMove(const MouseMoveEvent& e) override;
	//virtual void OnSetCursor(const SetCursorEvent& e) override;
};

