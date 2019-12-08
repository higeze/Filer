#pragma once
#include "UIElement.h"
#include "SheetEnums.h"

class CellProperty;
class CRect;
class CSize;
class CDC;
class CSheet;

class CCell;
class CEmptyCell;
class CTextCell;
class CSheetCell;
class CColumn;
class CRow;
class CContextMenuEventArgs;

class CMenu;

struct CellEventArgs;
struct CellContextMenuEventArgs;

class CCell:virtual public CUIElement
{
public:
	static CMenu ContextMenu;

protected:

	CSheet* m_pSheet;
	CRow* m_pRow;
	CColumn* m_pColumn;
	std::shared_ptr<CellProperty> m_spProperty;

	bool m_bSelected = false;
	bool m_bChecked = false;


	d2dw::CSizeF m_fitSize = d2dw::CSizeF();
	d2dw::CSizeF m_actSize = d2dw::CSizeF();
	bool m_bFitMeasureValid = false;
	bool m_bActMeasureValid = false;

	//LineType m_lineType = LineType::MultiLine;

public:
	//Constructor
	CCell(CSheet* pSheet = nullptr, CRow* pRow = nullptr, CColumn* pColumn = nullptr, std::shared_ptr<CellProperty> spProperty = nullptr, CMenu* pContextMenu= &CCell::ContextMenu);
	//Destructor
	virtual ~CCell(){}

	//Operator
	bool operator<(CCell& rhs);
	bool operator>(CCell& rhs);
	//Accesser
	CSheet* GetSheetPtr()const { return m_pSheet; }
	CRow* GetRowPtr()const { return m_pRow; }
	CColumn* GetColumnPtr()const { return m_pColumn; }
	//LineType GetLineType()const { return m_lineType; }
	virtual SizingType GetRowSizingType()const { return SizingType::Fit; }
	virtual SizingType GetColSizingType()const { return SizingType::Independ; }
	void SetFitMeasureValid(const bool& b) { m_bFitMeasureValid = b; }
	void SetActMeasureValid(const bool& b) { m_bActMeasureValid = b; }
	std::shared_ptr<CellProperty> GetPropertyPtr() { return m_spProperty; }

	//Size, Rect method
	virtual d2dw::CSizeF GetInitSize(d2dw::CDirect2DWrite& direct);
	virtual d2dw::CSizeF GetFitSize(d2dw::CDirect2DWrite& direct);
	virtual d2dw::CSizeF GetActSize(d2dw::CDirect2DWrite& direct);
	virtual d2dw::CSizeF MeasureContentSize(d2dw::CDirect2DWrite& direct);
	virtual d2dw::CSizeF MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite& direct);
	virtual d2dw::CSizeF MeasureSize(d2dw::CDirect2DWrite& direct);
	virtual d2dw::CSizeF MeasureSizeWithFixedWidth(d2dw::CDirect2DWrite& direct);
	virtual d2dw::CRectF CenterBorder2InnerBorder(d2dw::CRectF rcCenter);
	virtual d2dw::CRectF InnerBorder2Content(d2dw::CRectF rcInner);
	virtual d2dw::CRectF Content2InnerBorder(d2dw::CRectF rcContent);
	virtual d2dw::CRectF InnerBorder2CenterBorder(d2dw::CRectF rcInner);

	virtual FLOAT GetLeft()const;
	virtual FLOAT GetTop()const;
	virtual d2dw::CRectF GetRect()const;

	//Selected
	virtual bool GetSelected()const;
	virtual void SetSelected(const bool& selected);

	//Focused
	virtual bool GetFocused()const;

	//DoubleFocused
	virtual bool GetDoubleFocused()const;

	//Checked
	virtual bool GetChecked()const;
	virtual void SetChecked(const bool& bChecked);
	
	//Paint
	virtual void PaintBackground(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rc);
	virtual void PaintLine(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rc);
	virtual void PaintContent(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rc){/*Do Nothing*/}
	virtual void PaintFocus(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rc);
	
	//Event
	virtual void OnPaint(const PaintEvent& e);
	virtual void OnLButtonDown(const LButtonDownEvent& e) override;
	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e) override {/*Do Nothing*/ }
	virtual void OnLButtonUp(const LButtonUpEvent& e) override;
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e) override;

	virtual void OnContextMenu(const ContextMenuEvent& e) override;
	virtual void OnSetFocus(const SetFocusEvent& e);
	virtual void OnSetCursor(const SetCursorEvent& e){/*Do Nothing*/}
	virtual void OnKillFocus(const KillFocusEvent& e);
	virtual void OnKeyDown(const KeyDownEvent& e) override {/*Do Nothing*/};

	//String
	virtual std::wstring GetString();
	virtual std::wstring GetSortString(){return GetString();}

	virtual void SetString(const std::wstring& str);
	virtual void SetStringNotify(const std::wstring& str);
	virtual void SetStringCore(const std::wstring& str){/*Do Nothing*/};
	virtual bool Filter(const std::wstring& strFilter);

	//Compare
	virtual bool IsComparable()const=0;
	virtual Compares EqualCell(CCell* pCell, std::function<void(CCell*, Compares)> action);
	virtual Compares EqualCell(CEmptyCell* pCell, std::function<void(CCell*, Compares)> action);
	virtual Compares EqualCell(CTextCell* pCell, std::function<void(CCell*, Compares)> action);
	virtual Compares EqualCell(CSheetCell* pCell, std::function<void(CCell*, Compares)> action);

	//Menu
	CMenu* const m_pContextMenu;
	virtual CMenu* GetContextMenuPtr(){return m_pContextMenu;}

	//Clipboard
	virtual bool IsClipboardCopyable()const{return false;}

	//Update action
	virtual void OnPropertyChanged(const wchar_t*) override;
};
