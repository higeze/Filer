#pragma once
#include "D2DWControl.h"
#include "reactive_property.h"
#include "JsonSerializer.h"

class CSplitter : public CD2DWControl
{
protected:
	std::optional<CPointF> m_ptBeginDrag;
public:

	virtual const CRectF& GetMargin() const override
	{ 
		static const CRectF value(0.f, 0.f, 0.f, 0.f); return value; 
	}
	virtual const CRectF& GetPadding() const override
	{ 
		static const CRectF value(0.f, 0.f, 0.f, 0.f); return value; 
	}
	const SolidFill& GetNormalBackground() const override
	{
		static const SolidFill value(222.f / 255.f, 222.f / 255.f, 222.f / 255.f, 1.0f); return value;
	}

public:
	reactive_property_ptr<FLOAT> Maximum;
	reactive_property_ptr<FLOAT> Minimum;
	reactive_property_ptr<FLOAT> Value;
	FLOAT Width = 5.f;

public:
	CSplitter(CD2DWControl* pParentControl = nullptr);
	virtual ~CSplitter() = default;

	//Event
	virtual void OnCreate(const CreateEvt& e) override;
	virtual void OnPaint(const PaintEvent& e) override;
	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) override;
	virtual void OnLButtonEndDrag(const LButtonEndDragEvent& e) override;

	friend void to_json(json& j, const CSplitter& o)
	{
		to_json(j, static_cast<const CD2DWControl&>(o));

		j["Value"] = o.Value;
	}

	friend void from_json(const json& j, CSplitter& o)
	{
		from_json(j, static_cast<CD2DWControl&>(o));

		get_to(j, "Value", o.Value);
	}
	
};

class CVerticalSplitter:public CSplitter
{
public:
	using CSplitter::CSplitter;
	virtual ~CVerticalSplitter() = default;

	//MeasureArrange
	virtual void Measure(const CSizeF& availableSize) override
	{
		m_size.width = Width;
		//m_size.height = availableSize.height;
	}

	virtual void Arrange(const CRectF& rc) override
	{
		CD2DWControl::Arrange(rc);
	}
	
	//Event
	virtual void OnMouseMove(const MouseMoveEvent& e) override;
	virtual void OnSetCursor(const SetCursorEvent& e) override;
};

class CHorizontalSplitter:public CSplitter
{
public:
	using CSplitter::CSplitter;
	virtual ~CHorizontalSplitter() = default;

	//MeasureArrange
	virtual void Measure(const CSizeF& availableSize) override
	{
		//m_size.width = availableSize.width;
		m_size.height = Width;
	}

	virtual void Arrange(const CRectF& rc) 
	{
		CD2DWControl::Arrange(rc);
	}

	//Event
	virtual void OnMouseMove(const MouseMoveEvent& e) override;
	virtual void OnSetCursor(const SetCursorEvent& e) override;
};


