#pragma once
#include <memory>
#include "Direct2DWrite.h"
#include "ScrollProperty.h"

//struct PdfViewProperty
//{
//public:
//	std::shared_ptr<FormatF> Format;
//	std::shared_ptr<SolidLine> FocusedLine;
//	std::shared_ptr<SolidFill> NormalFill;
//	std::shared_ptr<SolidFill> SelectedFill;
//	std::shared_ptr<SolidFill> UnfocusSelectedFill;
//	//std::shared_ptr<SolidFill> HotFill;
//	std::shared_ptr<SolidFill> FindHighliteFill;
//
//	std::shared_ptr<CRectF> Padding;
//	//std::shared_ptr<ScrollProperty> VScrollPropPtr;
//	//std::shared_ptr<ScrollProperty> HScrollPropPtr;
//
//
//public:
//	PdfViewProperty()
//		:Format(std::make_shared<FormatF>(L"Meiryo UI", CDirect2DWrite::Points2Dips(9),  0.0f, 0.0f, 0.0f, 1.0f)),
//		FocusedLine(std::make_shared<SolidLine>(22.f/255.f, 160.f/255.f, 133.f/255.f, 1.0f, 1.0f)),
//		NormalFill(std::make_shared<SolidFill>(246.f/255.f, 246.f/255.f, 246.f/255.f, 1.0f)),
//		SelectedFill(std::make_shared<SolidFill>(0.f, 140.f / 255, 255.f / 255, 100.f / 255)),
//		UnfocusSelectedFill(std::make_shared<SolidFill>(224.f/255.f, 224.f/255.f, 224.f/255.f, 0.5f)),
//		FindHighliteFill(std::make_shared<SolidFill>(244.f / 255, 167.f / 255, 33.f / 255, 100.f / 255)),
//		Padding(std::make_shared<CRectF>(2.0f,2.0f,2.0f,2.0f)),
//		VScrollPropPtr(std::make_shared<ScrollProperty>()),
//		HScrollPropPtr(std::make_shared<ScrollProperty>()){};
//
//	virtual ~PdfViewProperty() = default;
//
//	NLOHMANN_DEFINE_TYPE_INTRUSIVE(PdfViewProperty,
//		FocusedLine,
//		NormalFill,
//		SelectedFill,
//		UnfocusSelectedFill,
//		Padding,
//		FindHighliteFill,
//		VScrollPropPtr,
//		HScrollPropPtr)
//};

