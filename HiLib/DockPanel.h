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
	virtual void ArrangeOverride(const CRectF& finalRect) override;
	//Add
	void Add() {}
	void Add(std::pair<std::shared_ptr<CD2DWControl>, DockEnum>&& p)
	{
		p.first->Dock.set(p.second);
		AddChildControlPtr(p.first);
	}
	template<class _Head, class... _Tail>
	void Add(_Head&& head, _Tail&&... tail)
	{
		Add(std::forward<_Head>(head));
		Add(std::forward<_Tail>(tail)...);
	}
public:

	friend void to_json(json& j, const CDockPanel& o)
	{
		to_json(j, static_cast<const CD2DWControl&>(o));

		json_safe_to(j, "Children", o.m_childControls);
	}

	friend void from_json(const json& j, CDockPanel& o)
	{
		from_json(j, static_cast<CD2DWControl&>(o));

		json_safe_from(j, "Children", o.m_childControls);
	}

};

JSON_ENTRY_TYPE(CD2DWControl, CDockPanel)

