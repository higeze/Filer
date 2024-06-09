#pragma once
#include <cereal/cereal.hpp>
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
	virtual void Measure(const CSizeF& availableSize) override;
	virtual void Arrange(const CRectF& rc) override;
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
    template<class Archive>
    void save(Archive & archive) const
    {
		archive(
			cereal::base_class<CD2DWControl>(this),
			cereal::make_nvp("Children", m_childControls));
    }
    template<class Archive>
    void load(Archive & archive)
    {
		archive(
			cereal::base_class<CD2DWControl>(this),
			cereal::make_nvp("Children", m_childControls));
    }

	friend void to_json(json& j, const CDockPanel& o)
	{
		to_json(j, static_cast<const CD2DWControl&>(o));

		j["Children"] = o.m_childControls;
	}

	friend void from_json(const json& j, CDockPanel& o)
	{
		from_json(j, static_cast<CD2DWControl&>(o));

		get_to(j, "Children", o.m_childControls);
	}

};
