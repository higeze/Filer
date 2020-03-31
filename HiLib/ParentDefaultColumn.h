#pragma once
#include "MapColumn.h"

class CParentDefaultColumn:public CMapColumn
{
protected:
	std::shared_ptr<bool> m_spVisible;

public:
	CParentDefaultColumn(CSheet* pSheet = nullptr, std::shared_ptr<bool> spVisible = nullptr):
		CMapColumn(pSheet), m_spVisible(spVisible){}
	virtual ~CParentDefaultColumn(){}

	bool GetVisible()const override
	{
		return *m_spVisible;
	}
	void SetVisible(const bool& bVisible, bool notify = true)override;

};