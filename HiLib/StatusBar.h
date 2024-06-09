#pragma once
#include <cereal/cereal.hpp>
#include "Direct2DWrite.h"
#include "D2DWControl.h"
#include "reactive_property.h"

/**************/
/* CStatusBar */
/**************/
class CStatusBar : public CD2DWControl
{
public:
	reactive_wstring_ptr Text;
public:
	const SolidFill& GetNormalBackground() const override
	{
		static const SolidFill value(222.f / 255.f, 222.f / 255.f, 222.f / 255.f, 1.0f); return value;
	}
	virtual const FormatF& GetFormat() const
	{
		static FormatF value(L"Meiryo UI", CDirect2DWrite::Points2Dips(9), 1.0f, 1.0f, 1.0f, 1.0f); return value;
	}
public:
	CStatusBar(CD2DWControl* pParentControl = nullptr);
	virtual ~CStatusBar();

	//MeasureArrange
	virtual CSizeF MeasureSize(const std::wstring& text);
	virtual void Measure(const CSizeF& availableSize) override;

	virtual void OnPaint(const PaintEvent& e);

	template<class Archive>
	void save(Archive& archive) const
	{
		archive(cereal::base_class<CD2DWControl>(this));
	}
	template<class Archive>
	void load(Archive& archive)
	{
		archive(cereal::base_class<CD2DWControl>(this));
	}

};