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

	virtual bool GetCheck() const override;
	virtual void SetCheck(bool check) override;

	virtual std::wstring GetString() override;
	virtual void SetString(const std::wstring& str)override;
	virtual void SetStringCore(const std::wstring& str)override;
	virtual bool IsComparable()const override
	{
		return false;
	}

	virtual void OnPropertyChanged(const wchar_t* name) override;
};