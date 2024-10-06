#pragma once

#include "D2DWControl.h"

class CToolBar: public CD2DWControl
{
public:
	CToolBar(CD2DWControl* pParentControl = nullptr);

	CSizeF MeasureOverride(const CSizeF& availableSize) override;
	void ArrangeOverride(const CRectF& finalRect) override;

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

public:
	friend void to_json(json& j, const CToolBar& o)
	{
		to_json(j, static_cast<const CD2DWControl&>(o));
		json_safe_to(j, "Children", o.m_childControls);
	}

	friend void from_json(const json& j, CToolBar& o)
	{
		from_json(j, static_cast<CD2DWControl&>(o));
		json_safe_from(j, "Children", o.m_childControls);
	}
};

JSON_ENTRY_TYPE(CD2DWControl, CToolBar)
