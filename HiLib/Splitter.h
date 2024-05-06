#pragma once
#include "D2DWControl.h"
#include "reactive_property.h"
#include "JsonSerializer.h"

class CSplitter : public CD2DWControl
{
protected:
	bool m_inDrag = false;
	CPointF m_ptBeginDrag;
public:

	virtual const CRectF& GetMargin() const override
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

public:
	CSplitter(CD2DWControl* pParentControl = nullptr)
		:CD2DWControl(pParentControl), Minimum(-1.f), Maximum(-1.f), Value(-1.f){}
	virtual ~CSplitter() = default;

	//Event
	virtual void OnPaint(const PaintEvent& e) override;
	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) override;
	virtual void OnLButtonEndDrag(const LButtonEndDragEvent& e) override;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_NOTHROW(CSplitter,
		Maximum,
		Minimum,
		Value)
	
};

class CHorizontalSplitter:public CSplitter
{
private:
	FLOAT Width = 5.f;
public:
	using CSplitter::CSplitter;
	virtual ~CHorizontalSplitter() = default;

	//MeasureArrange
	void Measure(const CSizeF& availableSize)
	{
		m_size.width = Width;
		m_size.height = availableSize.height;
	}
	
	//Event
	virtual void OnMouseMove(const MouseMoveEvent& e) override;
	virtual void OnSetCursor(const SetCursorEvent& e) override;
};

class CVerticalSplitter:public CSplitter
{
private:
	FLOAT Height = 5.f;
public:
	using CSplitter::CSplitter;
	virtual ~CVerticalSplitter() = default;

	//MeasureArrange
	void Measure(const CSizeF& availableSize)
	{
		m_size.width = availableSize.width;
		m_size.height = Height;
	}

	//Event
	virtual void OnMouseMove(const MouseMoveEvent& e) override;
	virtual void OnSetCursor(const SetCursorEvent& e) override;
};


