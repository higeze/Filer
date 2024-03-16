#pragma once
#include "MyWnd.h"
#include "D2DWControl.h"
#include "Sheet.h"
//#include "MyGdiPlusHelper.h"
#include "ThreadHelper.h"
#include <queue>
#include "DeadlineTimer.h"
#include <boost/sml.hpp>
#include <sigslot/signal.hpp>

namespace sml = boost::sml;

class CDirect2DWrite;
struct BackgroundProperty;
struct GridViewProperty;
class CMouseStateMachine;

class CVScroll;
class CHScroll;
class CCellTextBox;

struct GridViewStateMachine;

enum class GridViewMode
{
	None,
	ExcelLike,
};

class CGridView: public CSheet
{
public:
	std::unique_ptr<CVScroll> m_pVScroll;
	std::unique_ptr<CHScroll> m_pHScroll;

	static CMenu ContextMenu;
protected:
	std::shared_ptr<CCellTextBox> m_pEdit = nullptr;
	std::shared_ptr<CCell> m_pJumpCell = nullptr;
	CRectF m_rcUpdateRect;
	bool m_isUpdating = false;
	bool m_isEditExiting = false;
	GridViewMode m_gridViewMode = GridViewMode::None;
	std::vector<std::pair<size_t, size_t>> m_frozenTabStops = { std::make_pair(0, 1), std::make_pair(2, 1), std::make_pair(3, 1)};
	//std::vector<std::pair<size_t, size_t>> m_tabStops = { std::make_pair(3, 1) };

protected:
	CDeadlineTimer m_invalidateTimer;

	std::shared_ptr<GridViewProperty> m_spGridViewProp;
public:

	sigslot::signal<> SignalPreDelayUpdate;
	void DelayUpdate();

public:
	//Constructor
	CGridView(
		CD2DWControl* pParentControl,
		const std::shared_ptr<GridViewProperty>& spGridViewProp,
		CMenu* pContextMenu= &CGridView::ContextMenu);
	//Destructor
	virtual ~CGridView();

	//Getter Setter
	std::shared_ptr<GridViewProperty>& GetGridViewPropPtr() { return m_spGridViewProp; }
	//CRectF GetUpdateRect()const { return m_rcUpdateRect; }
	//void SetUpdateRect(CRectF rcUpdateRect) { m_rcUpdateRect = rcUpdateRect; }
	std::shared_ptr<CCellTextBox> GetEditPtr() { return m_pEdit; }
	void SetEditPtr(std::shared_ptr<CCellTextBox> pEdit) { m_pEdit = pEdit; }

protected:
	virtual void OnCellLButtonClk(const CellEventArgs& e) {}
	virtual void OnCellContextMenu(const CellContextMenuEventArgs& e) {}
	/******************/
	/* Window Message */
	/******************/
public:

	virtual void OnFilter();
	virtual void OnDelayUpdate();

	virtual void OnCommandEditHeader(const CommandEvent& e);
	virtual void OnCommandDeleteColumn(const CommandEvent& e);
	virtual void OnCommandResizeSheetCell(const CommandEvent& e);
	virtual void OnCommandSelectAll(const CommandEvent& e);
	virtual void OnCommandDelete(const CommandEvent& e) { }
	virtual void OnCommandCopy(const CommandEvent& e);
	virtual void OnCommandCut(const CommandEvent& e) { }
	virtual void OnCommandPaste(const CommandEvent& e) { }
	virtual void OnCommandFind(const CommandEvent& e);

	/**************/
	/* UI Message */
	/**************/
	virtual void OnCreate(const CreateEvt& e) override;
	virtual void OnRect(const RectEvent& e) override;
	virtual void OnRectWoSubmit(const RectEvent& e);
	virtual	void OnPaint(const PaintEvent& e) override;
	virtual void OnMouseWheel(const MouseWheelEvent& e) override;
	virtual void OnSetFocus(const SetFocusEvent& e);
	virtual void OnKillFocus(const KillFocusEvent& e);
	virtual void OnSetCursor(const SetCursorEvent& e);
	virtual void OnContextMenu(const ContextMenuEvent& e);


	//virtual void OnCommand(const CommandEvent& e);

public:
	std::shared_ptr<CCell> TabNext(const std::shared_ptr<CCell>& spCurCell);
	std::shared_ptr<CCell> TabPrev(const std::shared_ptr<CCell>& spCurCell);
	void BeginEdit(CCell* pCell);
	void EndEdit();
	virtual void ClearFilter();
	virtual void FilterAll();

	virtual void EnsureVisibleCell(const std::shared_ptr<CCell>& pCell);
	void Jump(const std::shared_ptr<CCell>& spCell);
	virtual void Clear();

