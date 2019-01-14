#pragma once
#include "Direct2DWrite.h"
//#include "MyFont.h"
//#include "MyColor.h"
//#include "MyBrush.h"
//#include "MyRect.h"
//#include "MyPen.h"

//#include "MyFriendSerializer.h"

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
	virtual ~BackgroundProperty(){}

	FRIEND_SERIALIZER

    template <class Archive>
    void serialize(Archive& ar)
    {
		ar("BackgroundBrush",m_brush);
		ar("UsePicture",m_usePicture);
		ar("PicturePath",m_picturePath);
		ar("PicturePosition",m_picturePosition);
    }
};

//class CDrawTextProperty
//{
//public:
//	UINT m_uFormat;
//	CDrawTextProperty():m_uFormat(0){}
//	CDrawTextProperty(UINT uFormat/*,DRAWTEXTPARAMS dtp*/):m_uFormat(uFormat)/*,m_dtp(dtp)*/{}
//	virtual ~CDrawTextProperty(){}
//	//DRAWTEXTPARAMS m_dtp;
//	UINT GetFormat()const{return m_uFormat;}
//	//DRAWTEXTPARAMS GetDrawTextParams()const{return m_dtp;}
//
//	FRIEND_SERIALIZER
//    template <class Archive>
//    void serialize(Archive& ar)
//    {
//        ar("Format",m_uFormat);
//    }
//
//};
//typedef std::shared_ptr<CDrawTextProperty> DrawTextPropertyPtr;

class CellProperty
{
public:
	std::shared_ptr<d2dw::FontAndColor> FontAndColor;
	std::shared_ptr<d2dw::SolidLine> Line;
	std::shared_ptr<d2dw::SolidLine> FocusedLine;
	std::shared_ptr<d2dw::SolidFill> NormalFill;
	std::shared_ptr<d2dw::SolidFill> FocusedFill;
	std::shared_ptr<d2dw::SolidFill> SelectedFill;
	std::shared_ptr<d2dw::SolidFill> UnfocusSelectedFill;
	std::shared_ptr<d2dw::SolidFill> HotFill;
	std::shared_ptr<d2dw::CRectF> Padding;

public:
	CellProperty()
		:FontAndColor(std::make_shared<d2dw::FontAndColor>(L"Meiryo UI", d2dw::CDirect2DWrite::Points2Dips(9.0f),  0.0f, 0.0f, 0.0f, 1.0f)),
		Line(std::make_shared<d2dw::SolidLine>(221.0f/255, 206.0f/255, 188.0f/255, 1.0f, 1.0f)),
		FocusedLine(std::make_shared<d2dw::SolidLine>(22.0f/255, 160.0f/255, 133.0f/255, 1.0f, 1.0f)),
		NormalFill(std::make_shared<d2dw::SolidFill>(246.0f/255, 246.0f/255, 246.0f/255, 1.0f)),
		FocusedFill(std::make_shared<d2dw::SolidFill>(0.0f/255, 224.0f/255, 226.0f/255, 1.0f)),
		SelectedFill(std::make_shared<d2dw::SolidFill>(195.0f/255, 224.0f/255, 226.0f/255, 0.35f)),
		UnfocusSelectedFill(std::make_shared<d2dw::SolidFill>(224.0f/255, 224.0f/255, 224.0f/255, 0.3f)),
		HotFill(std::make_shared<d2dw::SolidFill>(1.0f, 1.0f, 1.0f, 0.3f)),
		//m_spBrChecked(std::make_shared<CBrush>(RGB(241,196,15))),
		//m_spDrawTextProperty(std::make_shared<CDrawTextProperty>(DT_LEFT|DT_TOP|DT_NOPREFIX|DT_WORDBREAK|DT_EDITCONTROL)),
		Padding(std::make_shared<d2dw::CRectF>(2.0f,2.0f,2.0f,2.0f)){};

	CellProperty(
		std::shared_ptr<d2dw::FontAndColor> fontAndColor,
		std::shared_ptr<d2dw::SolidLine> line,
		std::shared_ptr<d2dw::SolidLine> focusedLine,
		std::shared_ptr<d2dw::SolidFill> normalFill,
		std::shared_ptr<d2dw::SolidFill> focusedFill,
		std::shared_ptr<d2dw::SolidFill> selectedFill,
		std::shared_ptr<d2dw::SolidFill> unfocusSelectedFill,
		std::shared_ptr<d2dw::SolidFill> hotFill,
		std::shared_ptr<d2dw::CRectF> padding)
		:FontAndColor(fontAndColor),
		Line(line),
		FocusedLine(focusedLine),
		NormalFill(normalFill),
		FocusedFill(focusedFill),
		SelectedFill(selectedFill),
		UnfocusSelectedFill(unfocusSelectedFill),
		HotFill(hotFill),

