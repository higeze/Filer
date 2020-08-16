#pragma once
#include <msctf.h>
#include "IBridgeTSFInterface.h"
#include "CellProperty.h"
#include "UIElement.h"
#include "observable.h"
#include "Timer.h"
#include "MyWnd.h"
#include "Scroll.h"
#include "TextboxStateMachine.h"
#include "UIElement.h"


class LayoutLineInfo;
struct D2DContext;
class CTextboxWnd;
class CTextStore;
class CTextEditSink;
class IBridgeTSFInterface;
class CTextboxWnd;
class CGridView;
class CTextCell;

namespace d2dw
{
	class CVScroll;
	class CHScroll;
}

struct COMPOSITIONRENDERINFO
{
	COMPOSITIONRENDERINFO(int start, int end, TF_DISPLAYATTRIBUTE displayAttribute)
		:Start(start), End(end), DisplayAttribute(displayAttribute){ }
	int Start;
	int End;
	TF_DISPLAYATTRIBUTE DisplayAttribute;
};

template<typename T>
bool in_range(const T& value, const T& min, const T& max)
{
	return min <= value && value <= max;
}

class D2DTextbox: public IBridgeTSFInterface, public CUIElement
{
/**********/
/* Static */
/**********/
public:
	static bool AppTSFInit();
	static void AppTSFExit();

/*********/
/* Field */
/*********/

protected:
/* Caret */
	enum caret
	{
		OldCaret = 0,
		CurCaret,	
		AncCaret,
		SelBegin,
		SelEnd,
	};
	observable_tuple<int, int, int, int, int> m_carets;
	d2dw::CPointF m_caretPoint;
/* Text */
	observable_wstring m_text;
/* Scroll */
	std::unique_ptr<d2dw::CVScroll> m_pVScroll;
	std::unique_ptr<d2dw::CHScroll> m_pHScroll;
/* Other */
	bool m_hasBorder = true;
	bool m_isScrollable = false;
	bool m_bCaret = false;
	bool m_isFirstDrawCaret = true;
	bool m_isClosing = false;
	std::unique_ptr<CTextboxStateMachine> m_pTextMachine;

public:
	D2DTextbox(
		CWnd* pWnd,
		CTextCell* pCell,
		const std::shared_ptr<TextboxProperty> pProp,
		const std::wstring& text,
		std::function<void(const std::wstring&)> changed,
		std::function<void(const std::wstring&)> final);
	virtual ~D2DTextbox();
private:
	void InitTSF();
	void UninitTSF();
public:
	// Getter
	int GetSelectionStart() { return std::get<caret::SelBegin>(m_carets); }
	int GetSelectionEnd() { return std::get<caret::SelEnd>(m_carets); }
	CTextCell* GetCellPtr() { return m_pCell; }

public:
	/******************/
	/* Windows Message*/
	/******************/
	virtual void OnPaint(const PaintEvent& e){ m_pTextMachine->process_event(e); }
	virtual void OnSetFocus(const SetFocusEvent& e) override { m_pTextMachine->process_event(e); }
	virtual void OnKillFocus(const KillFocusEvent& e) override { m_pTextMachine->process_event(e); }
	virtual void OnKeyDown(const KeyDownEvent& e) override { m_pTextMachine->process_event(e); }
	virtual void OnLButtonDown(const LButtonDownEvent& e) override { m_pTextMachine->process_event(e); }
	virtual void OnLButtonUp(const LButtonUpEvent& e) override { m_pTextMachine->process_event(e); }
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e) override { m_pTextMachine->process_event(e); }
	virtual void OnMouseMove(const MouseMoveEvent& e) override { m_pTextMachine->process_event(e); }
	virtual void OnSetCursor(const SetCursorEvent& e) override { m_pTextMachine->process_event(e); }
	virtual void OnContextMenu(const ContextMenuEvent& e) override { m_pTextMachine->process_event(e); }
	virtual void OnChar(const CharEvent& e) override { m_pTextMachine->process_event(e); }

	virtual void OnMouseWheel(const MouseWheelEvent& e);
	virtual void OnClose(const CloseEvent& e);
	virtual void OnRect(const RectEvent& e);

	virtual void Normal_Paint(const PaintEvent& e);
	virtual void Normal_LButtonDown(const LButtonDownEvent& e);
	virtual void Normal_LButtonUp(const LButtonUpEvent& e) { /*Do nothing*/ }
	virtual void Normal_LButtonClk(const LButtonClkEvent& e) { /*Do nothing*/ }
	virtual void Normal_LButtonSnglClk(const LButtonSnglClkEvent& e) { /*Do nothing*/ }
	virtual void Normal_LButtonDblClk(const LButtonDblClkEvent& e);
	virtual void Normal_RButtonDown(const RButtonDownEvent& e) { /*Do nothing*/ }
	virtual void Normal_MouseMove(const MouseMoveEvent& e);
	virtual void Normal_MouseLeave(const MouseLeaveEvent& e) { /*Do nothing*/ }
	virtual void Normal_SetCursor(const SetCursorEvent& e);
	virtual void Normal_ContextMenu(const ContextMenuEvent& e) { /*Do nothing*/ }
	virtual void Normal_KeyDown(const KeyDownEvent& e);
	virtual void Normal_Char(const CharEvent& e);
	virtual void Normal_SetFocus(const SetFocusEvent& e);
	virtual void Normal_KillFocus(const KillFocusEvent& e);

	virtual void VScrlDrag_OnEntry(const LButtonDownEvent& e);
	virtual void VScrlDrag_OnExit();
	virtual void VScrlDrag_MouseMove(const MouseMoveEvent& e);
	//virtual bool VScrl_Guard_SetCursor(const SetCursorEvent& e);
	//virtual void VScrl_SetCursor(const SetCursorEvent& e);

	virtual bool VScrlDrag_Guard_LButtonDown(const LButtonDownEvent& e);

	virtual void HScrlDrag_OnEntry(const LButtonDownEvent& e);
	virtual void HScrlDrag_OnExit();
	virtual void HScrlDrag_MouseMove(const MouseMoveEvent& e);
	virtual bool HScrlDrag_Guard_LButtonDown(const LButtonDownEvent& e);
	//virtual bool HScrl_Guard_SetCursor(const SetCursorEvent& e);
	//virtual void HScrl_SetCursor(const SetCursorEvent& e);

	virtual void Error_StdException(const std::exception& e);




	// IBridgeInterface
	virtual d2dw::CRectF GetClientRect() const;
	virtual d2dw::CRectF GetPageRect() const;

	// Text Functions 
	observable_wstring& GetText() { return m_text; }
	void Clear();
	//std::wstring FilterInputString(LPCWSTR s, UINT len);
