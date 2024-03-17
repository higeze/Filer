#pragma once

#include "Row.h"

class CColumn;
class CPathCell;

class CPathRow :public CHeaderRow
{
private:
	std::shared_ptr<CCell> m_spPathCell;
	std::shared_ptr<CCell> m_spHeaderCell;
public:
	CPathRow(CGridView* pSheet, std::shared_ptr<CellProperty> spProperty) 
		:CHeaderRow(pSheet, spProperty) {}

	virtual bool HasCell()const override{ return true; }
	virtual std::shared_ptr<CCell>& Cell(CColumn* pCol) override;
	virtual bool IsTrackable()const override { return false; }
};
