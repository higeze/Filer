#pragma once
#include "Button.h"
#include "reactive_property.h"

class CPreviewButton : public CButton
{
public:
	reactive_property_ptr<bool> IsPreview;
public:
	CPreviewButton(CD2DWControl* pParentControl = nullptr)
		:CButton(pParentControl)
	{
		Content.set(*IsPreview ? L"Preview" : L"Normal");
		Command.subscribe([this]() {
			IsPreview.set(!*IsPreview);
			Content.set(*IsPreview ? L"Preview" : L"Normal");
		}, Dummy);
	}

	friend void to_json(json& j, const CPreviewButton& o)
	{
		to_json(j, static_cast<const CD2DWControl&>(o));
	}

	friend void from_json(const json& j, CPreviewButton& o)
	{
		from_json(j, static_cast<CD2DWControl&>(o));
	}
};
