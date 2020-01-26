#pragma once
#include "CheckBoxCell.h"


class CBoolCell:public CCheckBoxCell
{
private:
	bool m_bool;
public:
	CBoolCell::CBoolCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty, bool b)
		:CCheckBoxCell(pSheet, pRow, pColumn, spProperty), m_bool(b){}
	virtual ~CBoolCell() = default;

	virtual bool GetCheck() const override
	{
		return m_bool;
	}

	virtual void SetCheck(bool check) override
	{
		m_bool = check;
	}
};
