#pragma once
#include "Band.h"

class CCell;
class Sheet;
class CColumn;
struct RowTag;

class CRow:public CBand
{
public:
	typedef RowTag Tag;
protected:
	FLOAT m_height;
	FLOAT m_top;
	FLOAT m_minHeight = 2;
	FLOAT m_maxHeight = 100;

public:
	CRow(CSheet* pSheet):CBand(pSheet),m_height(0),m_top(0){}
	virtual ~CRow(){}

	virtual bool HasCell()const { return false; }
	virtual std::shared_ptr<CCell>& Cell(CColumn* pCol) { throw std::exception("Not implemented"); }

	virtual FLOAT GetDefaultHeight();
	virtual FLOAT GetHeight();
	virtual void SetHeight(const FLOAT height);
	virtual void SetHeightWithoutSignal(const FLOAT height){m_height=height;}

	//virtual FLOAT GetTop()const{return m_top+Offset();}
	virtual FLOAT GetTop()const{return m_top;}
	virtual void SetTop(const FLOAT& top){m_top=top;}//TODO low setter
	virtual void SetTopWithoutSignal(const FLOAT& top){m_top=top;}

	//virtual FLOAT GetBottom()const{return m_top+m_height+Offset();}
	virtual FLOAT GetBottom()const{return m_top + m_height;}

	virtual void SetVisible(const bool& bVisible, bool notify = true)override;
	virtual void SetSelected(const bool& bSelected);
	virtual void OnCellPropertyChanged(CCell* pCell, const wchar_t* name) override;
	virtual void OnPropertyChanged(const wchar_t* name) override;
	virtual SizingType GetSizingType()const override { return SizingType::None; }
};

class CGridView;

class CParentRow:public CRow
{
public:
	CParentRow(CGridView* pGrid);
	virtual ~CParentRow(){}
};

class CParentHeaderRow:public CParentRow
{
public:
	using CParentRow::CParentRow;
	virtual bool IsTrackable()const override { return true; }
};

class CSheetCell;

class CChildRow:public CRow
{
public:
	CChildRow(CSheetCell* pSheetCell);
	virtual ~CChildRow(){}
};

class CChildHeaderRow:public CChildRow
{
public:
	CChildHeaderRow(CSheetCell* pGrid):CChildRow(pGrid){}
	virtual bool IsTrackable()const override { return true; }
};
