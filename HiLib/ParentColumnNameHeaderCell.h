#pragma once

#include "ParentHeaderCell.h"
#include "SheetEnums.h"
#include "MyMenu.h"


class CParentColumnNameHeaderCell:public CParentHeaderCell
{
public:
	static CMenu ContextMenu;

private:

	static const FLOAT TRIANGLE_WIDTH;
	static const FLOAT TRIANGLE_HEIGHT;
	static const d2dw::CRectF TRIANGLE_MARGIN;
	static const FLOAT MIN_COLUMN_WIDTH;

public:
	CParentColumnNameHeaderCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty, CMenu* pMenu = &CParentColumnNameHeaderCell::ContextMenu)
		:CParentHeaderCell(pSheet, pRow, pColumn, spProperty, pMenu?pMenu:&CParentColumnNameHeaderCell::ContextMenu){}
	virtual ~CParentColumnNameHeaderCell(){}

	Sorts GetSort()const;
	d2dw::CSizeF GetSortSize()const;
	//virtual d2dw::CSizeF GetInitSize(d2dw::CDirect2DWrite& direct) override {return d2dw::CSizeF(MIN_COLUMN_WIDTH,0);}
	virtual d2dw::CSizeF MeasureContentSize(d2dw::CDirect2DWrite& direct) override;
	virtual d2dw::CSizeF MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite& direct) override;
	virtual void PaintContent(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint) override;

	virtual void OnLButtonClk(const LButtonClkEvent& e)override;

	void PaintSortMark(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint);
};

class CParentColumnHeaderStringCell:public CParentColumnNameHeaderCell
{
private:
	std::wstring m_strHeader;
public:
	CParentColumnHeaderStringCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty,std::wstring str,CMenu* pMenu=nullptr)
		:CParentColumnNameHeaderCell(pSheet, pRow, pColumn, spProperty,pMenu),m_strHeader(str){}
	virtual ~CParentColumnHeaderStringCell(){}
	virtual std::wstring GetString(){return m_strHeader;}
	virtual void SetStringCore(const std::wstring& str){m_strHeader = str;}


};