#pragma once
#include "ParentMapColumn.h"

class CParentDefaultColumn:public CParentMapColumn
{
protected:
	std::shared_ptr<bool> m_spVisible;

public:
	CParentDefaultColumn(CGridView* pGrid = nullptr, std::shared_ptr<bool> spVisible = nullptr):
		CParentMapColumn(pGrid), m_spVisible(spVisible){}
	virtual ~CParentDefaultColumn(){}

	bool GetVisible()const override
	{
		return *m_spVisible;
	}
	void SetVisible(const bool& bVisible, bool notify = true)override;

};