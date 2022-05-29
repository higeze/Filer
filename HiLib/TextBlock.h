#pragma once
#include "D2DWControl.h"
#include "ReactiveProperty.h"

struct TextBlockProperty
{
public:
	FormatF Format = FormatF(L"Meiryo UI", CDirect2DWrite::Points2Dips(9), 0.0f, 0.0f, 0.0f, 1.0f);
	FormatF DisableFormat = FormatF(L"Meiryo UI", CDirect2DWrite::Points2Dips(9), 200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 1.0f);

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar("Format", Format);
	}
};


class CTextBlock:public CD2DWControl
{
private:
	std::shared_ptr<TextBlockProperty> m_spProperty;
	ReactiveProperty<std::wstring> m_text;
public:
	CTextBlock(CD2DWControl* pControl, const std::shared_ptr<TextBlockProperty>& spProperty)
		:CD2DWControl(pControl), m_spProperty(spProperty){}
	virtual ~CTextBlock() = default;
	//
	ReactiveProperty<std::wstring>& GetText() { return m_text; }

	//Event
	virtual void OnPaint(const PaintEvent& e) override;
};