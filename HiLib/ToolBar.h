#pragma once

#include "D2DWControl.h"

class CToolBar: public CD2DWControl
{
public:
	CToolBar(CD2DWControl* pParentControl = nullptr);
	void Measure(const CSizeF& availableSize) override;
	void Arrange(const CRectF& rc) override;

	void OnCreate(const CreateEvt& e) override;
	void OnPaint(const PaintEvent& e) override;

	//Add
	void Add() {}

	template<class _Head>
	void Add(_Head&& child)
	{
		AddChildControlPtr(std::forward<_Head>(child));
	}

	template<class _Head, class... _Tail>
	void Add(_Head&& head, _Tail&&... tail)
	{
		Add(std::forward<_Head>(head));
		Add(std::forward<_Tail>(tail)...);
	}

    template<class Archive>
    void save(Archive & archive) const
    {
		archive(cereal::base_class<CD2DWControl>(this));
        //archive(cereal::make_nvp("Children", m_childControls));
    }

    template<class Archive>
    void load(Archive & archive)
    {
		archive(cereal::base_class<CD2DWControl>(this));
        //archive(cereal::make_nvp("Children", m_childControls));
    }

	friend void to_json(json& j, const CToolBar& o)
	{
		to_json(j, static_cast<const CD2DWControl&>(o));

		j["Children"] = o.m_childControls;
	}

	friend void from_json(const json& j, CToolBar& o)
	{
		from_json(j, static_cast<CD2DWControl&>(o));

		get_to(j, "Children", o.m_childControls);
	}
};