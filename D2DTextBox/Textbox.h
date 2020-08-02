#pragma once
#include <msctf.h>
#include "IBridgeTSFInterface.h"
#include "CellProperty.h"
#include "UIElement.h"
#include "observable.h"
#include "Timer.h"
#include "MyWnd.h"

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
	int nStart;
	int nEnd;
	TF_DISPLAYATTRIBUTE da;
};

class D2DTextbox: public IBridgeTSFInterface
{
public:
	static bool AppTSFInit();
	static void AppTSFExit();
protected:
	enum caret
	{
		OldCaret = 0,
		CurCaret,	
		AncCaret,
		SelBegin,
		SelEnd,
	};
	observable_tuple<int, int, int, int, int> m_carets;
	observable_wstring m_text;
	bool m_hasBorder = true;
	bool m_isWrap = true;
	bool m_isScrollable = false;

//	bool m_recalc = true;

//	LARGE_INTEGER m_frequency;
	bool m_bCaret = false;
//	LARGE_INTEGER m_gtm, m_pregtm;

	bool m_isFirstDrawCaret = true;
	bool m_isClosing = false;
	std::unique_ptr<d2dw::CVScroll> m_pVScroll;
	std::unique_ptr<d2dw::CHScroll> m_pHScroll;
	//FLOAT m_vertialScroll;

public:
	D2DTextbox(
		CWnd* pWnd,
		//////CGridView* pWnd, 
		CTextCell* pCell,
		std::shared_ptr<CellProperty> pProp,
		std::function<std::wstring()> getter,
		std::function<void(const std::wstring&)> setter,
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
//	void SetVertialScroll(FLOAT scroll) { m_vertialScroll = scroll; }
	//float GetLineWidth() { return row_width_; }
	//UINT GetLineCount() { return nLineCnt_; }

public:
	virtual void OnClose(const CloseEvent& e);
	virtual void OnPaint(const PaintEvent& e);
	virtual void OnSetFocus(const SetFocusEvent& e);
	virtual void OnKillFocus(const KillFocusEvent& e);
	virtual void OnKeyDown(const KeyDownEvent& e);
	virtual void OnLButtonDown(const LButtonDownEvent& e);
	virtual void OnMouseMove(const MouseMoveEvent& e);
	virtual void OnMouseWheel(const MouseWheelEvent& e);
	virtual void OnContextMenu(const ContextMenuEvent& e){}

	virtual void OnChar(const CharEvent& e);
	virtual void OnRect(const RectEvent& e);

	virtual void UpdateScroll();

	// IBridgeInterface
	virtual d2dw::CRectF GetClientRect() const;
	virtual d2dw::CRectF GetPageRect() const;
	virtual d2dw::CRectF GetContentRect() const;

	// Text Functions 
	observable_wstring& GetText() { return m_text; }
	void Clear();
	//std::wstring FilterInputString(LPCWSTR s, UINT len);
private:
	bool CopySelectionToClipboard();
	bool PasteFromClipboard();

public:

	// Selection
	void MoveCaret(const int& newPos);
	void MoveCaretWithShift(const int& newPos);
	void MoveSelection(const int& selFirst, const int& selLast);
	BOOL InsertAtSelection(LPCWSTR psz);
	//BOOL DeleteAtSelection(BOOL fBack);
	//BOOL DeleteSelection();
	//void SetText(LPCWSTR str);
	void CancelEdit();
	void ClearText();
	void EnsureVisibleCaret();


	// Render
	virtual bool GetIsVisible()const;
	void Render();
	void ResetCaret();
	void DrawCaret(const d2dw::CRectF& rc);
	FLOAT GetLineHeight();
	//	BOOL Layout();


	//void InvalidateRect();
	void ClearCompositionRenderInfo();
	BOOL AddCompositionRenderInfo(int nStart, int nEnd, TF_DISPLAYATTRIBUTE *pda);
public:
	std::vector<d2dw::CRectF> GetOriginCharRects() const;
	std::vector<d2dw::CRectF> GetScrolledCharRects() const;
	std::vector<d2dw::CRectF> GetScrolledCursorCharRects() const;

	std::optional<d2dw::CRectF> GetScrolledCharRectFromPos(const int& pos);
	std::optional<d2dw::CRectF> GetScrolledCursorCharRectFromPos(const int& pos);
	std::optional<int> GetCharPosFromPoint(const d2dw::CPointF& pt);

	std::optional<int> GetFirstCharPosInLine(const int& pos);
	std::optional<int> GetLastCharPosInLine(const int& pos);


	//int CharPosFromNearPoint(const d2dw::CPointF& pt);
	//BOOL RectFromCharPos(UINT nPos, d2dw::CRectF *prc);

	//UINT FineFirstEndCharPosInLine(UINT uCurPos, BOOL bFirst);

protected:
	CTimer m_timer;
	std::wstring m_strInit;
	CTextCell* m_pCell;
	std::shared_ptr<CellProperty> m_pProp;
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

	COMPOSITIONRENDERINFO *pCompositionRenderInfo_;
	UINT nCompositionRenderInfo_;
	TfEditCookie ecTextStore_;

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

struct TextboxProperty:public CellProperty
{};

class D2DTextbox2 :public D2DTextbox
{
public:
	D2DTextbox2(CWnd* pWnd, const std::shared_ptr<TextboxProperty>& spProp, 
		std::function<std::wstring()> getter,
		std::function<void(const std::wstring&)> setter,
		std::function<void(const std::wstring&)> changed,
		std::function<void(const std::wstring&)> final)
		:D2DTextbox(pWnd, nullptr, spProp, getter, setter, changed, final)
	{
		m_hasBorder = false;
		m_isWrap = false;
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

	virtual void OnContextMenu(const ContextMenuEvent& e) override;
};