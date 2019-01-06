#pragma once

#include "TextCell.h"

class IInplaceEdit;
class CSize;
class CDC;

class CPathCell:public CEditableCell
{
public:
	/**
	 *  Constructor
	 */
	CPathCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn,std::shared_ptr<CellProperty> spProperty,CMenu* pMenu = nullptr)
		:CEditableCell(pSheet,pRow, pColumn,spProperty,pMenu){}
	/**
	 *  Destructor
	 */
	virtual ~CPathCell(void){}
	d2dw::CSizeF GetFitSize(d2dw::CDirect2DWrite& direct)override;
	d2dw::CSizeF GetActSize(d2dw::CDirect2DWrite& direct)override;
	//CSize MeasureContentSize(CDC* pDC)override;
	d2dw::CSizeF MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite& direct)override;
	/**
	 *  Get string from column property
	 */
	virtual std::wstring GetString() override;

	virtual d2dw::CRectF CPathCell::GetRect()const override;
	/**
	 *  Set string to column property
	 */
	//virtual void SetString(const std::wstring& str)override;
	virtual void SetStringCore(const std::wstring& str)override;
	virtual void OnPaint(const PaintEvent& e)override;

	/**
	 *  Comparable or not
	 */
	virtual bool IsComparable()const override
	{
		return false;
	}
	virtual bool CanSetStringOnEditing()const override{return false;}
};

