#pragma once
#include "CheckBoxCell.h"


class CBoolCell:public CCheckBoxCell
{
private:
	CheckBoxState m_checkboxState;
public:
	CBoolCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty, bool b)
		:CCheckBoxCell(pSheet, pRow, pColumn, spProperty), m_checkboxState(b?CheckBoxState::True:CheckBoxState::False){}
	virtual ~CBoolCell() = default;

	virtual CheckBoxState GetCheckBoxState() const override
	{
		return m_checkboxState;
	}

	virtual void SetCheckBoxState(const CheckBoxState& state) override
	{
		m_checkboxState = state;
	}
};
