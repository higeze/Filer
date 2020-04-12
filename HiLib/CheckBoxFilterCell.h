#pragma once
#include "CheckBoxCell.h"
#include "DeadlineTimer.h"

class CCheckBoxFilterCell :public CCheckBoxCell
{
private:
	CDeadlineTimer m_deadlinetimer;
	bool m_check = false;

public:
	CCheckBoxFilterCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty);
	virtual ~CCheckBoxFilterCell() = default;

	virtual CheckBoxState GetCheckBoxState() const override;
	virtual void SetCheckBoxState(const CheckBoxState& state) override;

	virtual std::wstring GetString() override;
	virtual void SetStringCore(const std::wstring& str)override;

	virtual void OnPropertyChanged(const wchar_t* name) override;
};