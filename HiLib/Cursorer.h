#pragma once

#include "Sheet.h"
//Pre-Declaration
class CSheet;
class CCell;
class CGridView;
class ISheetState;
struct EventArgs;
struct MouseEvent;
struct SetCursorEvent;

class CCursorer
{
private:
	std::shared_ptr<CCell> m_oldCell;
	std::shared_ptr<CCell> m_currentCell;
	std::shared_ptr<CCell> m_anchorCell;
	std::shared_ptr<CCell> m_focusedCell;
	std::shared_ptr<CCell> m_doubleFocusedCell;
	bool m_isDragPossible;
public:
	CCursorer():m_isDragPossible(false){}
	virtual ~CCursorer(){}
	//Getter/Setter
	std::shared_ptr<CCell> GetFocusedCell(){return m_focusedCell;}
	void SetFocusedCell(std::shared_ptr<CCell>& cell){m_focusedCell = cell;}
	std::shared_ptr<CCell> GetDoubleFocusedCell() { return m_doubleFocusedCell; }
	//Event handler
	virtual void OnLButtonDown(CSheet* pSheet, const LButtonDownEvent& e);
	virtual void OnLButtonUp(CSheet* pSheet, const LButtonUpEvent& e);
	virtual void OnLButtonDblClk(CSheet* pSheet, const LButtonDblClkEvent& e){/*Do Nothing*/}
	virtual void OnRButtonDown(CSheet* pSheet, const RButtonDownEvent& e);
	virtual void OnMouseMove(CSheet* pSheet, const MouseMoveEvent& e) {/*Do Nothing*/ }
	virtual void OnMouseLeave(CSheet* pSheet, const MouseLeaveEvent& e);
	virtual void OnSetCursor(CSheet* pSheet, const SetCursorEvent& e) {/*Do Nothing*/ }
	virtual void OnKeyDown(CSheet* pSheet, const KeyDownEvent& e);


	virtual void OnCursorDown(std::shared_ptr<CCell>& cell);
	virtual void OnCursorUp(std::shared_ptr<CCell>& cell);
	virtual void OnCursorLeave(std::shared_ptr<CCell>& cell);
	virtual void OnCursor(std::shared_ptr<CCell>& cell);
	virtual void OnCursorCtrl(std::shared_ptr<CCell>& cell);
	virtual void OnCursorShift(std::shared_ptr<CCell>& cell);
	virtual void OnCursorCtrlShift(std::shared_ptr<CCell>& cell);
	//virtual void UpdateCursor();
private:
	virtual void UpdateCursor(std::shared_ptr<CCell>& cell, bool old  = true, bool current = true, bool anchor = true, bool focus = true);

public:
	virtual void OnCursorClear(CSheet* pSheet);

	std::vector<RC> GetFocusedRCs(CSheet* pSheet)const;
	std::vector<RC> GetSelectedRCs(CSheet* pSheet)const;
	std::vector<std::shared_ptr<CRow>> GetSelectedRows(CSheet* pSheet)const;
	std::vector<std::shared_ptr<CColumn>> GetSelectedColumns(CSheet* pSheet)const;
	void SetFocusedRCs(CSheet* pSheet, std::vector<RC> rcs);
	void SetSelectedRCs(CSheet* pSheet, std::vector<RC> rocos);
	void SetSelectedRows(CSheet* pSheet, std::vector<std::shared_ptr<CRow>> rcs);
	void SetSelectedColumns(CSheet* pSheet, std::vector<std::shared_ptr<CColumn>> cols);

	void Clear()
	{
		m_oldCell=nullptr;
		m_currentCell= nullptr;
		m_anchorCell= nullptr;
		m_focusedCell= nullptr;
		m_doubleFocusedCell = nullptr;
	}
};

class CSheetCellCursorer:public CCursorer
{
public:
	CSheetCellCursorer():CCursorer(){}
	virtual ~CSheetCellCursorer(){}
	virtual void OnLButtonDown(CSheet* pSheet, const LButtonDownEvent& e) override;
	virtual void OnRButtonDown(CSheet* pSheet, const RButtonDownEvent& e) override;
	virtual void OnKeyDown(CSheet* pSheet, const KeyDownEvent& e) override;
};