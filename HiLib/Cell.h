#pragma once
#include "UIElement.h"
#include "SheetEnums.h"
#include "named_arguments.h"

struct CellProperty;
class CRect;
class CSize;
class CGridView;
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

//#define MAKELONG(a, b)      ((LONG)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))


#define MAKELONGLONG(a, b)      ((LONGLONG)(((LONG)(((LONG_PTR)(a)) & 0xffffffff)) | ((LONGLONG)((LONG)(((LONG_PTR)(b)) & 0xffffffff))) << 32))


class CCell :virtual public CUIElement
{
protected:

	CGridView* m_pGrid;
	CRow* m_pRow;
	CColumn* m_pColumn;
	std::shared_ptr<CellProperty> m_spCellProperty;

	bool m_bSelected = false;
	bool m_bChecked = false;

	CSizeF m_fitSize = CSizeF();
	CSizeF m_actSize = CSizeF();
	bool m_isFitMeasureValid = false;
	bool m_isActMeasureValid = false;
	bool m_isWrappable = true;

public:
	//Constructor
	template<typename... Args>
	CCell(CGridView* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty, Args... args)
		:m_pGrid(pSheet),
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
	CGridView* GetGridPtr()const { return m_pGrid; }
	CRow* GetRowPtr()const { return m_pRow; }
	CColumn* GetColumnPtr()const { return m_pColumn; }
	LONGLONG GetRowColumnPtr()const { return MAKELONGLONG(m_pRow, m_pColumn); }
	virtual bool GetIsWrappable()const { return m_isWrappable; }
	void SetFitMeasureValid(const bool& b) { m_isFitMeasureValid = b; }
	void SetActMeasureValid(const bool& b) { m_isActMeasureValid = b; }
	std::shared_ptr<CellProperty> GetCellPropertyPtr() { return m_spCellProperty; }

	//Size, Rect method
	virtual CSizeF GetInitSize(CDirect2DWrite* pDirect);
	virtual CSizeF GetFitSize(CDirect2DWrite* pDirect);
	virtual CSizeF GetActSize(CDirect2DWrite* pDirect);
	virtual CSizeF MeasureContentSize(CDirect2DWrite* pDirect);
	virtual CSizeF MeasureContentSizeWithFixedWidth(CDirect2DWrite* pDirect);
	virtual CSizeF MeasureSize(CDirect2DWrite* pDdirect);
	virtual CSizeF MeasureSizeWithFixedWidth(CDirect2DWrite* pDirect);
	virtual CRectF CenterBorder2InnerBorder(CRectF rcCenter);
	virtual CRectF InnerBorder2Content(CRectF rcInner);
	virtual CRectF Content2InnerBorder(CRectF rcContent);
	virtual CRectF InnerBorder2CenterBorder(CRectF rcInner);

	virtual FLOAT GetLeft()const;
	virtual FLOAT GetTop()const;
	virtual CRectF GetRectInWnd()const override;

	//Visible
	bool GetIsVisible()const;
	//Selected
	virtual bool GetIsSelected()const;
	virtual void SetIsSelected(const bool& selected);
	//Focused
	virtual bool GetIsFocused()const;
	//DoubleFocused
	virtual bool GetIsDoubleFocused()const;
	//Checked
	virtual bool GetIsChecked()const;
	virtual void SetIsChecked(const bool& bChecked);

	//Paint
	virtual void PaintBackground(CDirect2DWrite* pDirect, CRectF rc);
	virtual void PaintNormalBackground(CDirect2DWrite* pDirect, CRectF rc);
	virtual void PaintSelectedBackground(CDirect2DWrite* pDirect, CRectF rc);
	virtual void PaintHotBackground(CDirect2DWrite* pDirect, CRectF rc);
	virtual void PaintLine(CDirect2DWrite* pDirect, CRectF rc);
	virtual void PaintContent(CDirect2DWrite* pDirect, CRectF rc) {/*Do Nothing*/ }
	virtual void PaintFocus(CDirect2DWrite* pDirect, CRectF rc);

	//Event
	virtual void OnPaint(const PaintEvent& e);
	virtual void OnRButtonDown(const RButtonDownEvent& e) override {}
	virtual void OnLButtonDown(const LButtonDownEvent& e) override;
	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e) override {/*Do Nothing*/ }
	virtual void OnLButtonUp(const LButtonUpEvent& e) override;
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e) override;
	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) override {/*Do Nothing*/ }

	virtual void OnContextMenu(const ContextMenuEvent& e) override {/*Do Nothing*/ };
	virtual void OnSetFocus(const SetFocusEvent& e) override;
	virtual void OnSetCursor(const SetCursorEvent& e) override { *e.HandledPtr = FALSE; /*Do Nothing*/ };
	virtual void OnKillFocus(const KillFocusEvent& e) override;
	virtual void OnKeyDown(const KeyDownEvent& e) override { *e.HandledPtr = FALSE; /*Do Nothing*/ };
	virtual void OnChar(const CharEvent& e) override {/*Do Nothing*/ };

	//String
	virtual std::wstring GetString();
	virtual std::wstring GetSortString() { return GetString(); }

	virtual void SetString(const std::wstring& str, bool notify = true);
	virtual void SetStringCore(const std::wstring& str) {/*Do Nothing*/ };
	virtual bool Filter(const std::wstring& strFilter);

	//Clipboard
	virtual bool GetIsClipboardCopyable()const { return false; }

	//Update action
	virtual void OnPropertyChanged(const wchar_t*) override;
};
