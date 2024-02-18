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
	reactive_property_ptr<CPDFDoc> Doc;
	reactive_property_ptr<FLOAT> VScroll;
	reactive_property_ptr<FLOAT> HScroll;
	reactive_property_ptr<FLOAT> Scale;
	
	PdfTabData(const std::wstring& path = std::wstring())
		:TabData(),
		Doc(),
		VScroll(0.f),
		HScroll(0.f),
		Scale(-1.f)
	{
		Doc.get_unconst()->Path.set(path);
		Doc.get_unconst()->Load(path);
	}

	virtual ~PdfTabData() = default;

	virtual bool AcceptClosing(CD2DWWindow* pWnd, bool isWndClosing) override;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_NOTHROW(
		PdfTabData,
		Doc)
};

