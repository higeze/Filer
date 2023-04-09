#pragma once
#include "D2DWWindow.h"
#include "D2DWControl.h"
#include "Scroll.h"
#include "ScrollProperty.h"
#include "ReactiveProperty.h"

#include "PDFViewProperty.h"
#include <msctf.h>

#include <cstdlib>
#include <utility>
 
#include <windows.h>
#include <shcore.h>
#include <d3d11_1.h>
#include <dxgi1_2.h>
#include <d2d1_2.h>
#include <wincodec.h>

#include "PdfViewStateMachine.h"
#include "FileIsInUse.h"

#include "getter_macro.h"

#include <future>
#include <mutex>

#include "PDFViewport.h"
#include "PDFCaret.h"
//#include "D2DPDFBitmapDrawer.h"

class CPDFDoc;
class CD2DPDFBitmapDrawer;
class CVScroll;
class CHScroll;


enum class InitialScaleMode
{
	None,
	MinWidthHeight,
	Width,
	Height
};

enum class PDFMode
{
	Pan,
	Text
};


class CPdfView : public CD2DWControl
{
public:
	/* Scroll */
	SHAREDPTR_GETTER(CVScroll, VScroll)
	SHAREDPTR_GETTER(CHScroll, HScroll)
	//LAZY_GETTER_NO_CLEAR_IMPL(std::vector<CRectF>, OriginHighliteRects)

	/**********/
	/* Static */
	/**********/
public:

	/*********/
	/* Field */
	/*********/
protected:
	//CPDFViewport m_viewport;
	//PdfBmpKey m_prevKey;
	CPDFCaret m_caret;

	CRectF m_rect;
	std::shared_ptr<PdfViewProperty> m_pProp;
	
	//ReactiveProperty
	ReactiveWStringProperty m_path;
	ReactiveCommand<void> m_open;
	ReactiveProperty<FLOAT> m_scale;
	ReactiveWStringProperty m_find;
	ReactiveProperty<D2D1_BITMAPSOURCE_ORIENTATION> m_rotate;
	FLOAT m_prevScale;
	int m_curPageNo;

	FLOAT m_minScale = 0.1f;
	FLOAT m_maxScale = 8.f;



	std::unique_ptr<CPDFDoc> m_pdf;
	std::unique_ptr<CD2DPDFBitmapDrawer> m_pdfDrawer;
	CComPtr<IFileIsInUse> m_pFileIsInUse;

    //std::vector<std::unique_ptr<CPdfPage>> m_pdfPages;
   // std::vector<CComPtr<ID2D1Bitmap1>> m_pdfBmps;

	std::unique_ptr<CPdfViewStateMachine> m_pMachine;

	InitialScaleMode m_initialScaleMode;
	PDFMode m_mode = PDFMode::Text;


public:
	CPdfView(CD2DWControl* pParentControl, const std::shared_ptr<PdfViewProperty>& pProp);
	virtual ~CPdfView();
public:
	//
	std::unique_ptr<CPDFDoc>& GetDocPtr() { return m_pdf; }
	// Getter
	ReactiveWStringProperty& GetPath() { return m_path; }
	ReactiveProperty<FLOAT>& GetScale() { return m_scale; }
	ReactiveWStringProperty& GetFind() { return m_find; }
	ReactiveCommand<void>& GetOpenCommand() { return m_open; }

	FLOAT GetMinScale() const { return m_minScale; }
	FLOAT GetMaxScale() const { return m_maxScale; }
	
	//std::function<CComPtr<IPdfRendererNative>& ()> GetPdfRenderer;
	CRectF GetRenderRectInWnd();
	CSizeF GetRenderSize();
	CSizeF GetRenderContentSize();

	std::tuple<CRectF, CRectF> GetRects() const;

public:
	/************/
	/* Viewport */
	/************/	

	CRectF Wnd2Ctrl(const CRectF& rcInWnd);
	CPointF Wnd2Ctrl(const CPointF& ptInWnd);
	CRectF Ctrl2Wnd(const CRectF& rcInCtrl);
	CPointF Ctrl2Wnd(const CPointF& ptInCtrl);

	CRectF Ctrl2Doc(const CRectF& rcInCtrl);
	CPointF Ctrl2Doc(const CPointF& ptInCtrl);
	CRectF Doc2Ctrl(const CRectF& rcInDoc);
	CPointF Doc2Ctrl(const CPointF& ptInDoc);

	CRectF Doc2Page(const int& i, const CRectF& rcInDoc);
	CPointF Doc2Page(const int& i, const CPointF& ptInDoc);
	CRectF Page2Doc(const int& i, const CRectF& rcInPage);
	CPointF Page2Doc(const int& i, const CPointF& ptInPage);
	std::tuple<int, CPointF> Doc2Page(const CPointF& ptInDoc);

	CRectF Page2PdfiumPage(const int& page, const CRectF& rcInPage);
	CPointF Page2PdfiumPage(const int& page, const CPointF& ptInPage);
	CRectF PdfiumPage2Page(const int& page, const CRectF& rcInPdfiumPage);
	CPointF PdfiumPage2Page(const int& page, const CPointF& ptInPdfiumPage);

