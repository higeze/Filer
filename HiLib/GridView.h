#pragma once
#include "MyWnd.h"
#include "Sheet.h"
//#include "MyGdiPlusHelper.h"
#include "ThreadHelper.h"
#include <queue>
#include "DeadlineTimer.h"
#include <boost/sml.hpp>

namespace sml = boost::sml;

class CDirect2DWrite;
struct BackgroundProperty;
struct GridViewProperty;
class CMouseStateMachine;
class CTextBox;

namespace d2dw
{
	class CVScroll;
	class CHScroll;
}

struct GridViewStateMachine;

class CGridView: public CSheet
{
public:
	std::unique_ptr<d2dw::CVScroll> m_pVScroll;
	std::unique_ptr<d2dw::CHScroll> m_pHScroll;

	static CMenu ContextMenu;
protected:
	CWnd* m_pWnd;
	CTextBox* m_pEdit = nullptr;
	bool m_isFocusable = true;
	d2dw::CRectF m_rect;
	d2dw::CRectF m_rcUpdateRect;
	bool m_isUpdating = false;
protected:
	CDeadlineTimer m_invalidateTimer;

	std::shared_ptr<GridViewProperty> m_spGridViewProp;
public:

	boost::signals2::signal<void()> SignalPreDelayUpdate;
	static UINT WM_DELAY_UPDATE;
	void DelayUpdate();

public:
	//Constructor
	CGridView(
		CWnd* pWnd,
		std::shared_ptr<GridViewProperty>& spGridViewProp,
		CMenu* pContextMenu= &CGridView::ContextMenu);
	//Destructor
	virtual ~CGridView();

	//Getter Setter
	std::shared_ptr<GridViewProperty>& GetGridViewPropPtr() { return m_spGridViewProp; }
	d2dw::CRectF GetUpdateRect()const { return m_rcUpdateRect; }
	void SetUpdateRect(d2dw::CRectF rcUpdateRect) { m_rcUpdateRect = rcUpdateRect; }
	CTextBox* GetEditPtr() { return m_pEdit; }
	void SetEditPtr(CTextBox* pEdit) { m_pEdit = pEdit; }
	virtual CWnd* GetWndPtr()const override { return m_pWnd; }
	d2dw::CRectF GetRectInWnd()const { return m_rect; }


protected:
	virtual void OnCellLButtonClk(CellEventArgs& e) {}
	virtual void OnCellContextMenu(CellContextMenuEventArgs& e) {}
	/******************/
	/* Window Message */
	/******************/
public:
	template<typename T>
	LRESULT UserInputMachine_Message(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_pMouseMachine->process_event(T(this, wParam, lParam, &bHandled));
		SubmitUpdate();
		return 0;
	}

	virtual LRESULT OnFilter(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnDelayUpdate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

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
	virtual void OnRect(const RectEvent& e) override;
	virtual	void OnPaint(const PaintEvent& e) override;
	virtual void OnMouseWheel(const MouseWheelEvent& e) override;
	virtual void OnSetFocus(const SetFocusEvent& e);
	virtual void OnKillFocus(const KillFocusEvent& e);
	virtual void OnSetCursor(const SetCursorEvent& e);
	virtual void OnContextMenu(const ContextMenuEvent& e);
	virtual void OnCommand(const CommandEvent& e);

public:
	void BeginEdit(CCell* pCell);
	void EndEdit();
	virtual void ClearFilter();
	virtual void FilterAll();

	virtual void EnsureVisibleCell(const std::shared_ptr<CCell>& pCell);
	void Jump(std::shared_ptr<CCell>& spCell);
	virtual void Clear();

	virtual std::wstring FullXMLSave(){return std::wstring();}
	virtual void FullXMLLoad(const std::wstring& str){}

public:

	void FindNext(const std::wstring& findWord, bool matchCase, bool matchWholeWord);
	void FindPrev(const std::wstring& findWord, bool matchCase, bool matchWholeWord);

	virtual CGridView* GetGridPtr(){return this;};

	virtual d2dw::CPointF GetScrollPos()const override;
	virtual void SetScrollPos(const CPoint& ptScroll);

	virtual FLOAT GetVerticalScrollPos()const;
	virtual FLOAT GetHorizontalScrollPos()const;
	virtual d2dw::CRectF GetPaintRect();
	virtual d2dw::CRectF GetPageRect();
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
	virtual bool GetIsFocused()const;


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
	virtual void VScrlDrag_LButtonUp(const LButtonUpEvent& e);
	virtual void VScrlDrag_MouseMove(const MouseMoveEvent& e);
	//HScrollDrag
	virtual void HScrlDrag_OnEntry();
	virtual void HScrlDrag_OnExit();
	virtual void HScrlDrag_LButtonDown(const LButtonDownEvent& e);
	virtual bool HScrlDrag_Guard_LButtonDown(const LButtonDownEvent& e);
	virtual void HScrlDrag_LButtonUp(const LButtonUpEvent& e);
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
	virtual bool Edit_Guard_KeyDown(const KeyDownEvent& e);
	virtual bool Edit_Guard_KeyDownWithNormal(const KeyDownEvent& e);
	virtual bool Edit_Guard_KeyDownWithoutNormal(const KeyDownEvent& e);
	virtual void Edit_KeyDown(const KeyDownEvent& e);
	virtual void Edit_Char(const CharEvent& e);
};
