#pragma once
#include "TextCell.h"

class CPathCell:public CTextCell
{
public:
	/**
	 *  Constructor
	 */
	CPathCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn,std::shared_ptr<CellProperty> spProperty)
		:CTextCell(pSheet,pRow, pColumn,spProperty, arg<"editmode"_s>() = EditMode::LButtonDownEdit){}
	/**
	 *  Destructor
	 */
	virtual ~CPathCell(void){}
	d2dw::CSizeF GetFitSize(d2dw::CDirect2DWrite* pDirect)override;
	d2dw::CSizeF GetActSize(d2dw::CDirect2DWrite* pDirect)override;
	//CSize MeasureContentSize(CDC* pDC)override;
	d2dw::CSizeF MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite* pDirect)override;
	/**
	 *  Get string from column property
	 */
	virtual std::wstring GetString() override;

	virtual d2dw::CRectF CPathCell::GetRectInWnd()const override;
	/**
	 *  Set string to column property
	 */
	//virtual void SetString(const std::wstring& str)override;
	virtual void SetStringCore(const std::wstring& str)override;
	virtual void OnPaint(const PaintEvent& e)override;

	virtual bool IsLeftestCell()const;

	virtual bool CanSetStringOnEditing()const override{return false;}
};