	std::tuple<int, CPointF> Wnd2PdfiumPage(const CPointF& ptInWnd);
	CRectF PdfiumPage2Wnd(const int& i, const CRectF rcInPdfiumPage);
	CPointF PdfiumPage2Wnd(const int& i, const CPointF ptInPdfiumPage);

public:
	/******************/
	/* Windows Message*/
	/******************/
	virtual void OnCreate(const CreateEvt& e);
	virtual void OnPaint(const PaintEvent& e) { m_pMachine->process_event(e); }
	virtual void OnSetFocus(const SetFocusEvent& e) override { m_pMachine->process_event(e); }
	virtual void OnKillFocus(const KillFocusEvent& e) override { m_pMachine->process_event(e); }
	virtual void OnKeyDown(const KeyDownEvent& e) override { m_pMachine->process_event(e); }
	virtual void OnLButtonDown(const LButtonDownEvent& e) override { m_pMachine->process_event(e); }
	virtual void OnLButtonUp(const LButtonUpEvent& e) override { m_pMachine->process_event(e); }
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e) override { m_pMachine->process_event(e); }
	virtual void OnMouseMove(const MouseMoveEvent& e) override { m_pMachine->process_event(e); }
	virtual void OnLButtonBeginDrag(const LButtonBeginDragEvent& e) override 
	{
		e.WndPtr->SetCapturedControlPtr(std::dynamic_pointer_cast<CD2DWControl>(shared_from_this()));
		m_pMachine->process_event(e); 
	}
	virtual void OnLButtonEndDrag(const LButtonEndDragEvent& e) override 
	{
		e.WndPtr->ReleaseCapturedControlPtr();
		m_pMachine->process_event(e);
	}
	virtual void OnSetCursor(const SetCursorEvent& e) override { m_pMachine->process_event(e); }
	virtual void OnContextMenu(const ContextMenuEvent& e) override { m_pMachine->process_event(e); }
	virtual void OnChar(const CharEvent& e) override { m_pMachine->process_event(e); }

	virtual void OnMouseWheel(const MouseWheelEvent& e) override;
	virtual void OnClose(const CloseEvent& e) override;
	virtual void OnDestroy(const DestroyEvent& e) override;
	virtual void OnEnable(const EnableEvent& e) override;

	virtual void OnRect(const RectEvent& e) override;
	virtual void OnWndKillFocus(const KillFocusEvent& e) override;

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
	virtual void Normal_MouseMove(const MouseMoveEvent& e) { /*Do nothing*/ }
	virtual void Normal_MouseLeave(const MouseLeaveEvent& e) { /*Do nothing*/ }
	virtual void Normal_ContextMenu(const ContextMenuEvent& e);
	virtual void Normal_KeyDown(const KeyDownEvent& e);
	virtual void Normal_Char(const CharEvent& e) { /*Do nothing*/ }
	virtual void Normal_SetFocus(const SetFocusEvent& e) { /*Do nothing*/ }
	virtual void Normal_KillFocus(const KillFocusEvent& e);

	virtual void NormalPan_LButtonDown(const LButtonDownEvent& e) {}
	virtual void NormalPan_SetCursor(const SetCursorEvent& e);

	virtual void NormalText_LButtonDown(const LButtonDownEvent& e);
	virtual void NormalText_SetCursor(const SetCursorEvent& e);

	virtual void VScrlDrag_OnEntry(const LButtonBeginDragEvent& e);
	virtual void VScrlDrag_OnExit(const LButtonEndDragEvent& e);
	virtual void VScrlDrag_MouseMove(const MouseMoveEvent& e);
	virtual void VScrlDrag_SetCursor(const SetCursorEvent& e);
	virtual bool VScrl_Guard_SetCursor(const SetCursorEvent& e);
	virtual bool VScrlDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e);

	virtual void HScrlDrag_OnEntry(const LButtonBeginDragEvent& e);
	virtual void HScrlDrag_OnExit(const LButtonEndDragEvent& e);
	virtual void HScrlDrag_MouseMove(const MouseMoveEvent& e);
	virtual void HScrlDrag_SetCursor(const SetCursorEvent& e);
	virtual bool HScrl_Guard_SetCursor(const SetCursorEvent& e);
	virtual bool HScrlDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e);

	virtual void Panning_OnEntry(const LButtonBeginDragEvent& e);
	virtual void Panning_OnExit(const LButtonEndDragEvent& e);
	virtual void Panning_MouseMove(const MouseMoveEvent& e);
	virtual void Panning_SetCursor(const SetCursorEvent& e);
	virtual bool Panning_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e);

	virtual void TextDrag_OnEntry(const LButtonBeginDragEvent& e);
	virtual void TextDrag_OnExit(const LButtonEndDragEvent& e);
	virtual void TextDrag_MouseMove(const MouseMoveEvent& e);
	virtual void TextDrag_SetCursor(const SetCursorEvent& e);
	virtual bool TextDrag_Guard_LButtonBeginDrag(const LButtonBeginDragEvent& e);

	virtual void Error_StdException(const std::exception& e);

	void Open();
	void Open(const std::wstring& path);
	void Close();

	void UpdateScroll();

	void Update();

};
