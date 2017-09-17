#pragma once

#include "Cell.h"

class CFileIconStringCell:public CCell
{
private:

public:
	CFileIconStringCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CCellProperty> spProperty);
	virtual ~CFileIconStringCell(){}

	virtual bool IsComparable()const override{return false;}
	virtual void PaintContent(CDC* pDC, CRect rcPaint)override;
	virtual CSize MeasureContentSize(CDC* pDC) override;
	virtual CSize MeasureContentSizeWithFixedWidth(CDC* pDC) override;
	virtual UINT GetFormat()const{ return DT_CENTER|DT_VCENTER|DT_NOPREFIX|DT_WORDBREAK|DT_EDITCONTROL; }
};