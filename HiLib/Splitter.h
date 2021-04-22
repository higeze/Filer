#pragma once
#include "D2DWControl.h"
#include "ReactiveProperty.h"
#include "JsonSerializer.h"

struct SplitterProperty
{
public:
	SolidFill BackgroundFill = SolidFill(222.f / 255.f, 222.f / 255.f, 222.f / 255.f, 1.0f);
	FLOAT Width = 5.f;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar("BackgroundFill", BackgroundFill);
		ar("Width", Width);
	}

    friend void to_json(json& j, const SplitterProperty& o);
    friend void from_json(const json& j, SplitterProperty& o);
};

void to_json(json& j, const SplitterProperty& o)
{
	j = json{
		{"BackgroundFill", o.BackgroundFill},
		{"Width", o.Width}
	};
}
void from_json(const json& j, SplitterProperty& o)
{
	j.at("BackgroundFill").get_to(o.BackgroundFill);
	j.at("Width").get_to(o.Width);
}



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
	CHorizontalSplitter(CD2DWControl* pParentControl, CD2DWControl* pLeftControl, CD2DWControl* pRightControl, const std::shared_ptr<SplitterProperty>& spButtonProperty)
		:CD2DWControl(pParentControl), m_pLeftControl(pLeftControl), m_pRightControl(pRightControl), m_spSplitterProperty(spButtonProperty){}
	virtual ~CHorizontalSplitter() = default;

	ReactiveProperty<FLOAT>& GetSplitterLeft() { return m_splitterLeft; }

	//Event
	virtual void OnPaint(const PaintEvent& e) override;
	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) override;
	virtual void OnLButtonEndDrag(const LButtonEndDragEvent& e) override;
	virtual void OnMouseMove(const MouseMoveEvent& e) override;
	virtual void OnSetCursor(const SetCursorEvent& e) override;

    template <class Archive>
	void serialize(Archive& ar) {}

    friend void to_json(json& j, const CHorizontalSplitter& o);
    friend void from_json(const json& j, CHorizontalSplitter& o);
};

void to_json(json& j, const CHorizontalSplitter& o){}
void from_json(const json& j, CHorizontalSplitter& o){}



