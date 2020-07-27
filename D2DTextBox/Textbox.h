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
	bool m_recalc = true;

//	LARGE_INTEGER m_frequency;
	bool m_bCaret = false;
//	LARGE_INTEGER m_gtm, m_pregtm;

	bool m_isFirstDrawCaret = true;
	bool m_isClosing = false;

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
	~D2DTextbox();
private:
	void InitTSF();
	void UninitTSF();
public:
	// Getter
	int GetSelectionStart() { return std::get<caret::SelBegin>(m_carets); }
	int GetSelectionEnd() { return std::get<caret::SelEnd>(m_carets); }
	CTextCell* GetCellPtr() { return m_pCell; }
	//float GetLineWidth() { return row_width_; }
	//UINT GetLineCount() { return nLineCnt_; }

public:
	virtual void OnClose(const CloseEvent& e);
	virtual void OnPaint(const PaintEvent& e);
	virtual void OnKeyDown(const KeyDownEvent& e);
	virtual void OnLButtonDown(const LButtonDownEvent& e);
	virtual void OnMouseMove(const MouseMoveEvent& e);
	virtual void OnChar(const CharEvent& e);

	// IBridgeInterface
	virtual d2dw::CRectF GetClientRect() const;
	d2dw::CRectF GetContentRect() const;

	// Text Functions 
	observable_wstring& GetText() { return m_text; }
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
	std::vector<d2dw::CRectF> GetCharRects();
	std::optional<d2dw::CRectF> GetCharRectFromPos(const int& pos);
	std::optional<int> GetCharPosFromPoint(const d2dw::CPointF& pt);

	std::optional<int> GetFirstCharPosInLine(const int& pos);
	std::optional<int> GetLastCharPosInLine(const int& pos);


	//int CharPosFromNearPoint(const d2dw::CPointF& pt);
	//BOOL RectFromCharPos(UINT nPos, d2dw::CRectF *prc);

	//UINT FineFirstEndCharPosInLine(UINT uCurPos, BOOL bFirst);

private:
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
		:D2DTextbox(pWnd, nullptr, spProp, getter, setter, changed, final){}

	d2dw::CRectF GetClientRect() const override
	{
		CRect rc = m_pWnd->GetClientRect();
		rc.DeflateRect(1);
		return m_pWnd->GetDirectPtr()->Pixels2Dips(rc);
	}

	bool GetIsVisible() const override
	{
		return true;
	}
};