#pragma once
#include "TextCell.h"
#include "GDIUIElement.h"


/**
 * CBoolCell
 */
class CBoolCell:public CTextCell
{
private:
	bool m_bool;
	CCheckBox m_checkBox;
public:
	CBoolCell(CSheet* pSheet,CRow* pRow, CColumn* pColumn,std::shared_ptr<CellProperty> spProperty,bool b);
	virtual ~CBoolCell(){}

	virtual void PaintContent(CDC* pDC,CRect rcPaint);
	virtual CSize MeasureSize(CDC* pDC);
	virtual CSize MeasureSizeWithFixedWidth(CDC* pDC);

	virtual std::wstring GetString();
	virtual void SetStringCore(const std::wstring& str);

	virtual void OnLButtonDown(const LButtonDownEvent& e) override;

	virtual bool IsComparable()const;
};
