#pragma once
#include "MyWnd.h"
#include "Sheet.h"
//#include "MyGdiPlusHelper.h"
#include "ThreadHelper.h"
#include <queue>
#include "DeadlineTimer.h"
#include <boost/sml.hpp>
#include "GridViewStateMachine.h"

namespace sml = boost::sml;



class CDirect2DWrite;
class BackgroundProperty;
struct GridViewProperty;
struct CMouseStateMachine;
class D2DTextbox;

namespace d2dw
{
	class CVScroll;
	class CHScroll;
}

struct Machine;

class CGridView:public CWnd,public CSheet
{
public:
	//struct StateMachineImpl;
	//std::unique_ptr<StateMachineImpl> m_pImpl;

	std::unique_ptr<d2dw::CVScroll> m_pVScroll;
	std::unique_ptr<d2dw::CHScroll> m_pHScroll;

	static CMenu ContextMenu;
protected:
	D2DTextbox* m_pEdit = nullptr;
	bool m_isFocusable = true;

	d2dw::CRectF m_rcUpdateRect;
	bool m_isUpdating = false;

	std::shared_ptr<CRow> m_rowHeaderHeader; /**< Header Header row */
	std::shared_ptr<CRow> m_rowNameHeader; /**< Name Header row */
	std::shared_ptr<CRow> m_rowFilter; /**< Filter row */

	std::shared_ptr<CMouseStateMachine> m_pMouseStateMachine;

	std::shared_ptr<d2dw::CDirect2DWrite> m_pDirect;

protected:
	CDeadlineTimer m_invalidateTimer;

	std::shared_ptr<GridViewProperty> m_spGridViewProp;
public:

	boost::signals2::signal<void(CColumn*)> SignalColumnInserted;
	boost::signals2::signal<void(CColumn*)> SignalColumnErased;
	boost::signals2::signal<void(CColumn*, int, int)> SignalColumnMoved;
	boost::signals2::signal<void()> SignalPreDelayUpdate;
	static UINT WM_DELAY_UPDATE;
	void DelayUpdate();

	virtual void ColumnInserted(CColumnEventArgs& e)override;
	virtual void ColumnErased(CColumnEventArgs& e)override;
	virtual void ColumnMoved(CMovedEventArgs<ColTag>& e)override;

public:
	//Constructor
	CGridView(
		std::shared_ptr<GridViewProperty>& spGridViewProp,
		CMenu* pContextMenu= &CGridView::ContextMenu);
	//Destructor
	~CGridView();

