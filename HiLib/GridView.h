#pragma once
#include "MyWnd.h"
#include "Sheet.h"
#include "MyGdiPlusHelper.h"
#include "ThreadHelper.h"
#include <queue>
#include "DeadlineTimer.h"

class CDirect2DWrite;
class BackgroundProperty;
struct GridViewProperty;
struct CMouseStateMachine;
class CInplaceEdit;
namespace d2dw
{
	class CVScroll;
	class CHScroll;
}

class CGridView:public CWnd,public CSheet
{
public:
	std::unique_ptr<d2dw::CVScroll> m_pVScroll;
	std::unique_ptr<d2dw::CHScroll> m_pHScroll;

	static CMenu ContextMenu;
protected:
	CInplaceEdit* m_pEdit = nullptr;
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
	std::shared_ptr<d2dw::CDirect2DWrite>& GetDirect() { return m_pDirect; }
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
	void SetEditPtr(CInplaceEdit* pEdit) { m_pEdit = pEdit; }

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
	virtual LRESULT OnKeyDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);

	virtual LRESULT OnFilter(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnLButtonDblClkTimeExceed(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnDelayUpdate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	//virtual LRESULT OnCmdEnChange(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);

	virtual LRESULT OnCommandEditHeader(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);
	virtual LRESULT OnCommandDeleteColumn(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);
	virtual LRESULT OnCommandResizeSheetCell(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);
	virtual std::shared_ptr<CDC> GetClientDCPtr()const;

	virtual void OnContextMenu(const ContextMenuEvent& e);

	virtual void OnCellLButtonClk(CellEventArgs& e);
	virtual void OnCellContextMenu(CellContextMenuEventArgs& e);

	//virtual void OnLButtonDown(const LButtonDownEvent& e) override;
	//virtual void OnLButtonUp(const LButtonUpEvent& e) override;
	//virtual void OnMouseMove(const MouseMoveEvent& e) override;

	virtual void OnKeyDown(const KeyDownEvent& e) override;
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

	virtual CPoint GetScrollPos()const override;
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

	Status SaveGIFWithNewColorTable(Image *pImage,IStream* pIStream,const CLSID* clsidEncoder,DWORD nColors,BOOL fTransparent);

	virtual CColumn* GetParentColumnPtr(CCell* pCell)override;	
	
	HGLOBAL GetPaintMetaFileData();
	HENHMETAFILE GetPaintEnhMetaFileData();
	HENHMETAFILE GetAllEnhMetaFileData();

};