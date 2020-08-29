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
		std::shared_ptr<SheetProperty> spSheetProperty = nullptr,
		std::shared_ptr<CellProperty> spCellProperty = nullptr,
		CMenu* pMenu=nullptr);
	virtual ~CSheetCell() = default;
	//Rect
	virtual FLOAT GetTop()const override;
	virtual FLOAT GetLeft()const override;
	virtual d2dw::CRectF GetRect()const override;
	virtual d2dw::CRectF GetPaintRect() override;

	virtual d2dw::CSizeF MeasureContentSize(d2dw::CDirect2DWrite* pDirect) override;
	virtual d2dw::CSizeF MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite* pDirect) override;

	//virtual void SetFocused(const bool& bFocused);
	virtual bool GetIsSelected()const override;
	virtual void SetIsSelected(const bool& bSelected) override;
	virtual bool GetIsFocused()const override;


	//Paint
	virtual void PaintContent(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint) override;
	
	//Event
	virtual void OnPaint(const PaintEvent& e) override {}
	virtual void OnClose(const CloseEvent & e) override {}
	virtual void OnRect(const RectEvent& e) override {}

	virtual void OnLButtonDown(const LButtonDownEvent& e) override;
	virtual void OnLButtonUp(const LButtonUpEvent& e) override;
	virtual void OnLButtonClk(const LButtonClkEvent& e) override;
	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e) override;
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e) override;
	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) override;

	virtual void OnRButtonDown(const RButtonDownEvent& e) override;

	virtual void OnMButtonDown(const MouseEvent& e) override {}//TODO
	virtual void OnMButtonUp(const MouseEvent& e) override {}//TODO

	virtual void OnMouseMove(const MouseMoveEvent& e) override;
	virtual void OnMouseEnter(const MouseEvent& e) override {}
	virtual void OnMouseLeave(const MouseLeaveEvent& e) override;

	virtual void OnMouseWheel(const MouseWheelEvent& e) override {}

	virtual void OnKeyDown(const KeyDownEvent& e)  override;
	virtual void OnSysKeyDown(const SysKeyDownEvent& e)  override {}
	virtual void OnChar(const CharEvent& e)  override;
	
	virtual void OnContextMenu(const ContextMenuEvent& e) override;
	virtual void OnSetFocus(const SetFocusEvent& e) override;
	virtual void OnSetCursor(const SetCursorEvent& e) override;
	virtual void OnKillFocus(const KillFocusEvent& e) override;

	//String
	virtual std::wstring GetString() override {return CSheet::GetSheetString();}
	virtual void SetString(const std::wstring& str, bool notify)  override {/*Do Nothing*/}
	virtual bool Filter(const std::wstring& strFilter)const;

	virtual d2dw::CPointF GetScrollPos()const;
	std::shared_ptr<CDC> GetClientDCPtr()const;
	virtual CGridView* GetGridPtr();

	void OnCellPropertyChanged(CCell* pCell, const wchar_t* name) override;

	virtual void ColumnInserted(CColumnEventArgs& e) override;
	virtual void ColumnErased(CColumnEventArgs& e) override;
//	virtual void ColumnHeaderEndTrack(CColumnEventArgs& e) override;
	virtual void RowInserted(CRowEventArgs& e) override;
	virtual void RowErased(CRowEventArgs& e) override;

	virtual CColumn* GetParentColumnPtr(CCell* pCell) override;
	virtual void OnPropertyChanged(const wchar_t* name) override;
	virtual void OnRowPropertyChanged(CRow* pRow, const wchar_t* name) override;
	virtual void OnColumnPropertyChanged(CColumn* pCol, const wchar_t* name) override;


	//virtual SizingType GetRowSizingType()const { return SizingType::Depend; }
	//virtual SizingType GetColSizingType()const { return SizingType::Depend; }

	virtual void UpdateRow()override;
	virtual void UpdateColumn()override;

};