	//Getter Setter
	d2dw::CDirect2DWrite* GetDirectPtr() override { return m_pDirect.get(); }
	std::shared_ptr<GridViewProperty>& GetGridViewPropPtr() { return m_spGridViewProp; }
	d2dw::CRectF GetUpdateRect()const { return m_rcUpdateRect; }
	void SetUpdateRect(d2dw::CRectF rcUpdateRect) { m_rcUpdateRect = rcUpdateRect; }
	std::shared_ptr<CMouseStateMachine> GetMouseStateMachine() { return m_pMouseStateMachine; }
	void SetMouseStateMachine(std::shared_ptr<CMouseStateMachine>& machine) { m_pMouseStateMachine = machine; }
	virtual std::shared_ptr<CRow> GetHeaderHeaderRowPtr()const { return m_rowHeaderHeader; }
	virtual void SetHeaderHeaderRowPtr(std::shared_ptr<CRow> row) { m_rowHeaderHeader = row; }
	virtual std::shared_ptr<CRow> GetNameHeaderRowPtr()const { return m_rowNameHeader; }
	virtual void SetNameHeaderRowPtr(std::shared_ptr<CRow> row) { m_rowNameHeader = row; }
	virtual std::shared_ptr<CRow> GetFilterRowPtr()const { return m_rowFilter; }
	virtual void SetFilterRowPtr(std::shared_ptr<CRow> row) { m_rowFilter = row; }
	D2DTextbox* GetEditPtr() { return m_pEdit; }
	void SetEditPtr(D2DTextbox* pEdit) { m_pEdit = pEdit; }
	void BeginEdit(CCell* pCell);

protected:
	virtual LRESULT OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnClose(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnDestroy(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnEraseBkGnd(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnSize(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnPaint(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnMouseWheel(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	virtual LRESULT OnRButtonDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnLButtonDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnLButtonUp(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnLButtonDblClk(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);

	virtual LRESULT OnMouseMove(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnMouseLeave(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnSetCursor(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);

	virtual LRESULT OnContextMenu(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnKeyDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);

	virtual LRESULT OnFilter(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnLButtonDblClkTimeExceed(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnDelayUpdate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	//virtual LRESULT OnCmdEnChange(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);

	virtual LRESULT OnCommandEditHeader(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);
	virtual LRESULT OnCommandDeleteColumn(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);
	virtual LRESULT OnCommandResizeSheetCell(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);
	virtual std::shared_ptr<CDC> GetClientDCPtr()const;

	virtual void OnCellLButtonClk(CellEventArgs& e);
	virtual void OnCellContextMenu(CellContextMenuEventArgs& e);

	virtual void OnLButtonDown(const LButtonDownEvent& e) override;
	virtual void OnLButtonUp(const LButtonUpEvent& e) override;
	virtual void OnLButtonClk(const LButtonClkEvent& e) override;
	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e);
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e) override;
	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) override;
	virtual void OnRButtonDown(const RButtonDownEvent& e) override;
	virtual void OnContextMenu(const ContextMenuEvent& e) override;
	virtual void OnMouseMove(const MouseMoveEvent& e) override;
	virtual void OnMouseLeave(const MouseLeaveEvent& e) override;
	virtual void OnSetCursor(const SetCursorEvent& e) override;
	virtual void OnKeyDown(const KeyDownEvent& e) override;
	virtual void OnChar(const CharEvent& e);
	virtual void OnBeginEdit(const BeginEditEvent& e);

public:
	virtual void ClearFilter();
	virtual void FilterAll();

	virtual void OnPaint(const PaintEvent& e);
	virtual void EnsureVisibleCell(const std::shared_ptr<CCell>& pCell);
	void Jump(std::shared_ptr<CCell>& spCell);
	virtual void Clear();

	virtual std::wstring FullXMLSave(){return std::wstring();}
	virtual void FullXMLLoad(const std::wstring& str){}

public:

	virtual LRESULT OnCommandSelectAll(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);
	virtual LRESULT OnCommandDelete(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled){return 0;}
	virtual LRESULT OnCommandCopy(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);
	virtual LRESULT OnCommandPaste(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled){return 0;}
	virtual LRESULT OnCommandFind(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);
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
	virtual FLOAT UpdateHeadersRow(FLOAT top);
	virtual FLOAT UpdateCellsRow(FLOAT top, FLOAT pageTop, FLOAT pageBottom);
	virtual void UpdateScrolls();
	virtual void Invalidate();

	//virtual void ColumnErased(CColumnEventArgs& e);
	virtual void SubmitUpdate();

//	Status SaveGIFWithNewColorTable(Image *pImage,IStream* pIStream,const CLSID* clsidEncoder,DWORD nColors,BOOL fTransparent);

	virtual CColumn* GetParentColumnPtr(CCell* pCell)override;	
	
	//HGLOBAL GetPaintMetaFileData();
	//HENHMETAFILE GetPaintEnhMetaFileData();
	//HENHMETAFILE GetAllEnhMetaFileData();

public:
	std::unique_ptr<boost::sml::sm<Machine>> m_pMachine;

public:
	//Normal
	void Normal_LButtonDown(const LButtonDownEvent& e);
	void Normal_LButtonUp(const LButtonUpEvent& e);
	void Normal_LButtonClk(const LButtonClkEvent& e);
	void Normal_LButtonSnglClk(const LButtonSnglClkEvent& e);
	void Normal_LButtonDblClk(const LButtonDblClkEvent& e);
	void Normal_RButtonDown(const RButtonDownEvent& e);
	void Normal_MouseMove(const MouseMoveEvent& e);
	void Normal_MouseLeave(const MouseLeaveEvent& e);
	bool Normal_Guard_SetCursor(const SetCursorEvent& e);
	void Normal_SetCursor(const SetCursorEvent& e);
	void Normal_ContextMenu(const ContextMenuEvent& e);
	void Normal_KeyDown(const KeyDownEvent& e);
	//RowTrack
	void RowTrack_LButtonDown(const LButtonDownEvent& e);
	bool RowTrack_Guard_LButtonDown(const LButtonDownEvent& e);
	void RowTrack_MouseMove(const MouseMoveEvent& e);
	void RowTrack_LButtonUp(const LButtonUpEvent& e);
	void RowTrack_MouseLeave(const MouseLeaveEvent& e);
	//ColTrack
	void ColTrack_LButtonDown(const LButtonDownEvent& e);
	bool ColTrack_Guard_LButtonDown(const LButtonDownEvent& e);
	void ColTrack_MouseMove(const MouseMoveEvent& e);
	void ColTrack_LButtonUp(const LButtonUpEvent& e);
	void ColTrack_MouseLeave(const MouseLeaveEvent& e);
	//VScrollDrag
	void VScrlDrag_OnEntry();
	void VScrlDrag_OnExit();
	void VScrlDrag_LButtonDown(const LButtonDownEvent& e);
	bool VScrlDrag_Guard_LButtonDown(const LButtonDownEvent& e);
	void VScrlDrag_LButtonUp(const LButtonUpEvent& e);
	void VScrlDrag_MouseMove(const MouseMoveEvent& e);
	//HScrollDrag
	void HScrlDrag_OnEntry();
	void HScrlDrag_OnExit();
	void HScrlDrag_LButtonDown(const LButtonDownEvent& e);
	bool HScrlDrag_Guard_LButtonDown(const LButtonDownEvent& e);
	void HScrlDrag_LButtonUp(const LButtonUpEvent& e);
	void HScrlDrag_MouseMove(const MouseMoveEvent& e);
	//RowDrag
	void RowDrag_LButtonBeginDrag(const LButtonBeginDragEvent& e);
	bool RowDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e);
	void RowDrag_MouseMove(const MouseMoveEvent& e);
	void RowDrag_LButtonUp(const LButtonUpEvent& e);
	void RowDrag_MouseLeave(const MouseLeaveEvent& e);
	//ColDrag
	void ColDrag_LButtonBeginDrag(const LButtonBeginDragEvent& e);
	bool ColDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e);
	void ColDrag_MouseMove(const MouseMoveEvent& e);
	void ColDrag_LButtonUp(const LButtonUpEvent& e);
	void ColDrag_MouseLeave(const MouseLeaveEvent& e);
	//ItemDrag
	void ItemDrag_LButtonBeginDrag(const LButtonBeginDragEvent& e);
	bool ItemDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e);
	void ItemDrag_MouseMove(const MouseMoveEvent& e);
	void ItemDrag_LButtonUp(const LButtonUpEvent& e);
	void ItemDrag_MouseLeave(const MouseLeaveEvent& e);
	//Edit
	void Edit_BeginEdit(const BeginEditEvent& e);
	void Edit_OnExit();
	void Edit_MouseMove(const MouseMoveEvent& e);
	bool Edit_Guard_LButtonDown(const LButtonDownEvent& e);
	void Edit_LButtonDown(const LButtonDownEvent& e);
	void Edit_LButtonUp(const LButtonUpEvent& e);
	bool Edit_Guard_KeyDown(const KeyDownEvent& e);
	void Edit_KeyDown(const KeyDownEvent& e);
	void Edit_Char(const CharEvent& e);







};


struct Machine
{
	class Normal {};
	class VScrlDrag {};
	class HScrlDrag {};
	class RowDrag {};
	class ColDrag {};
	class ItemDrag {};
	class RowTrack {};
	class ColTrack {};
	class Edit {};

	auto operator()() const noexcept
	{
		//auto call = [](auto... args) {return call_impl(args...); };
		auto test = call(&CGridView::Normal_LButtonDown);

		using namespace sml;
		return make_transition_table(
			*state<Normal> +event<LButtonDownEvent> / call(&CGridView::Normal_LButtonDown),
			state<Normal> +event<LButtonDownEvent>[call(&CGridView::VScrlDrag_Guard_LButtonDown)] / call(&CGridView::VScrlDrag_LButtonDown) = state<VScrlDrag>,
			state<Normal> +event<LButtonDownEvent>[call(&CGridView::HScrlDrag_Guard_LButtonDown)] / call(&CGridView::HScrlDrag_LButtonDown) = state<HScrlDrag>,
			state<Normal> +event<LButtonDownEvent>[call(&CGridView::RowTrack_Guard_LButtonDown)] / call(&CGridView::RowTrack_LButtonDown) = state<RowTrack>,
			state<Normal> +event<LButtonDownEvent>[call(&CGridView::ColTrack_Guard_LButtonDown)] / call(&CGridView::ColTrack_LButtonDown) = state<ColTrack>,

			state<Normal> +event<LButtonUpEvent> / call(&CGridView::Normal_LButtonUp),
			state<Normal> +event<LButtonClkEvent> / call(&CGridView::Normal_LButtonClk),
			state<Normal> +event<LButtonSnglClkEvent> / call(&CGridView::Normal_LButtonSnglClk),
			state<Normal> +event<LButtonDblClkEvent> / call(&CGridView::Normal_LButtonDblClk),
			state<Normal> +event<RButtonDownEvent> / call(&CGridView::Normal_RButtonDown),
			state<Normal> +event<MouseMoveEvent> / call(&CGridView::Normal_MouseMove),
			state<Normal> +event<MouseLeaveEvent> / call(&CGridView::Normal_MouseLeave),
			state<Normal> +event<ContextMenuEvent> / call(&CGridView::Normal_ContextMenu),
			state<Normal> +event<SetCursorEvent>[call(&CGridView::Normal_Guard_SetCursor)] / call(&CGridView::Normal_SetCursor),
			state<Normal> +event<KeyDownEvent> / call(&CGridView::Normal_KeyDown),

			state<Normal> +event<LButtonBeginDragEvent>[call(&CGridView::RowDrag_Guard_LButtonBeginDrag)] / call(&CGridView::RowDrag_LButtonBeginDrag) = state<RowDrag>,
			state<Normal> +event<LButtonBeginDragEvent>[call(&CGridView::ColDrag_Guard_LButtonBeginDrag)] / call(&CGridView::ColDrag_LButtonBeginDrag) = state<ColDrag>,
			state<Normal> +event<LButtonBeginDragEvent>[call(&CGridView::ItemDrag_Guard_LButtonBeginDrag)] / call(&CGridView::ItemDrag_LButtonBeginDrag) = state<ItemDrag>,
			state<Normal> +event<BeginEditEvent> / call(&CGridView::Edit_BeginEdit) = state<Edit>,


			state<VScrlDrag> +on_entry<_> / call(&CGridView::VScrlDrag_OnEntry),
			state<VScrlDrag> +on_exit<_> / call(&CGridView::VScrlDrag_OnExit),
			state<VScrlDrag> +event<LButtonUpEvent> / call(&CGridView::VScrlDrag_LButtonUp) = state<Normal>,

			state<HScrlDrag> +on_entry<_> / call(&CGridView::HScrlDrag_OnEntry),
			state<HScrlDrag> +on_exit<_> / call(&CGridView::HScrlDrag_OnExit),
			state<HScrlDrag> +event<LButtonUpEvent> / call(&CGridView::HScrlDrag_LButtonUp) = state<Normal>,

			state<RowDrag> +event<LButtonUpEvent> / call(&CGridView::RowDrag_LButtonUp) = state<Normal>,
			state<RowDrag> +event<MouseMoveEvent> / call(&CGridView::RowDrag_MouseMove),
			state<RowDrag> +event<MouseLeaveEvent> / call(&CGridView::RowDrag_MouseLeave) = state<Normal>,

			state<ColDrag> +event<LButtonUpEvent> / call(&CGridView::ColDrag_LButtonUp) = state<Normal>,
			state<ColDrag> +event<MouseMoveEvent> / call(&CGridView::ColDrag_MouseMove),
			state<ColDrag> +event<MouseLeaveEvent> / call(&CGridView::ColDrag_MouseLeave) = state<Normal>,

			state<ItemDrag> +event<LButtonUpEvent> / call(&CGridView::ItemDrag_LButtonUp) = state<Normal>,
			state<ItemDrag> +event<MouseMoveEvent> / call(&CGridView::ItemDrag_MouseMove),
			state<ItemDrag> +event<MouseLeaveEvent> / call(&CGridView::ItemDrag_MouseLeave) = state<Normal>,

			state<RowTrack> +event<LButtonUpEvent> / call(&CGridView::RowTrack_LButtonUp) = state<Normal>,
			state<RowTrack> +event<MouseMoveEvent> / call(&CGridView::RowTrack_MouseMove),
			state<RowTrack> +event<MouseLeaveEvent> / call(&CGridView::RowTrack_MouseLeave) = state<Normal>,

			state<ColTrack> +event<LButtonUpEvent> / call(&CGridView::ColTrack_LButtonUp) = state<Normal>,
			state<ColTrack> +event<MouseMoveEvent> / call(&CGridView::ColTrack_MouseMove),
			state<ColTrack> +event<MouseLeaveEvent> / call(&CGridView::ColTrack_MouseLeave) = state<Normal>,

			state<Edit> +on_exit<_> / call(&CGridView::Edit_OnExit),
			state<Edit> +event<LButtonDownEvent>[call(&CGridView::Edit_Guard_LButtonDown)] / call(&CGridView::Normal_LButtonDown) = state<Normal>,
			state<Edit> +event<LButtonDownEvent> / call(&CGridView::Edit_LButtonDown),
			state<Edit> +event<LButtonUpEvent> / call(&CGridView::Edit_LButtonUp),
			state<Edit> +event<KeyDownEvent>[call(&CGridView::Edit_Guard_KeyDown)] = state<Normal>,
			state<Edit> +event<KeyDownEvent> / call(&CGridView::Edit_KeyDown),
			state<Edit> +event<CharEvent> / call(&CGridView::Edit_Char));

	}
};
