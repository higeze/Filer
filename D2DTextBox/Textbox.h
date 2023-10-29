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
#include "property.h"
#include "encoding_type.h"
#include "TextEditSink.h"
#include "getter_macro.h"

#include "UndoReDoManager.h"

#include "reactive_property.h"
#include "reactive_string.h"

#include "TextCaret.h"


struct TextBoxProperty;
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

template<typename TRect>
bool in_range(const TRect& value, const TRect& min, const TRect& max)
{
	return min <= value && value <= max;
}

class CD2DWWindow;


class CTextBox : public IBridgeTSFInterface, public CD2DWControl
{
private:
	std::optional<CSizeF> m_opt_size;
public:
	void Measure(const CSizeF& availableSize, const std::wstring& text);
	const CSizeF& DesiredSize() const;
	void Arrange(const CRectF& rc) { m_rect = rc; UpdateAll(); }
	/************/
	/* Reactive */
	/************/
public:
	reactive_wstring_ptr Text;
	reactive_wstring_ptr EnterText;
	reactive_property_ptr<CTextCaret> Caret;
protected:
	// Caret
	enum caret
	{
		OldCaret = 0,
		CurCaret,
		AncCaret,
		SelBegin,
		SelEnd,
	};
	/*********/
	/* Field */
	/*********/
	/* Scroll */
	std::unique_ptr<CVScroll> m_pVScroll;
	std::unique_ptr<CHScroll> m_pHScroll;
	/* Other */
	bool m_acceptsTab = true;
	bool m_isEnterText = false;
	bool m_hasBorder = true;
	bool m_isScrollable = false;
	bool m_bCaret = false;
	bool m_isFirstDrawCaret = false;
	bool m_isClosing = false;
	std::unique_ptr<CTextBoxStateMachine> m_pTextMachine;

	CUnDoReDoManager m_doMgr;

	/* TSF */
	mutable TfEditCookie m_editCookie;
	DECLARE_LAZY_COMPTR_GETTER(ITfDocumentMgr, DocumentMgr)
	DECLARE_LAZY_COMPTR_GETTER(ITfContext, Context)
	DECLARE_LAZY_COMPTR_GETTER(CTextStore, TextStore)
	DECLARE_LAZY_COMPTR_GETTER(CTextEditSink, TextEditSink)

	/***************/
	/* Constructor */
	/* Destructor  */
	/***************/
public:

	CTextBox(
		CD2DWControl* pParentControl,
		std::unique_ptr<CVScroll>&& pVScroll,
		std::unique_ptr<CHScroll>&& pHScroll,
		const std::shared_ptr<TextBoxProperty> pProp,
		const std::wstring& text);

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
	void SetIsEnterText(bool value){ m_isEnterText = value; }

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
	// Size
	CSizeF MeasureSize(const std::wstring& text);
	// Caret
	void MoveCaret(const int& position, const CPointF& point);
	void MoveCaretWithShift(const int& position, const CPointF& point);
	void MoveSelection(const int& selFirst, const int& selLast);

	void Insert(const std::wstring& text);
	void ReplaceSelection(const std::wstring& text);
	void Replace(const std::size_t& index, const std::size_t& count, const std::wstring& text);
	void DeleteOne();
	void BackspaceOne();
	void DeleteSelection();

	//void SetText(LPCWSTR str);
	void CancelEdit();
	void ClearText();
	void EnsureVisibleCaret();
	virtual void UpdateAll();
	virtual void ClearOriginRects();
	virtual void ClearActualRects();
	virtual void UpdateScroll();


	// Render
	virtual bool GetIsVisible()const;
	// Paint
	virtual void PaintText(const PaintEvent& e);
	virtual void PaintSelection(const PaintEvent& e);
	virtual void PaintHighlite(const PaintEvent& e) {}
	virtual void PaintCaret(const PaintEvent& e);
	virtual void PaintCompositionLine(const PaintEvent& e);
	// Caret Blink
	virtual void StartCaretBlink();
	virtual void StopCaretBlink();

	FLOAT GetLineHeight();
	//	BOOL Layout();

	void TerminateCompositionString();
	//void InvalidateRect();
	void ClearCompositionRenderInfo();
	BOOL AddCompositionRenderInfo(int Start, int End, TF_DISPLAYATTRIBUTE* pda);

	//Getter
	LAZY_CCOMPTR_GETTER(IDWriteTextLayout1, TextLayout)

	LAZY_GETTER(std::vector<CRectF>, OriginCharRects)
	LAZY_GETTER(std::vector<CRectF>, OriginCursorCharRects)
	LAZY_GETTER(std::vector<CRectF>, OriginCaptureCharRects)
	LAZY_GETTER(std::vector<CRectF>, ActualCharRects)
	LAZY_GETTER(std::vector<CRectF>, ActualSelectionCharRects)
	LAZY_GETTER(std::vector<CRectF>, ActualCursorCharRects)
	LAZY_GETTER(std::vector<CRectF>, ActualCaptureCharRects)
	LAZY_GETTER(CRectF, OriginContentRect)
	LAZY_GETTER(CRectF, ActualContentRect)

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



	std::vector<COMPOSITIONRENDERINFO> m_compositionInfos;

	//COMPOSITIONRENDERINFO *pCompositionRenderInfo_;
	//UINT nCompositionRenderInfo_;

public:
	std::shared_ptr<TextBoxProperty> GetTextBoxPropertyPtr() { return m_pProp; }




};