	virtual std::wstring FullXMLSave(){return std::wstring();}
	virtual void FullXMLLoad(const std::wstring& str){}

public:

	void FindNext(const std::wstring& findWord, bool matchCase, bool matchWholeWord);
	void FindPrev(const std::wstring& findWord, bool matchCase, bool matchWholeWord);

	virtual CGridView* GetGridPtr(){return this;};

	virtual CPointF GetScrollPos()const override;
	virtual void SetScrollPos(const CPoint& ptScroll);

	virtual FLOAT GetVerticalScrollPos()const;
	virtual FLOAT GetHorizontalScrollPos()const;
	virtual CRectF GetPaintRect();
	virtual CRectF GetPageRect();
	std::pair<bool, bool> GetHorizontalVerticalScrollNecessity();
	
	virtual void UpdateAll();

	virtual void SortAllInSubmitUpdate();

	virtual void UpdateRow()override;
	virtual void UpdateColumn()override;

	virtual FLOAT UpdateHeadersRow(FLOAT top);
	virtual FLOAT UpdateCellsRow(FLOAT top, FLOAT pageTop, FLOAT pageBottom);
	virtual void UpdateScrolls();
	virtual void Invalidate();

	//virtual void ColumnErased(CColumnEventArgs& e);
	virtual void SubmitUpdate();
	virtual CColumn* GetParentColumnPtr(CCell* pCell)override;	

	virtual void Arrange(const CRectF& rc) override;


public:
	//Normal
	virtual void Normal_Paint(const PaintEvent& e) override;
	virtual void Normal_LButtonDown(const LButtonDownEvent& e) override;
	virtual void Normal_LButtonUp(const LButtonUpEvent& e) override;
	virtual void Normal_LButtonClk(const LButtonClkEvent& e) override;
	virtual void Normal_LButtonSnglClk(const LButtonSnglClkEvent& e) override;
	virtual void Normal_LButtonDblClk(const LButtonDblClkEvent& e) override;
	virtual void Normal_RButtonDown(const RButtonDownEvent& e) override;
	virtual void Normal_MouseMove(const MouseMoveEvent& e) override;
	virtual void Normal_MouseLeave(const MouseLeaveEvent& e) override;
	virtual bool Normal_Guard_SetCursor(const SetCursorEvent& e) override;
	virtual void Normal_SetCursor(const SetCursorEvent& e) override;
	virtual void Normal_ContextMenu(const ContextMenuEvent& e) override;
	virtual void Normal_KeyDown(const KeyDownEvent& e) override;
	//VScrollDrag
	virtual void VScrlDrag_OnEntry();
	virtual void VScrlDrag_OnExit();
	virtual void VScrlDrag_LButtonDown(const LButtonDownEvent& e);
	virtual bool VScrlDrag_Guard_LButtonDown(const LButtonDownEvent& e);
	virtual void VScrlDrag_MouseMove(const MouseMoveEvent& e);
	//HScrollDrag
	virtual void HScrlDrag_OnEntry();
	virtual void HScrlDrag_OnExit();
	virtual void HScrlDrag_LButtonDown(const LButtonDownEvent& e);
	virtual bool HScrlDrag_Guard_LButtonDown(const LButtonDownEvent& e);
	virtual void HScrlDrag_MouseMove(const MouseMoveEvent& e);
	//Edit
	//virtual void Edit_BeginEdit(const BeginEditEvent& e);
	//virtual void Edit_EndEdit(const EndEditEvent& e);
	virtual void Edit_OnEntry(const BeginEditEvent& e);
	virtual void Edit_OnExit();
	virtual void Edit_MouseMove(const MouseMoveEvent& e);
	virtual bool Edit_Guard_LButtonDown(const LButtonDownEvent& e);
	virtual void Edit_LButtonDown(const LButtonDownEvent& e);
	virtual void Edit_LButtonUp(const LButtonUpEvent& e);
	virtual void Edit_LButtonBeginDrag(const LButtonBeginDragEvent& e);
	virtual void Edit_LButtonEndDrag(const LButtonEndDragEvent& e);
	//virtual bool Edit_Guard_KeyDown(const KeyDownEvent& e);
	virtual bool Edit_Guard_KeyDown_ToNormal_Tab(const KeyDownEvent& e, boost::sml::back::process<BeginEditEvent> process);
	virtual bool Edit_Guard_KeyDown_ToNormal(const KeyDownEvent& e);
	virtual void Edit_KeyDown_Tab(const KeyDownEvent& e, boost::sml::back::process<BeginEditEvent> process);
	virtual void Edit_KeyDown(const KeyDownEvent& e);
	virtual void Edit_Char(const CharEvent& e);
};
