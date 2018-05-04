#pragma once
#include "Cell.h"

class CEmptyCell:public CCell
{
public:
	CEmptyCell(CSheet* pSheet = nullptr, CRow* pRow = nullptr, CColumn* pColumn = nullptr,
		std::shared_ptr<CCellProperty> spProperty = nullptr)
		:CCell(pSheet, pRow, pColumn, spProperty){}
	virtual ~CEmptyCell(){}
	
	virtual bool IsComparable()const{return true;}
	
	Compares EqualCell(CCell* pCell, std::function<void(CCell*, Compares)> action)
	{
		return pCell->EqualCell(this, action);
	}

	Compares EqualCell(CEmptyCell* pCell, std::function<void(CCell*, Compares)> action)
	{
		action(this, Compares::Same);
		return Compares::Same;
	}

	Compares EqualCell(CTextCell* pCell, std::function<void(CCell*, Compares)> action)
	{
		action(this, Compares::DiffNE);
		return Compares::DiffNE;
	}

	Compares EqualCell(CSheetCell* pCell, std::function<void(CCell*, Compares)> action)
	{
		action(this, Compares::DiffNE);
		return Compares::DiffNE;
	}
	virtual bool IsClipboardCopyable()const{return true;}
	virtual std::wstring GetClipboardData()const{return std::wstring();}
	virtual void SetClipboardData(const std::wstring& source){}
};

class CIncomparableEmptyCell:public CEmptyCell
{
public:
	CIncomparableEmptyCell(CSheet* pSheet = nullptr, CRow* pRow = nullptr, CColumn* pColumn = nullptr, std::shared_ptr<CCellProperty> spProperty = nullptr)
		:CEmptyCell(pSheet, pRow, pColumn, spProperty){}
	virtual ~CIncomparableEmptyCell(){}
	
	virtual bool IsComparable()const{return false;}
};
