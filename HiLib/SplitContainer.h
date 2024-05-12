#pragma once
#include "D2DWControl.h"
#include "reactive_property.h"
#include "JsonSerializer.h"

class CSplitter;

class CSplitContainer : public CD2DWControl
{
protected:
	std::shared_ptr<CD2DWControl> m_one;
	std::shared_ptr<CD2DWControl> m_two;
	std::shared_ptr<CSplitter> m_splitter;
protected:
	bool m_inDrag = false;
	CPointF m_ptBeginDrag;
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
	virtual void Measure(const CSizeF& availableSize) override;
	virtual void Arrange(const CRectF& rc) override;


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
	virtual void Measure(const CSizeF& availableSize) override;
	virtual void Arrange(const CRectF& rc) override;


	//Event
	//virtual void OnMouseMove(const MouseMoveEvent& e) override;
	//virtual void OnSetCursor(const SetCursorEvent& e) override;
};

