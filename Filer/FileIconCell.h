#pragma once
#include "Cell.h"

class CShellFile;

class CFileIconCell:public CCell, public std::enable_shared_from_this<CFileIconCell>
{
protected:
	mutable boost::signals2::connection m_conDelayUpdateAction;
public:
	CFileIconCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty);
	virtual ~CFileIconCell();

	virtual bool IsComparable()const override{return false;}
	virtual void PaintContent(d2dw::CDirect2DWrite& direct, d2dw::CRectF rcPaint)override;
	virtual d2dw::CSizeF MeasureContentSize(d2dw::CDirect2DWrite& direct) override;
	virtual d2dw::CSizeF MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite& direct) override;
	virtual std::shared_ptr<CShellFile> GetShellFile();
};