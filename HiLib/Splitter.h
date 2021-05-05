#pragma once
#include "D2DWControl.h"
#include "ReactiveProperty.h"
#include "JsonSerializer.h"

struct SplitterProperty
{
public:
	SolidFill BackgroundFill = SolidFill(222.f / 255.f, 222.f / 255.f, 222.f / 255.f, 1.0f);
	FLOAT Width = 5.f;

	SplitterProperty() {}

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar("BackgroundFill", BackgroundFill);
		ar("Width", Width);
	}

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(SplitterProperty,
		BackgroundFill,
		Width)
};

class CHorizontalSplitter:public CD2DWControl
{
private:
	std::shared_ptr<SplitterProperty> m_spSplitterProperty;
	ReactiveProperty<FLOAT> m_splitterLeft;

	CD2DWControl* m_pLeftControl;
	CD2DWControl* m_pRightControl;
	bool m_inDrag = false;
	CPointF m_ptBeginDrag;
public:
	CHorizontalSplitter(CD2DWControl* pParentControl = nullptr, CD2DWControl* pLeftControl = nullptr, CD2DWControl* pRightControl = nullptr, const std::shared_ptr<SplitterProperty>& spButtonProperty = nullptr)
		:CD2DWControl(pParentControl), m_pLeftControl(pLeftControl), m_pRightControl(pRightControl), m_spSplitterProperty(spButtonProperty){}
	virtual ~CHorizontalSplitter() = default;

	ReactiveProperty<FLOAT>& GetSplitterLeft() { return m_splitterLeft; }
	void SetLeftRightControl(CD2DWControl* pLeft, CD2DWControl* pRight)
	{
		m_pLeftControl = pLeft;
		m_pRightControl = pRight;
	}
	void SetProperty(const std::shared_ptr<SplitterProperty>& pProp)
	{
		m_spSplitterProperty = pProp;
	}

	//Event
	virtual void OnPaint(const PaintEvent& e) override;
	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) override;
	virtual void OnLButtonEndDrag(const LButtonEndDragEvent& e) override;
	virtual void OnMouseMove(const MouseMoveEvent& e) override;
	virtual void OnSetCursor(const SetCursorEvent& e) override;

    template <class Archive>
	void serialize(Archive& ar) {}

	friend void to_json(json& j, const CHorizontalSplitter& o) {}
	friend void from_json(const json& j, CHorizontalSplitter& o) {}
};


