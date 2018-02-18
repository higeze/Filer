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
	CPathCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn,std::shared_ptr<CCellProperty> spProperty,CMenu* pMenu = nullptr)
		:CEditableCell(pSheet,pRow, pColumn,spProperty,pMenu){}
	/**
	 *  Destructor
	 */
	virtual ~CPathCell(void){}
	CSize GetFitSize(CDC* pDC)override;
	CSize GetActSize(CDC* pDC)override;
	//CSize MeasureContentSize(CDC* pDC)override;
	CSize MeasureContentSizeWithFixedWidth(CDC* pDC)override;
	/**
	 *  Get string from column property
	 */
	virtual string_type GetString()const override;

	virtual CRect CPathCell::GetRect()const override;
	/**
	 *  Set string to column property
	 */
	//virtual void SetString(const string_type& str)override;
	virtual void SetStringCore(const string_type& str)override;
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

