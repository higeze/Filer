#pragma once
#include "D2DWControl.h"
#include "ButtonProperty.h"

//struct TabHeaderControlProperty
//{
//	FormatF Format;
//	CRectF Padding;
//	std::shared_ptr<ButtonProperty> ButtonProp;
//	TabHeaderControlProperty()
//		:Format(L"Meiryo UI", CDirect2DWrite::Points2Dips(9),  0.0f, 0.0f, 0.0f, 1.0f),
//		Padding(2.0f,2.0f,2.0f,2.0f),
//		ButtonProp(std::make_shared<ButtonProperty>())
//	{
//		ButtonProp->BorderLine = SolidLine(0.f, 0.f, 0.f, 0.f, 0.f);
//	};
//};
//
//struct TabControlProperty
//{
//	std::shared_ptr<TabHeaderControlProperty> HeaderProperty;
//	SolidLine Line;
//	SolidFill SelectedFill;
//	SolidFill UnfocusSelectedFill;
//	SolidFill NormalFill;
//	SolidFill HotFill;
//	CRectF Padding;
//
//	TabControlProperty()
//		:HeaderProperty(std::make_shared<TabHeaderControlProperty>()),
//		Line(221.f/255.f, 206.f/255.f, 188.f/255.f, 1.0f, 1.0f),
//		NormalFill(239.f/255.f, 239.f/255.f, 239.f/255.f, 1.0f),
//		SelectedFill(255.f/255.f, 255.f/255.f, 255.f/255.f, 1.0f),
//		UnfocusSelectedFill(247.f/255.f, 247.f/255.f, 247.f/255.f, 1.0f),
//		HotFill(1.0f, 1.0f, 1.0f, 0.3f),
//		Padding(2.0f,2.0f,2.0f,2.0f){};
//};