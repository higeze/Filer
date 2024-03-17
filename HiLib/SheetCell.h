//#pragma once
//#include "GridView.h"
//#include "Cell.h"
//
//class CSheetCell:public CSheet,public CCell
//{
//protected:
//
//public:
//	virtual bool CanResizeRow() const{return false;}
//	virtual bool CanResizeColumn() const{return false;}
//	virtual void AddRow();
//	virtual void Resize();
//	virtual void Resize(int row, int col){}
//
//public:
//	//Constructor
//	CSheetCell(
//		CGridView* pSheet = nullptr,
//		CRow* pRow = nullptr,
//		CColumn* pColumn = nullptr,
//		std::shared_ptr<SheetProperty> spSheetProperty = nullptr,
//		std::shared_ptr<CellProperty> spCellProperty = nullptr,
//		CMenu* pMenu=nullptr);
//	virtual ~CSheetCell() = default;
//	//Rect
//	virtual FLOAT GetTop()const override;
//	virtual FLOAT GetLeft()const override;
//	virtual CRectF GetRectInWnd()const override;
//	virtual CRectF GetPaintRect() override;
//
//	virtual CSizeF MeasureContentSize(CDirect2DWrite* pDirect) override;
//	virtual CSizeF MeasureContentSizeWithFixedWidth(CDirect2DWrite* pDirect) override;
//
//	//virtual void SetFocused(const bool& bFocused);
//	virtual bool GetIsSelected()const override;
//	virtual void SetIsSelected(const bool& bSelected) override;
//	virtual bool GetIsFocused()const override;
//
//	virtual CD2DWWindow* GetWndPtr()const override { return CSheet::GetWndPtr(); }
//
//	//Paint
//	virtual void PaintContent(CDirect2DWrite* pDirect, CRectF rcPaint) override;
//	
//	//Event
//	virtual void OnCreate(const CreateEvt& e);
//	virtual void OnDestroy(const DestroyEvent& e);
//	virtual void OnEnable(const EnableEvent& e);
//
//	virtual void OnPaint(const PaintEvent& e) override {}
//	virtual void OnClose(const CloseEvent & e) override {}
//	virtual void OnRect(const RectEvent& e) override {}
//	virtual void OnCommand(const CommandEvent& e) override {}
//
//	virtual void OnLButtonDown(const LButtonDownEvent& e) override;
//	virtual void OnLButtonUp(const LButtonUpEvent& e) override;
//	virtual void OnLButtonClk(const LButtonClkEvent& e) override;
//	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e) override;
//	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e) override;
//	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) override;
//	virtual void OnLButtonEndDrag(const LButtonEndDragEvent& e) override;
//
//	virtual void OnRButtonDown(const RButtonDownEvent& e) override;
//	virtual void OnRButtonUp(const RButtonUpEvent& e) override;
//
//	virtual void OnMButtonDown(const MButtonDownEvent& e) override {}
//	virtual void OnMButtonUp(const MButtonUpEvent& e) override {}
//
//	virtual void OnMouseMove(const MouseMoveEvent& e) override;
//	virtual void OnMouseEnter(const MouseEnterEvent& e) override {}
//	virtual void OnMouseLeave(const MouseLeaveEvent& e) override;
//
//	virtual void OnMouseWheel(const MouseWheelEvent& e) override {}
//
//	virtual void OnKeyDown(const KeyDownEvent& e)  override;
//	virtual void OnKeyUp(const KeyUpEvent& e) override;
//	virtual void OnKeyTraceDown(const KeyTraceDownEvent& e)  override;
//	virtual void OnKeyTraceUp(const KeyTraceUpEvent& e) override;
//	virtual void OnSysKeyDown(const SysKeyDownEvent& e)  override {}
//	virtual void OnChar(const CharEvent& e)  override;
//	virtual void OnImeStartComposition(const ImeStartCompositionEvent& e)  override;
//	
//	virtual void OnContextMenu(const ContextMenuEvent& e) override;
//	virtual void OnSetFocus(const SetFocusEvent& e) override;
//	virtual void OnSetCursor(const SetCursorEvent& e) override;
//	virtual void OnKillFocus(const KillFocusEvent& e) override;
//
//	//String
//	virtual std::wstring GetString() override {return CSheet::GetSheetString();}
//	virtual void SetString(const std::wstring& str, bool notify)  override {/*Do Nothing*/}
//	virtual bool Filter(const std::wstring& strFilter);
//
//	virtual CPointF GetScrollPos()const;
//	virtual CGridView* GetGridPtr();
//
//	void OnCellPropertyChanged(CCell* pCell, const wchar_t* name) override;
//
//	virtual void ColumnInserted(const CColumnEventArgs& e) override;
//	virtual void ColumnErased(const CColumnEventArgs& e) override;
////	virtual void ColumnHeaderEndTrack(const CColumnEventArgs& e) override;
//	virtual void RowInserted(const CRowEventArgs& e) override;
//	virtual void RowErased(const CRowEventArgs& e) override;
//
//	virtual CColumn* GetParentColumnPtr(CCell* pCell) override;
//	virtual void OnPropertyChanged(const wchar_t* name) override;
//	virtual void OnRowPropertyChanged(CRow* pRow, const wchar_t* name) override;
//	virtual void OnColumnPropertyChanged(CColumn* pCol, const wchar_t* name) override;
//
//
//	//virtual SizingType GetRowSizingType()const { return SizingType::Depend; }
//	//virtual SizingType GetColSizingType()const { return SizingType::Depend; }
//
//	virtual void UpdateRow()override;
//	virtual void UpdateColumn()override;
//
//};