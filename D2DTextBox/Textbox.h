#pragma once
#include "D2DContextEx.h"
#include "D2DWindow.h"
#include "D2DWindowMessage.h"
#include "D2DWindowMessageStruct.h"
#include "IBridgeTSFInterface.h"
#include "TextEditSink.h"
#include "CellProperty.h"

class LayoutLineInfo;
struct D2DContext;
class D2DWindow;
class CTextStore;
class CTextEditSink;
class IBridgeTSFInterface;
class D2DWindow;

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
	D2DTextbox(D2DWindow* pWnd, const std::wstring& initText, const std::shared_ptr<CellProperty>& pProp,std::function<void(const std::wstring&)> changed);
	~D2DTextbox();
	void InitTSF();
	void UninitTSF();

public:
	LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT EditWndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam);

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
	std::wstring GetText() const{ return m_text; }
	size_t GetTextSize() const { return m_text.size(); }
	int InsertText(LPCWSTR str, int pos = -1, int strlen = -1);
	void EraseText(const size_t off, size_t count);
	void SetText(LPCWSTR str);
	void SetText(VARIANT value);
	void ClearText() { SetText(L""); }
	std::wstring FilterInputString(LPCWSTR s, UINT len);
private:

	BOOL Clipboard(HWND hwnd, TCHAR ch);
	int TabCountCurrentRow();

public:

	// Selection ////////////////////////////////////////
	void MoveSelection(int nSelStart, int nSelEnd, bool bTrail = true);
	BOOL MoveSelectionAtPoint(const CPoint& pt);
	BOOL MoveSelectionAtNearPoint(const CPoint& pt);
	BOOL InsertAtSelection(LPCWSTR psz);
	BOOL DeleteAtSelection(BOOL fBack);
	BOOL DeleteSelection();
	void MoveSelectionNext();
	void MoveSelectionPrev();
	BOOL MoveSelectionUpDown(BOOL bUp, bool bShiftKey);
	BOOL MoveSelectionToLineFirstEnd(BOOL bFirst, bool bShiftKey);
	

	// Render /////////////////////////////////////
	void Render(D2DContext& hdc);
	void CalcRender(D2DContext& hdc);
	bool DrawCaret(D2DContext& cxt, const d2dw::CRectF& rc);
	BOOL Layout(D2DContext& hdc, const WCHAR *psz, int nCnt, const d2dw::CSizeF& sz, int nSelEnd, int& StarCharPos);
	BOOL Render(D2DContext& hdc, const d2dw::CRectF& rc, const WCHAR *psz, int nCnt, int nSelStart, int nSelEnd, bool bSelTrail, const COMPOSITIONRENDERINFO *pCompositionRenderInfo, UINT nCompositionRenderInfo);


	void InvalidateRect();
	int CurrentCaretPos();
	void ClearCompositionRenderInfo();
	BOOL AddCompositionRenderInfo(int nStart, int nEnd, TF_DISPLAYATTRIBUTE *pda);
	void OnTextChange(const std::wstring& text);
public:


	void OnEditChanged();

private:
	int OnKeyDown(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam);
	void OnPaint(D2DContext& hdc);
	BOOL OnKeyDown(WPARAM wParam, LPARAM lParam);
	void OnLButtonDown(WPARAM wParam, LPARAM lParam);
	void OnLButtonUp(WPARAM wParam, LPARAM lParam);
	void OnMouseMove(WPARAM wParam, LPARAM lParam);

public:

public:
	int CharPosFromPoint(const d2dw::CPointF& pt);
	int CharPosFromNearPoint(const d2dw::CPointF& pt);
	BOOL RectFromCharPos(UINT nPos, d2dw::CRectF *prc);

	UINT FineFirstEndCharPosInLine(UINT uCurPos, BOOL bFirst);


private:
	bool bRecalc_;
	D2D1_COLOR_F selected_halftone_color_;

	std::function<void(const std::wstring&)> m_changed;
	std::shared_ptr<CellProperty> m_pProp;

	std::vector<LINEINFO> m_lineInfos;
	UINT nLineCnt_;
	float nLineHeight_;
	FLOAT row_width_;
	int StarCharPos_;

	int m_selStart = 0;
	int m_selEnd = 0;
	int m_startCharPos = 0;
	std::wstring m_text;
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
	int nCompositionRenderInfo_;
	TfEditCookie ecTextStore_;

public:
	D2DWindow* m_pWnd;




public:
#if ( _WIN32_WINNT_WIN8 <= _WIN32_WINNT )
	static ITfThreadMgr2* s_pThreadMgr;
#else
	static ITfThreadMgr* s_pThreadMgr;
#endif
	static TfClientId s_tfClientId;
	static ITfKeystrokeMgr* s_pKeystrokeMgr;







};