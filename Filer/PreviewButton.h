#pragma once
#include "Button.h"
#include "reactive_property.h"

class CPreviewButton : public CButton
{
public:
	reactive_property_ptr<bool> IsPreview;
public:
	using CButton::CButton;

	void OnCreate(const CreateEvt& e)
	{
		CD2DWControl::OnCreate(e);

		Content.set(*IsPreview ? L"Preview" : L"Normal");
		Command.subscribe([this]() {
			IsPreview.set(!*IsPreview);
			Content.set(*IsPreview ? L"Preview" : L"Normal");
		}, shared_from_this());
	}

    template<class Archive>
    void save(Archive & archive) const
    {
		archive(cereal::base_class<CD2DWControl>(this));
    }
    template<class Archive>
    void load(Archive & archive)
    {
		archive(cereal::base_class<CD2DWControl>(this));
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
