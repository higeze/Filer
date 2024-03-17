#pragma once
#include "Cell.h"
#include "CheckBox.h"
#include "DeadlineTimer.h"

class CCheckBoxFilterCell :public CCell
{
private:
	const size_t CHECK_BOX_SIZE = 3;
	std::shared_ptr<int> Dummy;
	std::vector<CCheckBox> m_checkBoxes;
	CDeadlineTimer m_deadlinetimer;
public:
	CCheckBoxFilterCell(CGridView* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty);
	virtual ~CCheckBoxFilterCell() = default;
	virtual void PaintContent(CDirect2DWrite* pDirect, CRectF rcPaint) override;
	virtual CSizeF MeasureContentSize(CDirect2DWrite* pDirect) override;
	virtual CSizeF MeasureContentSizeWithFixedWidth(CDirect2DWrite* pDirect) override;
	virtual void SetStringCore(const std::wstring& str) override;
	virtual std::wstring GetString() override;

	virtual void OnPropertyChanged(const wchar_t* name) override;
	virtual void OnLButtonDown(const LButtonDownEvent& e) override;
};