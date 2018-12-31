#pragma once
#include "Sheet.h"
#include "Cell.h"

class CSheetCell:public CSheet,public CCell
{
protected:
	typedef CCell::size_type size_type;
	typedef CCell::string_type string_type;
	typedef CCell::coordinates_type coordinates_type;
public:
	virtual bool CanResizeRow() const{return false;}
	virtual bool CanResizeColumn() const{return false;}
	virtual void AddRow();
	virtual void Resize();
	virtual void Resize(size_type row, size_type col){}

public:
	//Constructor
	CSheetCell(
		CSheet* pSheet = nullptr,
		CRow* pRow = nullptr,
		CColumn* pColumn = nullptr,
		std::shared_ptr<CCellProperty> spProperty = nullptr,
		std::shared_ptr<CCellProperty> spHeaderProperty = nullptr,
		std::shared_ptr<CCellProperty> spFilterProperty = nullptr,
		std::shared_ptr<CCellProperty> spCellProperty = nullptr,
		CMenu* pMenu=nullptr);
	virtual ~CSheetCell(){}
	static CMenu SheetCellContextMenu;
	virtual CMenu* GetContextMenuPtr()override;
	//Property
	virtual std::shared_ptr<CCellProperty> GetHeaderPropertyPtr();
	virtual std::shared_ptr<CCellProperty> GetCellPropertyPtr();
	//Rect
	virtual coordinates_type GetTop()const override;
	virtual coordinates_type GetLeft()const override;
	virtual CRect GetRect()const override;
	virtual CRect GetPaintRect() override;

	virtual CSize MeasureSize(CDC* pDC) override;
	virtual CSize MeasureSizeWithFixedWidth(CDC* pDC) override;

	//virtual void SetFocused(const bool& bFocused);
	virtual bool GetSelected()const override;
	virtual void SetSelected(const bool& bSelected) override;

	//Paint
	virtual void PaintContent(CDC* pDC, CRect rcPaint) override;
	
	//Event
	virtual void OnLButtonDown(const LButtonDownEvent& e) override;
	virtual void OnLButtonUp(const LButtonUpEvent& e) override;
	virtual void OnLButtonClk(const LButtonClkEvent& e) override;
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e) override;

	virtual void OnMouseMove(const MouseMoveEvent& e) override;
	virtual void OnMouseLeave(const MouseLeaveEvent& e) override;
	virtual void OnContextMenu(const ContextMenuEvent& e) override;
	virtual void OnSetCursor(const SetCursorEvent& e) override;
	virtual void OnSetFocus(const SetFocusEvent& e) override;
	virtual void OnKillFocus(const KillFocusEvent& e) override;

	virtual void OnKeyDown(const KeyDownEvent& e)  override;
	//String
	virtual string_type GetString() override {return CSheet::GetSheetString();};
	virtual void SetString(const string_type& str)  override {/*Do Nothing*/};
	virtual bool Filter(const string_type& strFilter)const;
	//Compare
	virtual bool IsComparable()const;
	virtual Compares EqualCell(CCell* pCell, std::function<void(CCell*, Compares)> action);
	virtual Compares EqualCell(CEmptyCell* pCell, std::function<void(CCell*, Compares)> action);
	virtual Compares EqualCell(CTextCell* pCell, std::function<void(CCell*, Compares)> action);
	virtual Compares EqualCell(CSheetCell* pCell, std::function<void(CCell*, Compares)> action);

	virtual CPoint GetScrollPos()const;
	std::shared_ptr<CDC> GetClientDCPtr()const;
	virtual CGridView* GetGridPtr();

	void OnCellPropertyChanged(CCell* pCell, const wchar_t* name) override;

	virtual void ColumnInserted(CColumnEventArgs& e);
	virtual void ColumnErased(CColumnEventArgs& e);
	virtual void ColumnHeaderEndTrack(CColumnEventArgs& e);
	virtual void RowInserted(CRowEventArgs& e);
	virtual void RowErased(CRowEventArgs& e);

	virtual CColumn* GetParentColumnPtr(CCell* pCell) override;
	virtual void OnPropertyChanged(const wchar_t* name) override;
};