//#pragma once
//#include "reactive_property.h"
//
//class CGridLength
//{
//public:
//	reactive_property_ptr<bool> Auto;
//	reactive_property_ptr<bool> Star;
//	reactive_property_ptr<FLOAT> Value;
//	CGridLength(bool a = false, bool s = true, FLOAT v = 0.f)
//		:Auto(a), Star(s), Value(v) {}
//};
//
//class CColumnDefinition
//{
//public:
//	reactive_property_ptr<CGridLength> Width;
//};
//
//class CRowDefinition
//{
//public:
//	reactive_property_ptr<CGridLength> Width;
//};
//
//#include "D2DWControl.h"
//#include "reactive_vector.h"
//
//class CGrid :public CD2DWControl
//{
//public:
//	reactive_vector_ptr<CColumnDefinition> ColumnDefinitions;
//	reactive_vector_ptr<CRowDefinition> RowDefinitions;
//public:
//	using CD2DWControl::CD2DWControl;
//
//	virtual void Measure(const CSizeF& availableSize);
//	//virtual CSizeF DesiredSize() const { return m_size; }
//	//virtual CSizeF RenderSize() const { return CSizeF(m_size.width - GetMargin().Width(), m_size.height - GetMargin().Height()); }
//
//	//virtual CRectF RenderRect() const { return GetRectInWnd().DeflateRectCopy(GetMargin()); }
//	virtual void Arrange(const CRectF& rc);
//	//virtual CRectF ArrangedRect() const { return m_rect; }
//
//
//};
