#pragma once
#include "D2DWWindow.h"
#include "StatusBar.h"
#include <msctf.h>
#include "IBridgeTSFInterface.h"
#include "Timer.h"
#include "MyWnd.h"
#include "Scroll.h"
#include "TextboxStateMachine.h"
#include "D2DWControl.h"
#include "ReactiveProperty.h"
#include "DisplayAttribute.h"
#include "property.h"
#include "encoding_type.h"

struct TextBoxProperty;
class CTextStore;
class CTextEditSink;
class IBridgeTSFInterface;
class CGridView;

class CVScroll;
class CHScroll;

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

class CD2DWWindow;






class CTextBox : public IBridgeTSFInterface, public CD2DWControl
{
	/**********/
	/* Static */
	/**********/
public:
	static void AppTSFInit();
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
	//ReactiveProperty
	ReactiveWStringProperty m_text;
	ReactiveProperty<CPointF> m_caretPoint;
	ReactiveTupleProperty<int, int, int, int, int> m_carets;

	/* Text */
	/* Scroll */
	std::unique_ptr<CVScroll> m_pVScroll;
	std::unique_ptr<CHScroll> m_pHScroll;
	/* Other */
	bool m_hasBorder = true;
	bool m_isScrollable = false;
	bool m_bCaret = false;
	bool m_isFirstDrawCaret = false;
	bool m_isClosing = false;
	std::unique_ptr<CTextBoxStateMachine> m_pTextMachine;

public:
	CTextBox(
		CD2DWControl* pParentControl,
		const std::shared_ptr<TextBoxProperty> pProp,
		const std::wstring& text);
	virtual ~CTextBox();
private:
	void InitTSF();
	void UninitTSF();
public:
	// Getter
	void SetHasBorder(bool value) { m_hasBorder = value; }
	void SetIsScrollable(bool value){ m_isScrollable = value; }
	ReactiveWStringProperty& GetText() { return m_text; }
	ReactiveProperty<CPointF>& GetCaretPos() { return m_caretPoint; }
	ReactiveTupleProperty<int, int, int, int, int>& GetCarets() { return m_carets; }

	int GetSelectionStart() { return std::get<caret::SelBegin>(m_carets.get()); }
	int GetSelectionEnd() { return std::get<caret::SelEnd>(m_carets.get()); }

public:
	/******************/
	/* Windows Message*/
	/******************/
	virtual void OnCreate(const CreateEvt& e);
	virtual void OnDestroy(const DestroyEvent& e);
	virtual void OnPaint(const PaintEvent& e) { m_pTextMachine->process_event(e); }
	virtual void OnSetFocus(const SetFocusEvent& e) override { m_pTextMachine->process_event(e); }
	virtual void OnKillFocus(const KillFocusEvent& e) override { m_pTextMachine->process_event(e); }
	virtual void OnKeyDown(const KeyDownEvent& e) override { m_pTextMachine->process_event(e); }
	virtual void OnKeyUp(const KeyUpEvent& e) override { m_pTextMachine->process_event(e); }
	virtual void OnLButtonDown(const LButtonDownEvent& e) override { m_pTextMachine->process_event(e); }
	virtual void OnLButtonUp(const LButtonUpEvent& e) override { m_pTextMachine->process_event(e); }
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e) override { m_pTextMachine->process_event(e); }
	virtual void OnMouseMove(const MouseMoveEvent& e) override { m_pTextMachine->process_event(e); }
	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) override 
	{
		e.WndPtr->SetCapturedControlPtr(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
		m_pTextMachine->process_event(e);
	}
	virtual void OnLButtonEndDrag(const LButtonEndDragEvent& e) override
	{
		e.WndPtr->ReleaseCapturedControlPtr();
		m_pTextMachine->process_event(e);
	}
	virtual void OnSetCursor(const SetCursorEvent& e) override { m_pTextMachine->process_event(e); }
	virtual void OnContextMenu(const ContextMenuEvent& e) override { m_pTextMachine->process_event(e); }
	virtual void OnChar(const CharEvent& e) override { m_pTextMachine->process_event(e); }

	virtual void OnWndKillFocus(const KillFocusEvent& e) override;

	virtual void OnMouseWheel(const MouseWheelEvent& e);
	virtual void OnClose(const CloseEvent& e);
	virtual void OnRect(const RectEvent& e);

	/**************/
	/* SM Message */
	/**************/
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
	virtual void Normal_ContextMenu(const ContextMenuEvent& e);

	virtual void Normal_KeyDown(const KeyDownEvent& e);
	virtual void Normal_KeyUp(const KeyUpEvent& e);
	virtual void Normal_Char(const CharEvent& e);
	virtual void Normal_SetFocus(const SetFocusEvent& e);
	virtual void Normal_KillFocus(const KillFocusEvent& e);

	virtual void TextDrag_OnEntry(const LButtonBeginDragEvent& e);
	virtual void TextDrag_OnExit(const LButtonEndDragEvent& e);
	virtual void TextDrag_MouseMove(const MouseMoveEvent& e);

	virtual void VScrlDrag_OnEntry(const LButtonBeginDragEvent& e);
	virtual void VScrlDrag_OnExit(const LButtonEndDragEvent& e);
	virtual void VScrlDrag_MouseMove(const MouseMoveEvent& e);
	virtual bool VScrlDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e);
	//virtual bool VScrl_Guard_SetCursor(const SetCursorEvent& e);
	//virtual void VScrl_SetCursor(const SetCursorEvent& e);


	virtual void HScrlDrag_OnEntry(const LButtonBeginDragEvent& e);
	virtual void HScrlDrag_OnExit(const LButtonEndDragEvent& e);
	virtual void HScrlDrag_MouseMove(const MouseMoveEvent& e);
	virtual bool HScrlDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e);
	//virtual bool HScrl_Guard_SetCursor(const SetCursorEvent& e);
	//virtual void HScrl_SetCursor(const SetCursorEvent& e);

	virtual void Error_StdException(const std::exception& e);




	// IBridgeInterface
	virtual CRectF GetRectInWnd() const;
	virtual CRectF GetPageRect() const;

	// Text Functions 
	void Clear();
	//std::wstring FilterInputString(LPCWSTR s, UINT len);
