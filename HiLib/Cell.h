#pragma once
#include "UIElement.h"
#include "SheetEnums.h"
#include "MySize.h"
#include "MyRect.h"

class CCellProperty;
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
	typedef int size_type;
	typedef int coordinates_type;
	typedef std::wstring string_type;

	CSheet* m_pSheet;
	CRow* m_pRow;
	CColumn* m_pColumn;
	std::shared_ptr<CCellProperty> m_spProperty;

	bool m_bSelected = false;
	bool m_bChecked = false;


	CSize m_fitSize = CSize();
	CSize m_actSize = CSize();
	bool m_bFitMeasureValid = false;
	bool m_bActMeasureValid = false;

	LineType m_lineType = LineType::MultiLine;

public:
	//Constructor
	CCell(CSheet* pSheet = nullptr, CRow* pRow = nullptr, CColumn* pColumn = nullptr, std::shared_ptr<CCellProperty> spProperty = nullptr, CMenu* pContextMenu= &CCell::ContextMenu);
	//Destrucor
	virtual ~CCell(){}

	//Operator
	bool operator<(CCell& rhs);
	bool operator>(CCell& rhs);
	//Accesser
	CSheet* GetSheetPtr()const { return m_pSheet; }
	CRow* GetRowPtr()const { return m_pRow; }
	CColumn* GetColumnPtr()const { return m_pColumn; }
	LineType GetLineType()const { return m_lineType; }
	void SetFitMeasureValid(const bool& b) { m_bFitMeasureValid = b; }
	void SetActMeasureValid(const bool& b) { m_bActMeasureValid = b; }
	std::shared_ptr<CCellProperty> GetPropertyPtr() { return m_spProperty; }

	//Size, Rect method
	virtual CSize GetInitSize(CDC* pDC);
	virtual CSize GetFitSize(CDC* pDC);
	virtual CSize GetActSize(CDC* pDC);
	virtual CSize MeasureContentSize(CDC* pDC);
	virtual CSize MeasureContentSizeWithFixedWidth(CDC* pDC);
	virtual CSize MeasureSize(CDC* pDC);
	virtual CSize MeasureSizeWithFixedWidth(CDC* pDC);
	virtual CRect CenterBorder2InnerBorder(CRect rcCenter);
	virtual CRect InnerBorder2Content(CRect rcInner);
	virtual CRect Content2InnerBorder(CRect rcContent);
	virtual CRect InnerBorder2CenterBorder(CRect rcInner);

	virtual coordinates_type GetLeft()const;
	virtual coordinates_type GetTop()const;
	virtual CRect GetRect()const;

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
	virtual void PaintBackground(CDC* pDC, CRect rc);
	virtual void PaintLine(CDC* pDC, CRect rc);
	virtual void PaintContent(CDC* pDC, CRect rc){/*Do Nothing*/}
	virtual void PaintFocus(CDC* pDC, CRect rc);
	
	//Event
	virtual void OnPaint(const PaintEvent& e);
	virtual void OnLButtonDown(const LButtonDownEvent& e);
	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e) {/*Do Nothing*/ }
	virtual void OnLButtonUp(const LButtonUpEvent& e);
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e);

	virtual void OnContextMenu(const ContextMenuEvent& e);
	virtual void OnSetFocus(const SetFocusEvent& e);
	virtual void OnSetCursor(const SetCursorEvent& e){/*Do Nothing*/}
	virtual void OnKillFocus(const KillFocusEvent& e);
	virtual void OnKeyDown(const KeyDownEvent& e){/*Do Nothing*/};

	//String
	virtual string_type GetString();
	virtual string_type GetSortString(){return GetString();}

	virtual void SetString(const string_type& str);
	virtual void SetStringNotify(const string_type& str);
	virtual void SetStringCore(const string_type& str){/*Do Nothing*/};
	virtual bool Filter(const string_type& strFilter);

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