private:
	bool CopySelectionToClipboard();
	bool PasteFromClipboard();

public:

	/* Caret */
	void MoveCaret(const int& position, const d2dw::CPointF& point);
	void MoveCaretWithShift(const int& position, const d2dw::CPointF& point);
	void MoveSelection(const int& selFirst, const int& selLast);
	BOOL InsertAtSelection(LPCWSTR psz);
	//BOOL DeleteAtSelection(BOOL fBack);
	//BOOL DeleteSelection();
	//void SetText(LPCWSTR str);
	void CancelEdit();
	void ClearText();
	void EnsureVisibleCaret();
	void Update();
	virtual void UpdateRects();
	virtual void UpdateScroll();


	// Render
	virtual bool GetIsVisible()const;
	void Render();
	void ResetCaret();
	void DrawCaret(const d2dw::CRectF& rc);
	FLOAT GetLineHeight();
	//	BOOL Layout();


	//void InvalidateRect();
	void ClearCompositionRenderInfo();
	BOOL AddCompositionRenderInfo(int Start, int End, TF_DISPLAYATTRIBUTE *pda);
public:
	std::function<std::vector<d2dw::CRectF>&()> GetOriginCharRects;
	std::function<std::vector<d2dw::CRectF>&()> GetOriginCursorCharRects;
	std::function<std::vector<d2dw::CRectF>& ()> GetActualCharRects;
	std::function<std::vector<d2dw::CRectF>& ()> GetActualSelectionCharRects;

	std::function<d2dw::CRectF&()> GetOriginContentRect;
	std::function<d2dw::CRectF& ()> GetActualContentRect;

	//std::optional<d2dw::CRectF> GetOriginCharRect(const int& pos);
	//std::optional<d2dw::CRectF> GetActualCharRect(const int& pos);
	std::optional<int> GetOriginCharPosFromPoint(const d2dw::CPointF& pt);

	std::optional<int> GetFirstCharPosInLine(const int& pos);
	std::optional<int> GetLastCharPosInLine(const int& pos);


	//int CharPosFromNearPoint(const d2dw::CPointF& pt);
	//BOOL RectFromCharPos(UINT nPos, d2dw::CRectF *prc);

	//UINT FineFirstEndCharPosInLine(UINT uCurPos, BOOL bFirst);

protected:
	CTimer m_timer;
	std::wstring m_strInit;
	CTextCell* m_pCell;
	std::shared_ptr<TextboxProperty> m_pProp;
	std::function<std::wstring()> m_getter;
	std::function<void(const std::wstring&)> m_setter;
	std::function<void(const std::wstring&)> m_changed;
	std::function<void(const std::wstring&)> m_final;


	//std::vector<LINEINFO> m_lineInfos;
	//UINT nLineCnt_;
	//FLOAT m_lineHeight = 0.f;
	//FLOAT row_width_;
	//int StarCharPos_;


	CComPtr<CTextEditSink> m_pTextEditSink;
	CComPtr<CTextStore> m_pTextStore;
	CComPtr<ITfDocumentMgr> m_pDocumentMgr;
	CComPtr<ITfContext> m_pInputContext;

	std::vector<COMPOSITIONRENDERINFO> m_compositionInfos;

	//COMPOSITIONRENDERINFO *pCompositionRenderInfo_;
	//UINT nCompositionRenderInfo_;
	TfEditCookie m_editCookie;

public:
	//////CGridView* m_pWnd;
	CWnd* m_pWnd;

public:
#if ( _WIN32_WINNT_WIN8 <= _WIN32_WINNT )
	static ITfThreadMgr2* s_pThreadMgr;
#else
	static ITfThreadMgr* s_pThreadMgr;
#endif
	static TfClientId s_tfClientId;
	static ITfKeystrokeMgr* s_pKeystrokeMgr;

};

class D2DTextbox2 :public D2DTextbox
{
public:
	D2DTextbox2(
		CWnd* pWnd, 
		const std::shared_ptr<TextboxProperty>& spProp, 
		const std::wstring& text,
		std::function<void(const std::wstring&)> changed,
		std::function<void(const std::wstring&)> final)
		:D2DTextbox(pWnd, nullptr, spProp, text, changed, final)
	{
		m_hasBorder = false;
		m_isScrollable = true;
	}

	virtual ~D2DTextbox2(){}

	d2dw::CRectF GetClientRect() const override
	{
		CRect rc = m_pWnd->GetClientRect();
		//rc.DeflateRect(1);
		return m_pWnd->GetDirectPtr()->Pixels2Dips(rc);
	}

	bool GetIsVisible() const override
	{
		return true;
	}

	virtual void Normal_ContextMenu(const ContextMenuEvent& e) override;
};