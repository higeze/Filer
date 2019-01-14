#pragma once
#include "Sheet.h"
#include "Cell.h"

class CSheetCell:public CSheet,public CCell
{
protected:

public:
	virtual bool CanResizeRow() const{return false;}
	virtual bool CanResizeColumn() const{return false;}
	virtual void AddRow();
	virtual void Resize();
	virtual void Resize(int row, int col){}

public:
	//Constructor
	CSheetCell(
		CSheet* pSheet = nullptr,
		CRow* pRow = nullptr,
		CColumn* pColumn = nullptr,
		std::shared_ptr<CellProperty> spProperty = nullptr,
		std::shared_ptr<HeaderProperty> spHeaderProperty = nullptr,
		std::shared_ptr<CellProperty> spFilterProperty = nullptr,
		std::shared_ptr<CellProperty> spCellProperty = nullptr,
		CMenu* pMenu=nullptr);
	virtual ~CSheetCell(){}
	static CMenu SheetCellContextMenu;
	virtual CMenu* GetContextMenuPtr()override;
	//Property
	virtual std::shared_ptr<HeaderProperty> GetHeaderPropertyPtr();
	virtual std::shared_ptr<CellProperty> GetCellPropertyPtr();
	//Rect
	virtual FLOAT GetTop()const override;
	virtual FLOAT GetLeft()const override;
	virtual d2dw::CRectF GetRect()const override;
	virtual d2dw::CRectF GetPaintRect() override;

	virtual d2dw::CSizeF MeasureSize(d2dw::CDirect2DWrite& direct) override;
	virtual d2dw::CSizeF MeasureSizeWithFixedWidth(d2dw::CDirect2DWrite& direct) override;

	//virtual void SetFocused(const bool& bFocused);
	virtual bool GetSelected()const override;
	virtual void SetSelected(const bool& bSelected) override;

	//Paint
	virtual void PaintContent(d2dw::CDirect2DWrite& direct, d2dw::CRectF rcPaint) override;
	
	//Event
	virtual void OnLButtonDown(const LButtonDownEvent& e) override;
	virtual void OnLButtonUp(const LButtonUpEvent& e) override;
	virtual void OnLButtonClk(const LButtonClkEvent& e) override;
	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e) override;
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e) override;

	virtual void OnMouseMove(const MouseMoveEvent& e) override;
	virtual void OnMouseLeave(const MouseLeaveEvent& e) override;
	virtual void OnContextMenu(const ContextMenuEvent& e) override;
	virtual void OnSetCursor(const SetCursorEvent& e) override;
	virtual void OnSetFocus(const SetFocusEvent& e) override;
	virtual void OnKillFocus(const KillFocusEvent& e) override;

	virtual void OnKeyDown(const KeyDownEvent& e)  override;
	//String
	virtual std::wstring GetString() override {return CSheet::GetSheetString();};
	virtual void SetString(const std::wstring& str)  override {/*Do Nothing*/};
	virtual bool Filter(const std::wstring& strFilter)const;
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