protected:
	bool CopySelectionToClipboard();
	bool PasteFromClipboard();

public:

	/* Caret */
	void MoveCaret(const int& position, const CPointF& point);
	void MoveCaretWithShift(const int& position, const CPointF& point);
	void MoveSelection(const int& selFirst, const int& selLast);
	BOOL InsertAtSelection(LPCWSTR psz);
	//BOOL DeleteAtSelection(BOOL fBack);
	//BOOL DeleteSelection();
	//void SetText(LPCWSTR str);
	void CancelEdit();
	void ClearText();
	void EnsureVisibleCaret();
	void UpdateAll();
	virtual void ResetOriginRects();
	virtual void ResetActualRects();
	virtual void UpdateScroll();


	// Render
	virtual bool GetIsVisible()const;
	void Render();
	void ResetCaret();
	void DrawCaret(const CRectF& rc);
	FLOAT GetLineHeight();
	//	BOOL Layout();

	void TerminateCompositionString();
	//void InvalidateRect();
	void ClearCompositionRenderInfo();
	BOOL AddCompositionRenderInfo(int Start, int End, TF_DISPLAYATTRIBUTE* pda);
public:

	virtual const CComPtr<IDWriteTextLayout1>& GetTextLayoutPtr();


	property<std::vector<CRectF>> OriginCharRects;
	property<std::vector<CRectF>> OriginCursorCharRects;
	property<std::vector<CRectF>> OriginCaptureCharRects;

	property<std::vector<CRectF>> ActualCharRects;
	property<std::vector<CRectF>> ActualSelectionCharRects;
	property<std::vector<CRectF>> ActualCursorCharRects;
	property<std::vector<CRectF>> ActualCaptureCharRects;

	property<CRectF> OriginContentRect;
	property<CRectF> ActualContentRect;

	//std::optional<CRectF> GetOriginCharRect(const int& pos);
	//std::optional<CRectF> GetActualCharRect(const int& pos);
	std::optional<int> GetOriginCharPosFromPoint(const CPointF& pt);
	std::optional<int> GetActualCharPosFromPoint(const CPointF& pt);
	std::optional<int> GetActualCaptureCharPosFromPoint(const CPointF& pt);


	std::optional<int> GetFirstCharPosInLine(const int& pos);
	std::optional<int> GetLastCharPosInLine(const int& pos);


	//int CharPosFromNearPoint(const CPointF& pt);
	//BOOL RectFromCharPos(UINT nPos, CRectF *prc);

	//UINT FineFirstEndCharPosInLine(UINT uCurPos, BOOL bFirst);

protected:
	CTimer m_timer;
	std::wstring m_initText;
	std::shared_ptr<TextBoxProperty> m_pProp;
	std::function<std::wstring()> m_getter;
	std::function<void(const std::wstring&)> m_setter;

	CComPtr<IDWriteTextLayout1> m_pTextLayout;

	std::function<CComPtr<CTextEditSink>&()> GetTextEditSink;
	std::function<CComPtr<CTextStore>&()> GetTextStore;

	std::function<CComPtr<ITfDocumentMgr>&()> GetDocumentMgr;
	std::function<CComPtr<ITfContext>&()> GetContext;

	std::vector<COMPOSITIONRENDERINFO> m_compositionInfos;

	//COMPOSITIONRENDERINFO *pCompositionRenderInfo_;
	//UINT nCompositionRenderInfo_;
	TfEditCookie m_editCookie;

public:
	CDispAttrProps* GetDispAttrProps();
	HRESULT GetDisplayAttributeTrackPropertyRange(TfEditCookie ec, ITfContext* pic, ITfRange* pRange, ITfReadOnlyProperty** ppProp, CDispAttrProps* pDispAttrProps);
	HRESULT GetDisplayAttributeData(TfEditCookie ec, ITfReadOnlyProperty* pProp, ITfRange* pRange, TF_DISPLAYATTRIBUTE* pda, TfGuidAtom* pguid);


	/***********/
	/* Statics */
	/***********/
public:
#if ( _WIN32_WINNT_WIN8 <= _WIN32_WINNT )
	static std::function<CComPtr<ITfThreadMgr2>&()> GetThreadMgr;
#else
	static std::function<CComPtr<ITfThreadMgr>&()> GetThreadMgr;
#endif
	static TfClientId s_tfClientId;
	static std::function<CComPtr<ITfKeystrokeMgr>&()> GetKeystrokeMgr;

	static std::function<CComPtr<ITfDisplayAttributeMgr>& ()> GetDisplayAttributeMgr;

	static std::function<CComPtr<ITfCategoryMgr>& ()> GetCategoryMgr;

};
