#pragma once
#include "TextCell.h"


/**
 * CBoolCell
 */
class CBoolCell:public CTextCell
{
private:
	bool m_bool;
//	CCheckBox m_checkBox;
public:
	CBoolCell(CSheet* pSheet,CRow* pRow, CColumn* pColumn,std::shared_ptr<CellProperty> spProperty,bool b);
	virtual ~CBoolCell(){}

	virtual void PaintContent(d2dw::CDirect2DWrite& direct,d2dw::CRectF rcPaint) override;
	virtual d2dw::CSizeF MeasureSize(d2dw::CDirect2DWrite& direct) override;
	virtual d2dw::CSizeF MeasureSizeWithFixedWidth(d2dw::CDirect2DWrite& direct) override;

	virtual std::wstring GetString();
	virtual void SetStringCore(const std::wstring& str);

	virtual void OnLButtonDown(const LButtonDownEvent& e) override;

	virtual bool IsComparable()const;
};
