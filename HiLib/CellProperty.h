#pragma once
#include "Direct2DWrite.h"

enum class PicturePositon
{
	Fill,
	Fit,
	Stretch,
	Tile,
	Center
};

class BackgroundProperty
{
public:
	std::shared_ptr<d2dw::SolidFill> m_brush;
	std::shared_ptr<bool> m_usePicture;
	std::shared_ptr<std::wstring> m_picturePath;
	std::shared_ptr<PicturePositon> m_picturePosition;

	BackgroundProperty()
		:m_brush(std::make_shared<d2dw::SolidFill>(246.0f/255, 246.0f/255, 246.0f/255, 1.0f)),//asbestos
		m_usePicture(std::make_shared<bool>(false)),
		m_picturePath(std::make_shared<std::wstring>()),
		m_picturePosition(std::make_shared<PicturePositon>(PicturePositon::Center)){}
	virtual ~BackgroundProperty() = default;

    template <class Archive>
    void serialize(Archive& ar)
    {
		ar("BackgroundBrush",m_brush);
		ar("UsePicture",m_usePicture);
		ar("PicturePath",m_picturePath);
		ar("PicturePosition",m_picturePosition);
    }
};

class CellProperty
{
public:
	std::shared_ptr<d2dw::FormatF> Format;
	std::shared_ptr<d2dw::SolidLine> Line;
	std::shared_ptr<d2dw::SolidLine> EditLine;
	std::shared_ptr<d2dw::SolidLine> FocusedLine;
	std::shared_ptr<d2dw::SolidFill> NormalFill;
	std::shared_ptr<d2dw::SolidFill> FocusedFill;
	std::shared_ptr<d2dw::SolidFill> SelectedFill;
	std::shared_ptr<d2dw::SolidFill> UnfocusSelectedFill;
	std::shared_ptr<d2dw::SolidFill> HotFill;
	std::shared_ptr<d2dw::CRectF> Padding;

public:
	CellProperty()
		:Format(std::make_shared<d2dw::FormatF>(L"Meiryo UI", d2dw::CDirect2DWrite::Points2Dips(9),  0.0f, 0.0f, 0.0f, 1.0f)),
		Line(std::make_shared<d2dw::SolidLine>(221.f/255.f, 206.f/255.f, 188.f/255.f, 1.0f, 1.0f)),
		EditLine(std::make_shared<d2dw::SolidLine>(0.0f/255.f, 0.0f/255.f, 255.f/255.f, 1.0f, 1.0f)),
		FocusedLine(std::make_shared<d2dw::SolidLine>(22.f/255.f, 160.f/255.f, 133.f/255.f, 1.0f, 1.0f)),
		NormalFill(std::make_shared<d2dw::SolidFill>(246.f/255.f, 246.f/255.f, 246.f/255.f, 1.0f)),
		FocusedFill(std::make_shared<d2dw::SolidFill>(0.0f/255.f, 224.f/255.f, 226.f/255.f, 1.0f)),
		SelectedFill(std::make_shared<d2dw::SolidFill>(150.f/255.f, 200.f/255.f, 240.f/255.f, 0.5f)),
		UnfocusSelectedFill(std::make_shared<d2dw::SolidFill>(224.f/255.f, 224.f/255.f, 224.f/255.f, 0.5f)),
		HotFill(std::make_shared<d2dw::SolidFill>(1.0f, 1.0f, 1.0f, 0.3f)),
		Padding(std::make_shared<d2dw::CRectF>(2.0f,2.0f,2.0f,2.0f)){};

	CellProperty(
		std::shared_ptr<d2dw::FormatF> fontAndColor,
		std::shared_ptr<d2dw::SolidLine> line,
		std::shared_ptr<d2dw::SolidLine> editLine,
		std::shared_ptr<d2dw::SolidLine> focusedLine,
		std::shared_ptr<d2dw::SolidFill> normalFill,
		std::shared_ptr<d2dw::SolidFill> focusedFill,
		std::shared_ptr<d2dw::SolidFill> selectedFill,
		std::shared_ptr<d2dw::SolidFill> unfocusSelectedFill,
		std::shared_ptr<d2dw::SolidFill> hotFill,
		std::shared_ptr<d2dw::CRectF> padding)
		:Format(fontAndColor),
		Line(line),
		EditLine(editLine),
		FocusedLine(focusedLine),
		NormalFill(normalFill),
		FocusedFill(focusedFill),
		SelectedFill(selectedFill),
		UnfocusSelectedFill(unfocusSelectedFill),
		HotFill(hotFill),

		Padding(padding){};

	virtual ~CellProperty() = default;

	template <class Archive>
    void serialize(Archive& ar)
    {
		ar("Format",Format);
		ar("Line",Line);
		ar("EditLine", EditLine);
		ar("FocusedLine",FocusedLine);
		ar("NormalFill",NormalFill);
		ar("FocusedFill",FocusedFill);
		ar("SelectedFill",SelectedFill);
		ar("UnfocusSelectedFill", UnfocusSelectedFill);
		ar("HotFill", HotFill);
		ar("Padding",Padding);
    }

};

struct HeaderProperty:public CellProperty
{
public:
	std::shared_ptr<d2dw::SolidLine> DragLine;
public:
	HeaderProperty()
		:CellProperty(
		std::make_shared<d2dw::FormatF>(L"Meiryo UI", d2dw::CDirect2DWrite::Points2Dips(9), 0.0f, 0.0f, 0.0f, 1.0f),
		std::make_shared<d2dw::SolidLine>(211.f / 255.f, 206.f / 255.f, 188.f / 255.f, 1.0f, 1.0f),
		std::make_shared<d2dw::SolidLine>(0.f / 255.f, 0.f / 255.f, 255.f / 255.f, 1.0f, 1.0f),
		std::make_shared<d2dw::SolidLine>(22.f / 255.f, 160.f / 255, 133.f / 255.f, 1.0f, 1.0f),
		std::make_shared<d2dw::SolidFill>(239.f / 255.f, 239.f / 255, 232.f / 255.f, 1.0f),
		std::make_shared<d2dw::SolidFill>(195.f / 255.f, 224.f / 255, 226.f / 255.f, 1.0f),
		std::make_shared<d2dw::SolidFill>(195.f / 255.f, 224.f / 255, 226.f / 255.f, 0.3f),
		std::make_shared<d2dw::SolidFill>(224.f / 255.f, 224.f / 255, 224.f / 255.f, 0.3f),
		std::make_shared<d2dw::SolidFill>(1.0f, 1.0f, 1.0f, 0.3f),
		std::make_shared<d2dw::CRectF>(2.0f, 2.0f, 2.0f, 2.0f)),
		DragLine(std::make_shared<d2dw::SolidLine>(1.0f, 0.0f, 0.0f, 1.0f, 1.0f)){};
	virtual ~HeaderProperty(){}

    template <class Archive>
    void serialize(Archive& ar)
    {
		CellProperty::serialize(ar);
    }
};
