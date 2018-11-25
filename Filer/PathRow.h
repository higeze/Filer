#pragma once

#include "Row.h"

class CColumn;
class CPathCell;
class CParentRowHeaderCell;

class CPathRow :public CParentHeaderRow
{
private:
	std::shared_ptr<CCell> m_spPathCell;
	std::shared_ptr<CCell> m_spHeaderCell;
public:
	CPathRow(CGridView* pGrid) :CParentHeaderRow(pGrid) {}

	virtual bool HasCell()const override{ return true; }
	virtual std::shared_ptr<CCell>& Cell(CColumn* pCol) override;
};
