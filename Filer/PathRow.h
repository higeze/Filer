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
	CPathRow(CGridView* pSheet) 
		:CHeaderRow(pSheet) {}

	virtual bool HasCell()const override{ return true; }
	virtual std::shared_ptr<CCell>& Cell(CColumn* pCol) override;
	virtual bool IsTrackable()const override { return false; }
};
