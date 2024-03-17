#pragma once

#include "GridView.h"
//Pre-Declaration
class CGridView;
class CCell;
class CGridView;
class ISheetState;
struct Event;
struct MouseEvent;
struct SetCursorEvent;

class CCursorer
{
protected:
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
	void SetFocusedCell(const std::shared_ptr<CCell>& cell){m_focusedCell = cell;}
	std::shared_ptr<CCell> GetCurrentCell(){return m_currentCell;}
	void SetCurrentCell(const std::shared_ptr<CCell>& cell){m_currentCell = cell;}
	std::shared_ptr<CCell> GetDoubleFocusedCell() { return m_doubleFocusedCell; }
	//Event handler
	virtual void OnLButtonDown(CGridView* pSheet, const LButtonDownEvent& e);
	virtual void OnLButtonUp(CGridView* pSheet, const LButtonUpEvent& e);
	virtual void OnLButtonDblClk(CGridView* pSheet, const LButtonDblClkEvent& e){/*Do Nothing*/}
	virtual void OnRButtonDown(CGridView* pSheet, const RButtonDownEvent& e);
	virtual void OnMouseMove(CGridView* pSheet, const MouseMoveEvent& e) {/*Do Nothing*/ }
	virtual void OnMouseLeave(CGridView* pSheet, const MouseLeaveEvent& e);
	virtual void OnSetCursor(CGridView* pSheet, const SetCursorEvent& e) {/*Do Nothing*/ }
	virtual void OnKeyDown(CGridView* pSheet, const KeyDownEvent& e);


	virtual void OnCursorDown(const std::shared_ptr<CCell>& cell);
	virtual void OnCursorUp(const std::shared_ptr<CCell>& cell);
	virtual void OnCursorLeave(const std::shared_ptr<CCell>& cell);
	virtual void OnCursor(const std::shared_ptr<CCell>& cell);
	virtual void OnCursorCtrl(const std::shared_ptr<CCell>& cell);
	virtual void OnCursorShift(const std::shared_ptr<CCell>& cell);
	virtual void OnCursorCtrlShift(const std::shared_ptr<CCell>& cell);
	//virtual void UpdateCursor();
protected:
	virtual bool IsCursorTargetCell(const std::shared_ptr<CCell>& cell);
	virtual void UpdateCursor(const std::shared_ptr<CCell>& cell, bool old  = true, bool current = true, bool anchor = true, bool focus = true);

public:
	virtual void OnCursorClear(CGridView* pSheet);

	std::vector<Indexes> GetFocusedRCs(CGridView* pSheet)const;
	std::vector<Indexes> GetSelectedRCs(CGridView* pSheet)const;
	std::vector<std::shared_ptr<CRow>> GetSelectedRows(CGridView* pSheet)const;
	std::vector<std::shared_ptr<CColumn>> GetSelectedColumns(CGridView* pSheet)const;
	void SetFocusedRCs(CGridView* pSheet, std::vector<Indexes> rcs);
	void SetSelectedRCs(CGridView* pSheet, std::vector<Indexes> rocos);
	void SetSelectedRows(CGridView* pSheet, std::vector<std::shared_ptr<CRow>> rcs);
	void SetSelectedColumns(CGridView* pSheet, std::vector<std::shared_ptr<CColumn>> cols);

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
	virtual void OnLButtonDown(CGridView* pSheet, const LButtonDownEvent& e) override;
	virtual void OnRButtonDown(CGridView* pSheet, const RButtonDownEvent& e) override;
	virtual void OnKeyDown(CGridView* pSheet, const KeyDownEvent& e) override;
};

class CExcelLikeCursorer :public CCursorer
{
	using CCursorer::CCursorer;
	virtual void OnKeyDown(CGridView* pSheet, const KeyDownEvent& e) override;
};