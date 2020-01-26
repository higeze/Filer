#pragma once
#include "TextCell.h"

class CCheckBoxCell :public CCell
{
private:
public:
	CCheckBoxCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty);
	virtual ~CCheckBoxCell() = default;
	virtual bool GetCheck()const = 0;
	virtual void SetCheck(bool check) = 0;

	virtual void PaintContent(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint) override;
	virtual d2dw::CSizeF MeasureSize(d2dw::CDirect2DWrite* pDirect) override;
	virtual d2dw::CSizeF MeasureSizeWithFixedWidth(d2dw::CDirect2DWrite* pDirect) override;

	virtual std::wstring GetString();
	virtual void SetStringCore(const std::wstring& str);

	virtual void OnLButtonDown(const LButtonDownEvent& e) override;

	virtual bool IsComparable()const;
};