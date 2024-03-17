#pragma once
#include "Cell.h"

class CEmptyCell:public CCell
{
public:
	CEmptyCell(CGridView* pSheet = nullptr, CRow* pRow = nullptr, CColumn* pColumn = nullptr,
		std::shared_ptr<CellProperty> spProperty = nullptr)
		:CCell(pSheet, pRow, pColumn, spProperty){}
	virtual ~CEmptyCell(){}
	
	virtual bool GetIsClipboardCopyable()const{return true;}
	virtual std::wstring GetClipboardData()const{return std::wstring();}
	virtual void SetClipboardData(const std::wstring& source){}
};