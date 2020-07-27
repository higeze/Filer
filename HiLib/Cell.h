#pragma once
#include "UIElement.h"
#include "SheetEnums.h"
#include "named_arguments.h"

class CellProperty;
class CRect;
class CSize;
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
protected:

	CSheet* m_pSheet;
	CRow* m_pRow;
	CColumn* m_pColumn;
	std::shared_ptr<CellProperty> m_spCellProperty;

	bool m_bSelected = false;
	bool m_bChecked = false;

	d2dw::CSizeF m_fitSize = d2dw::CSizeF();
	d2dw::CSizeF m_actSize = d2dw::CSizeF();
	bool m_isFitMeasureValid = false;
	bool m_isActMeasureValid = false;
	bool m_isWrappable = true;

public:
	//Constructor
	template<typename... Args>
	CCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty, Args... args)
		:CUIElement(),
		m_pSheet(pSheet),
		m_pRow(pRow),
		m_pColumn(pColumn),
		m_spCellProperty(spProperty)
	{
		m_isWrappable = ::get(arg<"iswrap"_s>(), args..., default_(true));
	}
	//Destructor
	virtual ~CCell() = default;
	//Operator
	bool operator<(CCell& rhs);
	bool operator>(CCell& rhs);
	//Accesser
	CSheet* GetSheetPtr()const { return m_pSheet; }
	CRow* GetRowPtr()const { return m_pRow; }
	CColumn* GetColumnPtr()const { return m_pColumn; }
	virtual bool GetIsWrappable()const { return m_isWrappable; }
	void SetFitMeasureValid(const bool& b) { m_isFitMeasureValid = b; }
	void SetActMeasureValid(const bool& b) { m_isActMeasureValid = b; }
	std::shared_ptr<CellProperty> GetCellPropertyPtr() { return m_spCellProperty; }

	//Size, Rect method
	virtual d2dw::CSizeF GetInitSize(d2dw::CDirect2DWrite* pDirect);
	virtual d2dw::CSizeF GetFitSize(d2dw::CDirect2DWrite* pDirect);
	virtual d2dw::CSizeF GetActSize(d2dw::CDirect2DWrite* pDirect);
	virtual d2dw::CSizeF MeasureContentSize(d2dw::CDirect2DWrite* pDirect);
	virtual d2dw::CSizeF MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite* pDirect);
	virtual d2dw::CSizeF MeasureSize(d2dw::CDirect2DWrite* pDdirect);
	virtual d2dw::CSizeF MeasureSizeWithFixedWidth(d2dw::CDirect2DWrite* pDirect);
	virtual d2dw::CRectF CenterBorder2InnerBorder(d2dw::CRectF rcCenter);
	virtual d2dw::CRectF InnerBorder2Content(d2dw::CRectF rcInner);
	virtual d2dw::CRectF Content2InnerBorder(d2dw::CRectF rcContent);
	virtual d2dw::CRectF InnerBorder2CenterBorder(d2dw::CRectF rcInner);

	virtual FLOAT GetLeft()const;
	virtual FLOAT GetTop()const;
	virtual d2dw::CRectF GetRect()const;

	//Visible
	bool GetIsVisible()const;
	//Selected
	virtual bool GetIsSelected()const;
	virtual void SetIsSelected(const bool& selected);
	//Focused
	virtual bool GetIsFocused()const override;
	//DoubleFocused
	virtual bool GetIsDoubleFocused()const;
	//Checked
	virtual bool GetIsChecked()const;
	virtual void SetIsChecked(const bool& bChecked);

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
	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) override {/*Do Nothing*/ }

	virtual void OnContextMenu(const ContextMenuEvent& e) override {/*Do Nothing*/ };
	virtual void OnSetFocus(const SetFocusEvent& e) override;
	virtual void OnSetCursor(const SetCursorEvent& e) override { e.Handled = FALSE; /*Do Nothing*/ };
	virtual void OnKillFocus(const KillFocusEvent& e) override;
	virtual void OnKeyDown(const KeyDownEvent& e) override { e.Handled = FALSE; /*Do Nothing*/ };
	virtual void OnChar(const CharEvent& e) override {/*Do Nothing*/ };

	//String
	virtual std::wstring GetString();
	virtual std::wstring GetSortString(){return GetString();}

	virtual void SetString(const std::wstring& str, bool notify = true);
	virtual void SetStringCore(const std::wstring& str){/*Do Nothing*/};
	virtual bool Filter(const std::wstring& strFilter);

	//Clipboard
	virtual bool GetIsClipboardCopyable()const{return false;}

	//Update action
	virtual void OnPropertyChanged(const wchar_t*) override;
};