		Padding(padding){};

	virtual ~CellProperty(){}

	//virtual std::shared_ptr<CFont> GetFontPtr()const{return m_spFont;}
	//virtual std::shared_ptr<CPen> GetPenPtr()const{return m_spPen;}
	//virtual std::shared_ptr<CPen> GetFocusedPenPtr()const{return m_spPenFocused;}
	//virtual std::shared_ptr<CBrush> GetBackgroundBrushPtr()const{return m_spBrBackground;}
	//virtual std::shared_ptr<CBrush> GetFocusedBrushPtr()const{return m_spBrFocused;}
	//virtual std::shared_ptr<CBrush> GetSelectedBrushPtr()const{return m_spBrSelected;}
	//virtual std::shared_ptr<CBrush> GetUnfocusSelectedBrushPtr()const { return m_spBrUnfocusSelected; }
	//virtual std::shared_ptr<CBrush> GetCheckedBrushPtr()const{return m_spBrChecked;}
	////virtual std::shared_ptr<CDrawTextProperty> GetDrawTextPropertyPtr()const{return m_spDrawTextProperty;}
	////virtual UINT GetFormat()const{return m_spDrawTextProperty->GetFormat();}
	//virtual CRect GetPadding()const{return *m_spPadding;}
	//virtual std::shared_ptr<CRect> GetPaddingPtr(){return m_spPadding;}

//	FRIEND_SERIALIZER
	template <class Archive>
    void serialize(Archive& ar)
    {
		ar("FontAndColor",FontAndColor);
		ar("Line",Line);
		ar("FocusedLine",FocusedLine);
		ar("NormalFill",NormalFill);
		ar("FocusedFill",FocusedFill);
		ar("SelectedFill",SelectedFill);
		ar("UnfocusSelectedFill", UnfocusSelectedFill);
		ar("HotFill", HotFill);
		//ar("CheckedBrush",m_spBrChecked);
		//ar("DrawTextProperty",m_spDrawTextProperty);
		ar("Padding",Padding);
    }

};

//typedef std::shared_ptr<CellProperty> CellPropertyPtr;

struct HeaderProperty:public CellProperty
{
public:
	std::shared_ptr<d2dw::SolidLine> DragLine;

	//	CColor m_crTop;
//	CColor m_crBottom;
public:
	//CHeaderProperty():CCellProperty(),m_crTop(RGB(206,185,205)),m_crBottom(RGB(233,158,231)){}
	HeaderProperty()
		:CellProperty(
		std::make_shared<d2dw::FontAndColor>(L"Meiryo UI", d2dw::CDirect2DWrite::Points2Dips(9.0f), 0.0f, 0.0f, 0.0f, 1.0f),
		std::make_shared<d2dw::SolidLine>(211.0f / 255, 206.0f / 255, 188.0f / 255, 1.0f, 1.0f),
		std::make_shared<d2dw::SolidLine>(22.0f / 255, 160.0f / 255, 133.0f / 255, 1.0f, 1.0f),
		std::make_shared<d2dw::SolidFill>(239.0f / 255, 239.0f / 255, 232.0f / 255, 1.0f),
		std::make_shared<d2dw::SolidFill>(195.0f / 255, 224.0f / 255, 226.0f / 255, 1.0f),
		std::make_shared<d2dw::SolidFill>(195.0f / 255, 224.0f / 255, 226.0f / 255, 0.3f),
		std::make_shared<d2dw::SolidFill>(241.0f / 255, 196.0f / 255, 15.0f / 255, 0.3f),
		std::make_shared<d2dw::SolidFill>(1.0f, 1.0f, 1.0f, 0.3f),
		std::make_shared<d2dw::CRectF>(2.0f, 2.0f, 2.0f, 2.0f)),
		DragLine(std::make_shared<d2dw::SolidLine>(1.0f, 0.0f, 0.0f, 1.0f, 1.0f)){};
//		m_crTop(RGB(232,232,232)),m_crBottom(RGB(232,232,242)){}
	virtual ~HeaderProperty(){}
	//CColor GetTopColor()const
	//{
	////	return m_crTop;
	//}
	//CColor GetBottomColor()const
	//{
	//	return m_crBottom;
	//}

	FRIEND_SERIALIZER
    template <class Archive>
    void serialize(Archive& ar)
    {
		CellProperty::serialize(ar);
		//ar("TopColor",m_crTop);
		//ar("BottomColor",m_crBottom);
    }
};

//typedef std::shared_ptr<CHeaderProperty> HeadersPropertyPtr;
