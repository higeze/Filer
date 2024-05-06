//#pragma once
//
//#include "Direct2DWrite.h"
//#include "ScrollProperty.h"
//#include "JsonSerializer.h"
//
//struct TextBoxProperty
//{
//public:
//	//std::shared_ptr<FormatF> Format;
//	//std::shared_ptr<SolidLine> Line;
//	std::shared_ptr<SolidLine> CompositionLine;
//	std::shared_ptr<SolidLine> FocusedLine;
//	std::shared_ptr<SolidLine> BlankLine;
//	std::shared_ptr<SolidFill> NormalFill;
//	std::shared_ptr<SolidFill> FocusedFill;
//	std::shared_ptr<SolidFill> SelectedFill;
//	std::shared_ptr<SolidFill> UnfocusSelectedFill;
//	std::shared_ptr<SolidFill> HotFill;
//	std::shared_ptr<CRectF> Padding; 
//	std::shared_ptr<ScrollProperty> VScrollPropPtr;
//	std::shared_ptr<ScrollProperty> HScrollPropPtr;
//	bool IsWrap = true;
//
//
//public:
//	TextBoxProperty()
//		:Format(std::make_shared<FormatF>(L"Meiryo UI", CDirect2DWrite::Points2Dips(9),  0.0f, 0.0f, 0.0f, 1.0f)),
//		Line(std::make_shared<SolidLine>(221.f/255.f, 206.f/255.f, 188.f/255.f, 1.0f, 1.0f)),
//		CompositionLine(std::make_shared<SolidLine>(0.0f/255.f, 0.0f/255.f, 255.f/255.f, 1.0f, 1.0f)),
//		FocusedLine(std::make_shared<SolidLine>(22.f/255.f, 160.f/255.f, 133.f/255.f, 1.0f, 1.0f)),
//		BlankLine(std::make_shared<SolidLine>(100.f / 255.f, 100.f / 255.f, 100.f / 255.f, 0.6f, 1.0f)),
//		NormalFill(std::make_shared<SolidFill>(246.f/255.f, 246.f/255.f, 246.f/255.f, 1.0f)),
//		FocusedFill(std::make_shared<SolidFill>(0.0f/255.f, 224.f/255.f, 226.f/255.f, 1.0f)),
//		SelectedFill(std::make_shared<SolidFill>(0.f, 140.f / 255, 255.f / 255, 100.f / 255)),
//		UnfocusSelectedFill(std::make_shared<SolidFill>(224.f/255.f, 224.f/255.f, 224.f/255.f, 0.5f)),
//		HotFill(std::make_shared<SolidFill>(1.0f, 1.0f, 1.0f, 0.3f)),
//		Padding(std::make_shared<CRectF>(2.0f,2.0f,2.0f,2.0f)),
//		VScrollPropPtr(std::make_shared<ScrollProperty>()),
//		HScrollPropPtr(std::make_shared<ScrollProperty>()),
//		IsWrap(true){};
//
//	TextBoxProperty(const std::shared_ptr<ScrollProperty>& spVScrollProp, const std::shared_ptr<ScrollProperty>& spHScrollProp)
//		:Format(std::make_shared<FormatF>(L"Meiryo UI", CDirect2DWrite::Points2Dips(9),  0.0f, 0.0f, 0.0f, 1.0f)),
//		Line(std::make_shared<SolidLine>(221.f/255.f, 206.f/255.f, 188.f/255.f, 1.0f, 1.0f)),
//		CompositionLine(std::make_shared<SolidLine>(0.0f/255.f, 0.0f/255.f, 255.f/255.f, 1.0f, 1.0f)),
//		FocusedLine(std::make_shared<SolidLine>(22.f/255.f, 160.f/255.f, 133.f/255.f, 1.0f, 1.0f)),
//		BlankLine(std::make_shared<SolidLine>(100.f / 255.f, 100.f / 255.f, 100.f / 255.f, 0.6f, 1.0f)),
//		NormalFill(std::make_shared<SolidFill>(246.f/255.f, 246.f/255.f, 246.f/255.f, 1.0f)),
//		FocusedFill(std::make_shared<SolidFill>(0.0f/255.f, 224.f/255.f, 226.f/255.f, 1.0f)),
//		SelectedFill(std::make_shared<SolidFill>(150.f/255.f, 200.f/255.f, 240.f/255.f, 0.5f)),
//		UnfocusSelectedFill(std::make_shared<SolidFill>(224.f/255.f, 224.f/255.f, 224.f/255.f, 0.5f)),
//		HotFill(std::make_shared<SolidFill>(1.0f, 1.0f, 1.0f, 0.3f)),
//		Padding(std::make_shared<CRectF>(2.0f,2.0f,2.0f,2.0f)),
//		IsWrap(true),
//		VScrollPropPtr(spVScrollProp),
//		HScrollPropPtr(spHScrollProp){};
//
//	TextBoxProperty(
//		std::shared_ptr<FormatF> fontAndColor,
//		std::shared_ptr<SolidLine> line,
//		std::shared_ptr<SolidLine> editLine,
//		std::shared_ptr<SolidLine> focusedLine,
//		std::shared_ptr<SolidFill> normalFill,
//		std::shared_ptr<SolidFill> focusedFill,
//		std::shared_ptr<SolidFill> selectedFill,
//		std::shared_ptr<SolidFill> unfocusSelectedFill,
//		std::shared_ptr<SolidFill> hotFill,
//		std::shared_ptr<CRectF> padding)
//		:Format(fontAndColor),
//		Line(line),
//		CompositionLine(editLine),
//		FocusedLine(focusedLine),
//		NormalFill(normalFill),
//		FocusedFill(focusedFill),
//		SelectedFill(selectedFill),
//		UnfocusSelectedFill(unfocusSelectedFill),
//		HotFill(hotFill),
//		Padding(padding),
//		BlankLine(std::make_shared<SolidLine>(100.f / 255.f, 100.f / 255.f, 100.f / 255.f, 0.6f, 1.0f)),
//		VScrollPropPtr(std::make_shared<ScrollProperty>()),
//		HScrollPropPtr(std::make_shared<ScrollProperty>()),
//		IsWrap(true){};
//
//	virtual ~TextBoxProperty() = default;
//
//	template <class Archive>
//    void save(Archive& ar)
//    {
//		ar("Format",Format);
//		ar("Line",Line);
//		ar("CompositionLine", CompositionLine);
//		ar("FocusedLine",FocusedLine);
//		ar("BlankLine", BlankLine);
//		ar("NormalFill",NormalFill);
//		ar("FocusedFill",FocusedFill);
//		ar("SelectedFill",SelectedFill);
//		ar("UnfocusSelectedFill", UnfocusSelectedFill);
//		ar("HotFill", HotFill);
//		ar("Padding",Padding);
//		ar("VScrollProperty", VScrollPropPtr);
//		ar("HScrollProperty", HScrollPropPtr);
//		ar("IsWrap", IsWrap);
//	}
//
//	template <class Archive>
//    void load(Archive& ar)
//    {
//		ar("Format",Format);
//		ar("Line",Line);
//		ar("CompositionLine", CompositionLine);
//		ar("FocusedLine",FocusedLine);
//		ar("BlankLine", BlankLine);
//		ar("NormalFill",NormalFill);
//		ar("FocusedFill",FocusedFill);
//		ar("SelectedFill",SelectedFill);
//		ar("UnfocusSelectedFill", UnfocusSelectedFill);
//		ar("HotFill", HotFill);
//		ar("Padding",Padding);
//		ar("VScrollProperty", VScrollPropPtr);
//		ar("HScrollProperty", HScrollPropPtr);
//		ar("IsWrap", IsWrap);
//	}
//
//	NLOHMANN_DEFINE_TYPE_INTRUSIVE(TextBoxProperty,
//		Format,
//		Line,
//		CompositionLine,
//		FocusedLine,
//		BlankLine,
//		NormalFill,
//		FocusedFill,
//		SelectedFill,
//		UnfocusSelectedFill,
//		HotFill,
//		Padding,
//		VScrollPropPtr,
//		HScrollPropPtr,
//		IsWrap)
//};
//
