#pragma once
#include <msctf.h>
#include "IBridgeTSFInterface.h"
#include "CellProperty.h"
#include "UIElement.h"
#include "observable.h"

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

struct CHARINFO
{
	d2dw::CRectF rc;
	float GetWidth() { return rc.right - rc.left; }
};

struct LINEINFO
{
	int nPos;
	int nCnt;
	std::vector<CHARINFO> CharInfos;
};

class CaretRect
{
public:
	CaretRect(bool bTrail) :bTrail_(bTrail), cnt_(0) {};

	void Push(const d2dw::CRectF& rc, int row, int colStart, int colLast)
	{
		if (bTrail_) {
			rc_ = rc;
			row_ = row;
			col_ = colLast;
		} else if (!bTrail_ && cnt_ == 0) {
			rc_ = rc;
			row_ = row;
			col_ = colStart;
		}

		cnt_++;
	}

	d2dw::CRectF Get() { return rc_; }
	bool empty() { return cnt_ == 0; }

	int row() { return row_; }
	int col() { return col_; }


	bool IsComplete(int rowno)
	{
		if (cnt_ == 0) return false;

		if (bTrail_) {
			if (row_ == rowno)
				return false;
		}

		return true;
	}

private:
	bool bTrail_;
	int cnt_;
	d2dw::CRectF rc_;
	int row_, col_;
};


class D2DTextbox: public IBridgeTSFInterface
{
public:
	static bool AppTSFInit();
	static void AppTSFExit();

public:
	D2DTextbox(
		CGridView* pWnd, 
		CTextCell* pCell,
		std::shared_ptr<CellProperty> pProp,
		std::function<std::wstring()> getter,
		std::function<void(const std::wstring&)> setter,
		std::function<void(const std::wstring&)> changed,
		std::function<void()> final);
	~D2DTextbox();
	void InitTSF();
	void UninitTSF();

public:
	//LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
	virtual void OnCreate(const CreateEvent& e);
	virtual void OnClose(const CloseEvent& e);
	virtual void OnPaint(const PaintEvent& e);
	virtual void OnKeyDown(const KeyDownEvent& e);
	virtual void OnLButtonDown(const LButtonDownEvent& e);
	virtual void OnLButtonUp(const LButtonUpEvent& e);
	virtual void OnMouseMove(const MouseMoveEvent& e);
	virtual void OnChar(const CharEvent& e);
	//virtual void OnKillFocus(const KillFocusEvent& e);

	// Getter ////////////////////////////////////////////////////
	int GetSelectionStart() { return m_selStart; }
	int GetSelectionEnd() { return m_selEnd; }
	float GetLineHeight() { return nLineHeight_; }
	float GetLineWidth() { return row_width_; }
	UINT GetLineCount() { return nLineCnt_; }


	// IBridgeInterface///////////////////////////////////////////
	d2dw::CRectF GetClientRect() const;
	d2dw::CRectF GetContentRect() const;

	// Text Functions ////////////////////////////////////////
	observable_wstring& GetText() { return m_text; }
	std::wstring FilterInputString(LPCWSTR s, UINT len);
private:

	BOOL Clipboard(HWND hwnd, TCHAR ch);

public:

	// Selection ////////////////////////////////////////
	void MoveSelection(int nSelStart, int nSelEnd, bool bTrail = true);
	BOOL MoveSelectionAtPoint(const d2dw::CPointF& pt);
	BOOL MoveSelectionAtNearPoint(const d2dw::CPointF& pt);
	BOOL InsertAtSelection(LPCWSTR psz);
	BOOL DeleteAtSelection(BOOL fBack);
	BOOL DeleteSelection();
	void SetText(LPCWSTR str);
	void CancelEdit();
	void ClearText();
	void MoveSelectionNext();
	void MoveSelectionPrev();
	BOOL MoveSelectionUpDown(BOOL bUp, bool bShiftKey);
	BOOL MoveSelectionToLineFirstEnd(BOOL bFirst, bool bShiftKey);
	

	// Render /////////////////////////////////////
	void Render();
	void DrawCaret(const d2dw::CRectF& rc);
	BOOL Layout();


	void InvalidateRect();
	void ClearCompositionRenderInfo();
	BOOL AddCompositionRenderInfo(int nStart, int nEnd, TF_DISPLAYATTRIBUTE *pda);
public:
	int CharPosFromPoint(const d2dw::CPointF& pt);
	int CharPosFromNearPoint(const d2dw::CPointF& pt);
	BOOL RectFromCharPos(UINT nPos, d2dw::CRectF *prc);

	UINT FineFirstEndCharPosInLine(UINT uCurPos, BOOL bFirst);

private:
	std::wstring m_strInit;
	CTextCell* m_pCell;
	std::shared_ptr<CellProperty> m_pProp;
	std::function<std::wstring()> m_getter;
	std::function<void(const std::wstring&)> m_setter;
	std::function<void(const std::wstring&)> m_changed;
	std::function<void()> m_final;
	bool bRecalc_;

	std::vector<LINEINFO> m_lineInfos;
	UINT nLineCnt_;
	float nLineHeight_;
	FLOAT row_width_;
	int StarCharPos_;

	int m_selStart = 0;
	int m_selEnd = 0;
	int m_startCharPos = 0;
	observable_wstring m_text;
	bool m_isSelTrail = true;
	bool m_isSingleLine = false;

	UINT m_selDragStart;

	LARGE_INTEGER m_frequency;
	bool m_bCaret = false;
	LARGE_INTEGER m_gtm, m_pregtm;

	CComPtr<CTextEditSink> m_pTextEditSink;
	CComPtr<CTextStore> m_pTextStore;
	CComPtr<ITfDocumentMgr> m_pDocumentMgr;
	CComPtr<ITfContext> m_pInputContext;

	COMPOSITIONRENDERINFO *pCompositionRenderInfo_;
	UINT nCompositionRenderInfo_;
	TfEditCookie ecTextStore_;

public:
	CGridView* m_pWnd;

public:
#if ( _WIN32_WINNT_WIN8 <= _WIN32_WINNT )
	static ITfThreadMgr2* s_pThreadMgr;
#else
	static ITfThreadMgr* s_pThreadMgr;
#endif
	static TfClientId s_tfClientId;
	static ITfKeystrokeMgr* s_pKeystrokeMgr;

};