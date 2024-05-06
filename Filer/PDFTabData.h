#pragma once
#include "TabControl.h"
#include "PDFDoc.h"

#include "reactive_property.h"
#include "reactive_string.h"
#include "reactive_command.h"


/**************/
/* PdfTabData */
/**************/
struct PdfTabData :public TabData
{
	reactive_property_ptr<std::shared_ptr<CPDFDoc>> Doc;
	reactive_property_ptr<FLOAT> VScroll;
	reactive_property_ptr<FLOAT> HScroll;
	reactive_property_ptr<FLOAT> Scale;
	
	PdfTabData(const std::wstring& path = std::wstring(), const std::wstring password = std::wstring())
		:TabData(),
		Doc(std::make_shared<CPDFDoc>()),
		VScroll(0.f),
		HScroll(0.f),
		Scale(-1.f)
	{
		Doc.get_unconst()->Path.set(path);
		Doc.get_unconst()->Password.set(password);
		Doc.get_unconst()->Load(path);
	}

	virtual ~PdfTabData() = default;

	PdfTabData(const PdfTabData& other)
		:PdfTabData(other.Doc->GetPath(), *other.Doc->Password) {}

	virtual std::shared_ptr<TabData> ClonePtr() const override { return std::make_shared<PdfTabData>(*this); }

	virtual bool AcceptClosing(CD2DWWindow* pWnd, bool isWndClosing) override;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_NOTHROW(
		PdfTabData,
		Doc)
};

