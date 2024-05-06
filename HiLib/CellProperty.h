#pragma once
#include "Direct2DWrite.h"
#include "ScrollProperty.h"
#include <regex>
#include "reactive_property.h"
#include "JsonSerializer.h"
#include "TextBoxProperty.h"

//REMOVE
//enum class PicturePositon
//{
//	Fill,
//	Fit,
//	Stretch,
//	Tile,
//	Center
//};

//struct BackgroundProperty
//{
//public:
//	std::shared_ptr<SolidFill> m_brush;
//	std::shared_ptr<bool> m_usePicture;
//	std::shared_ptr<std::wstring> m_picturePath;
//	std::shared_ptr<PicturePositon> m_picturePosition;
//
//	BackgroundProperty()
//		:m_brush(std::make_shared<SolidFill>(246.0f/255, 246.0f/255, 246.0f/255, 1.0f)),//asbestos
//		m_usePicture(std::make_shared<bool>(false)),
//		m_picturePath(std::make_shared<std::wstring>()),
//		m_picturePosition(std::make_shared<PicturePositon>(PicturePositon::Center)){}
//	virtual ~BackgroundProperty() = default;
//
//    template <class Archive>
//    void serialize(Archive& ar)
//    {
//		ar("BackgroundBrush",m_brush);
//		ar("UsePicture",m_usePicture);
//		ar("PicturePath",m_picturePath);
//		ar("PicturePosition",m_picturePosition);
//    }
//
//	friend void to_json(json& j, const BackgroundProperty& o)
//	{
//		j = json{
//			{"BackgroundBrush", o.m_brush},
//			{"UsePicture", o.m_usePicture},
//			{"PicturePath", o.m_picturePath},
//			{"PicturePosition", o.m_picturePosition}
//		};
//	}
//	friend void from_json(const json& j, BackgroundProperty& o)
//	{
//		j.at("BackgroundBrush").get_to(o.m_brush);
//		j.at("UsePicture").get_to(o.m_usePicture);
//		j.at("PicturePath").get_to(o.m_picturePath);
//		j.at("PicturePosition").get_to(o.m_picturePosition);
//	}
//};
//
//struct CellProperty :public TextBoxProperty
//{
//public:
//	using TextBoxProperty::TextBoxProperty;
//};
//
//
//struct HeaderProperty:public CellProperty
//{
//public:
//	std::shared_ptr<SolidLine> DragLine;
//public:
//	HeaderProperty()
//		:CellProperty(
//		std::make_shared<FormatF>(L"Meiryo UI", CDirect2DWrite::Points2Dips(9), 0.0f, 0.0f, 0.0f, 1.0f),
//		std::make_shared<SolidLine>(211.f / 255.f, 206.f / 255.f, 188.f / 255.f, 1.0f, 1.0f),
//		std::make_shared<SolidLine>(0.f / 255.f, 0.f / 255.f, 255.f / 255.f, 1.0f, 1.0f),
//		std::make_shared<SolidLine>(22.f / 255.f, 160.f / 255, 133.f / 255.f, 1.0f, 1.0f),
//		std::make_shared<SolidFill>(239.f / 255.f, 239.f / 255, 232.f / 255.f, 1.0f),
//		std::make_shared<SolidFill>(195.f / 255.f, 224.f / 255, 226.f / 255.f, 1.0f),
//		std::make_shared<SolidFill>(195.f / 255.f, 224.f / 255, 226.f / 255.f, 0.3f),
//		std::make_shared<SolidFill>(224.f / 255.f, 224.f / 255, 224.f / 255.f, 0.3f),
//		std::make_shared<SolidFill>(1.0f, 1.0f, 1.0f, 0.3f),
//		std::make_shared<CRectF>(2.0f, 2.0f, 2.0f, 2.0f)),
//		DragLine(std::make_shared<SolidLine>(1.0f, 0.0f, 0.0f, 1.0f, 1.0f)){};
//	virtual ~HeaderProperty(){}
//
//    template <class Archive>
//    void save(Archive& ar)
//    {
//		CellProperty::save(ar);
//    }
//    template <class Archive>
//    void load(Archive& ar)
//    {
//		CellProperty::load(ar);
//    }
//};
