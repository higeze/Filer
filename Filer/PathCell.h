#pragma once
#include "TextCell.h"

class CPathCell:public CTextCell
{
public:
	/**
	 *  Constructor
	 */
	CPathCell(CGridView* pSheet, CRow* pRow, CColumn* pColumn)
		:CTextCell(pSheet,pRow, pColumn, arg<"editmode"_s>() = EditMode::LButtonDownEdit){}
	/**
	 *  Destructor
	 */
	virtual ~CPathCell(void){}
	CSizeF GetFitSize(CDirect2DWrite* pDirect)override;
	CSizeF GetActSize(CDirect2DWrite* pDirect)override;
	//CSize MeasureContentSize(CDC* pDC)override;
	CSizeF MeasureContentSizeWithFixedWidth(CDirect2DWrite* pDirect)override;
	/**
	 *  Get string from column property
	 */
	virtual std::wstring GetString() override;
	virtual void SetString(const std::wstring& str, bool notify);

	virtual CRectF GetRectInWnd()const override;
	/**
	 *  Set string to column property
	 */
	//virtual void SetString(const std::wstring& str)override;
	virtual void SetStringCore(const std::wstring& str)override;
	virtual void OnPaint(const PaintEvent& e)override;

	virtual bool IsLeftestCell()const;

	virtual bool CanSetStringOnEditing()const override{return false;}
};

