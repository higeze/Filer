#pragma once
#include "Row.h"
#include <unordered_map>

class CCell;
class CColumn;

class CMapRow:public CRow
{
protected:
	std::unordered_map<const CColumn*,std::shared_ptr<CCell>> m_mapCell; /**<Map to hold row-cell*/
public:
	template<typename... Args>
	CMapRow(CGridView* pSheet = nullptr, Args... args):CRow(pSheet, args...){}
	virtual ~CMapRow() = default;

	virtual void Clear(){m_mapCell.clear();}
	virtual void Erase(const CColumn* pCol) 
	{
		if (m_mapCell.find(pCol) != m_mapCell.end()) {
			m_mapCell.erase(pCol);
		}
	}
};