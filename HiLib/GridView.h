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
class D2DTextbox;

namespace d2dw
{
	class CVScroll;
	class CHScroll;
}

struct GridViewStateMachine;

class CGridView:public CWnd, public CSheet
{
	friend class CMouseStateMachine;
protected:
	std::unique_ptr<CMouseStateMachine> m_pMouseMachine;

public:
	std::unique_ptr<d2dw::CVScroll> m_pVScroll;
	std::unique_ptr<d2dw::CHScroll> m_pHScroll;

	static CMenu ContextMenu;
protected:
	D2DTextbox* m_pEdit = nullptr;
	bool m_isFocusable = true;

	d2dw::CRectF m_rcUpdateRect;
	bool m_isUpdating = false;

	std::shared_ptr<d2dw::CDirect2DWrite> m_pDirect;

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
		std::shared_ptr<GridViewProperty>& spGridViewProp,
		CMenu* pContextMenu= &CGridView::ContextMenu);
	//Destructor
	virtual ~CGridView();

	//Getter Setter
	d2dw::CDirect2DWrite* GetDirectPtr() override { return m_pDirect.get(); }
	std::shared_ptr<GridViewProperty>& GetGridViewPropPtr() { return m_spGridViewProp; }
	d2dw::CRectF GetUpdateRect()const { return m_rcUpdateRect; }
	void SetUpdateRect(d2dw::CRectF rcUpdateRect) { m_rcUpdateRect = rcUpdateRect; }
	D2DTextbox* GetEditPtr() { return m_pEdit; }
	void SetEditPtr(D2DTextbox* pEdit) { m_pEdit = pEdit; }

protected:
	virtual std::shared_ptr<CDC> GetClientDCPtr()const;
	virtual void OnCellLButtonClk(CellEventArgs& e);
	virtual void OnCellContextMenu(CellContextMenuEventArgs& e);
	/******************/
	/* Window Message */
	/******************/
	template<typename T>
	LRESULT UserInputMachine_Message(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_pMouseMachine->process_event(T(this, wParam, lParam, &bHandled));
		SubmitUpdate();
		return 0;
	}

	virtual LRESULT OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnClose(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnDestroy(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnEraseBkGnd(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnSize(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnPaint(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnSetCursor(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnContextMenu(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnFilter(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	//virtual LRESULT OnLButtonDblClkTimeExceed(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnDelayUpdate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	virtual LRESULT OnCommandEditHeader(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);
	virtual LRESULT OnCommandDeleteColumn(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& sc);
	virtual LRESULT OnCommandResizeSheetCell(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);
	virtual LRESULT OnCommandSelectAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	virtual LRESULT OnCommandDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) { return 0; }
	virtual LRESULT OnCommandCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	virtual LRESULT OnCommandPaste(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) { return 0; }
	virtual LRESULT OnCommandFind(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	/**************/
	/* UI Message */
	/**************/
	virtual void OnMouseWheel(const MouseWheelEvent& e);


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
