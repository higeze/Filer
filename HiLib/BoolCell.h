#pragma once
#include "CheckBoxCell.h"


class CBoolCell:public CCheckBoxCell
{
private:
	CheckBoxState m_checkboxState;
public:
	CBoolCell(CGridView* pSheet, CRow* pRow, CColumn* pColumn, bool b)
		:CCheckBoxCell(pSheet, pRow, pColumn), m_checkboxState(b?CheckBoxState::True:CheckBoxState::False){}
	virtual ~CBoolCell() = default;
};
