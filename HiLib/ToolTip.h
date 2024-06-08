#pragma once
#include "D2DWControl.h"
#include "reactive_string.h"

struct ToolTipProperty
{
	FormatF Format = FormatF(L"Meiryo UI", CDirect2DWrite::Points2Dips(9), 0.0f, 0.0f, 0.0f, 1.0f);
	SolidLine Line = SolidLine(30.f / 255.f, 30.f / 255.f, 30.f / 255.f, 1.0f, 1.0f);
	SolidFill BackgroundFill = SolidFill(1.f, 246.f / 255.f, 246.f / 255.f, 1.0f);
	CRectF Padding = CRectF(3.f, 3.f, 3.f, 3.f);
	FLOAT Radius = 3.f;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(
		ToolTipProperty,
		Format,
		Line,
		BackgroundFill,
		Padding)
};

class CToolTip :public CD2DWControl
{
private:
	std::shared_ptr<ToolTipProperty> m_spProp;
public:
	reactive_wstring_ptr Content;

	CToolTip(CD2DWControl* pParentControl, const std::shared_ptr<ToolTipProperty>& spProp);
	virtual ~CToolTip(){}

	void Measure(const CSizeF& availableSize) override;	
	//void OnCreate(const CreateEvt& e) override;
	void OnDestroy(const DestroyEvent& e) override;
	void OnPaint(const PaintEvent& e) override;
};