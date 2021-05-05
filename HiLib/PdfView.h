#pragma once
#include "D2DWWindow.h"
#include <msctf.h>
#include "Scroll.h"
#include "ScrollProperty.h"
#include "D2DWControl.h"
#include "ReactiveProperty.h"

#include <cstdlib>
#include <utility>
 
#include <windows.h>
#include <shcore.h>
#include <d3d11_1.h>
#include <dxgi1_2.h>
#include <d2d1_2.h>
#include <wincodec.h>

#include <windows.storage.h>
#include <windows.storage.streams.h>
#include <windows.data.pdf.h>
#include <windows.data.pdf.interop.h>


#include <wrl/client.h>
#include <wrl/event.h>
   
#pragma comment(lib, "shcore.lib")
#pragma comment(lib, "runtimeobject.lib")
#pragma comment(lib, "windows.data.pdf.lib")
#pragma comment(lib, "windowscodecs.lib")

#include "PdfViewStateMachine.h"


//TODOHIGH
namespace abipdf = ABI::Windows::Data::Pdf;
namespace abifoundation = ABI::Windows::Foundation;
namespace abistorage = ABI::Windows::Storage;
namespace abistreams = ABI::Windows::Storage::Streams;
namespace winfoundation = Windows::Foundation;
namespace wrl = Microsoft::WRL;
namespace wrlwrappers = Microsoft::WRL::Wrappers;


#include <future>
#include <mutex>

class CPdf;
class CPdfPage;
class CVScroll;
class CHScroll;

struct PdfViewProperty
{
public:
	std::shared_ptr<FormatF> Format;
	std::shared_ptr<SolidLine> FocusedLine;
	std::shared_ptr<SolidFill> NormalFill;
	std::shared_ptr<CRectF> Padding;
	std::shared_ptr<ScrollProperty> VScrollPropPtr;
	std::shared_ptr<ScrollProperty> HScrollPropPtr;

public:
	PdfViewProperty()
		:Format(std::make_shared<FormatF>(L"Meiryo UI", CDirect2DWrite::Points2Dips(9),  0.0f, 0.0f, 0.0f, 1.0f)),
		FocusedLine(std::make_shared<SolidLine>(22.f/255.f, 160.f/255.f, 133.f/255.f, 1.0f, 1.0f)),
		NormalFill(std::make_shared<SolidFill>(246.f/255.f, 246.f/255.f, 246.f/255.f, 1.0f)),
		Padding(std::make_shared<CRectF>(2.0f,2.0f,2.0f,2.0f)),
		VScrollPropPtr(std::make_shared<ScrollProperty>()),
		HScrollPropPtr(std::make_shared<ScrollProperty>()){};

	virtual ~PdfViewProperty() = default;

	template <class Archive>
    void serialize(Archive& ar)
    {
		ar("FocusedLine",FocusedLine);
		ar("NormalFill",NormalFill);
		ar("Padding",Padding);
		ar("VScrollProperty", VScrollPropPtr);
		ar("HScrollProperty", HScrollPropPtr);
	}

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(PdfViewProperty,
		FocusedLine,
		NormalFill,
		Padding,
		VScrollPropPtr,
		HScrollPropPtr)
};

enum class InitialScaleMode
{
	None,
	MinWidthHeight,
	Width,
	Height
};


class CPdfView : public CD2DWControl
{
	/**********/
	/* Static */
	/**********/
public:

	/*********/
	/* Field */
	/*********/
protected:
	std::shared_ptr<PdfViewProperty> m_pProp;
	
	//ReactiveProperty
	ReactiveWStringProperty m_path;
	ReactiveCommand<void> m_open;
	ReactiveProperty<FLOAT> m_scale;
	ReactiveProperty<D2D1_BITMAPSOURCE_ORIENTATION> m_rotate;
	FLOAT m_prevScale;

	/* Scroll */
	std::shared_ptr<CVScroll> m_pVScroll;
	std::shared_ptr<CHScroll> m_pHScroll;

	std::unique_ptr<CPdf> m_pdf;
    //std::vector<std::unique_ptr<CPdfPage>> m_pdfPages;
   // std::vector<CComPtr<ID2D1Bitmap1>> m_pdfBmps;

	std::unique_ptr<CPdfViewStateMachine> m_pMachine;

	InitialScaleMode m_initialScaleMode;



public:
	CPdfView(CD2DWControl* pParentControl, const std::shared_ptr<PdfViewProperty>& pProp);
	virtual ~CPdfView();
public:
	// Getter
	ReactiveWStringProperty& GetPath() { return m_path; }
	ReactiveProperty<FLOAT>& GetScale() { return m_scale; }
	ReactiveCommand<void>& GetOpenCommand() { return m_open; }
	
	std::function<CComPtr<IPdfRendererNative>& ()> GetPdfRenderer;
	CRectF GetRenderRectInWnd();
	CSizeF GetRenderSize();
	CSizeF GetRenderContentSize();

	std::tuple<CRectF, CRectF> GetRects() const;

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

	virtual void OnMouseWheel(const MouseWheelEvent& e);
	virtual void OnClose(const CloseEvent& e);
	virtual void OnRect(const RectEvent& e);

	/**************/
	/* SM Message */
	/**************/
	virtual void Normal_Paint(const PaintEvent& e);
	virtual void Normal_LButtonDown(const LButtonDownEvent& e){ /*Do nothing*/ }
	virtual void Normal_LButtonUp(const LButtonUpEvent& e) { /*Do nothing*/ }
	virtual void Normal_LButtonClk(const LButtonClkEvent& e) { /*Do nothing*/ }
	virtual void Normal_LButtonSnglClk(const LButtonSnglClkEvent& e) { /*Do nothing*/ }
	virtual void Normal_LButtonDblClk(const LButtonDblClkEvent& e) { /*Do nothing*/ }
	virtual void Normal_RButtonDown(const RButtonDownEvent& e) { /*Do nothing*/ }
	virtual void Normal_MouseMove(const MouseMoveEvent& e) { /*Do nothing*/ }
	virtual void Normal_MouseLeave(const MouseLeaveEvent& e) { /*Do nothing*/ }
	virtual void Normal_SetCursor(const SetCursorEvent& e);
	virtual void Normal_ContextMenu(const ContextMenuEvent& e);
	virtual void Normal_KeyDown(const KeyDownEvent& e);
	virtual void Normal_Char(const CharEvent& e) { /*Do nothing*/ }
	virtual void Normal_SetFocus(const SetFocusEvent& e) { /*Do nothing*/ }
	virtual void Normal_KillFocus(const KillFocusEvent& e) { /*Do nothing*/ }

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

	virtual void Error_StdException(const std::exception& e);

	void Open();
	void Open(const std::wstring& path);
	void UpdateScroll();

	void Update();

};
