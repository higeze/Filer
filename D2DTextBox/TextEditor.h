#pragma once

struct D2DContext;
class D2DWindow;
class CTextStore;
class CTextEditSink;
class D2DTextbox;
class IBridgeTSFInterface;

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


class CTextEditor 
{
private:
	UINT m_selDragStart;

	LARGE_INTEGER m_frequency;
	bool m_bCaret = false;
	LARGE_INTEGER m_gtm, m_pregtm;

	bool DrawCaret(D2DContext& cxt, const d2dw::CRectF& rc);

	public:
		int m_selStart = 0;
		int m_selEnd = 0;
		int m_startCharPos = 0;
		std::wstring m_text;
		bool m_isSelTrail = true;
		bool m_isSingleLine = false;

		std::function<void(const std::wstring&)> m_changed;
		D2DTextbox* m_pTxtbox;

		CTextEditor(D2DTextbox* pTxtbox);
		virtual ~CTextEditor(); 
        
		void MoveSelection(int nSelStart, int nSelEnd, bool bTrail=true);
		BOOL MoveSelectionAtPoint(const CPoint& pt);
		BOOL MoveSelectionAtNearPoint(const CPoint& pt);
		BOOL InsertAtSelection(LPCWSTR psz);
		BOOL DeleteAtSelection(BOOL fBack);
		BOOL DeleteSelection();

		void MoveSelectionNext();
		void MoveSelectionPrev();
		BOOL MoveSelectionUpDown(BOOL bUp, bool bShiftKey);
		BOOL MoveSelectionToLineFirstEnd(BOOL bFirst, bool bShiftKey);

		void Render(D2DContext& hdc);
		void CalcRender(D2DContext& hdc);
		int GetSelectionStart() {return m_selStart;}
		int GetSelectionEnd() {return m_selEnd;}

		void InitTSF();
		void UninitTSF();
 
		void InvalidateRect();
		int CurrentCaretPos();
		void ClearCompositionRenderInfo();
		BOOL AddCompositionRenderInfo(int nStart, int nEnd, TF_DISPLAYATTRIBUTE *pda);
		void OnTextChange(const std::wstring& text);
		IBridgeTSFInterface* bri_;

		void SetFocus();

		
		void Reset( IBridgeTSFInterface* rect_size );
public:
		void Create();

		LRESULT  WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam);
		CTextEditor& GetEditor() { return *this; }

		void OnEditChanged();

private:
	//void Move(int x, int y, int nWidth, int nHeight);
	void OnSetFocus(WPARAM wParam, LPARAM lParam);
	void OnPaint(D2DContext& hdc);
	BOOL OnKeyDown(WPARAM wParam, LPARAM lParam);
	void OnLButtonDown(WPARAM wParam, LPARAM lParam);
	void OnLButtonUp(WPARAM wParam, LPARAM lParam);
	void OnMouseMove(WPARAM wParam, LPARAM lParam);

public:
	BOOL Layout(D2DContext& hdc, const WCHAR *psz, int nCnt, const d2dw::CSizeF& sz, int nSelEnd, int& StarCharPos);
	BOOL Render(D2DContext& hdc, const d2dw::CRectF& rc, const WCHAR *psz, int nCnt, int nSelStart, int nSelEnd, bool bSelTrail, const COMPOSITIONRENDERINFO *pCompositionRenderInfo, UINT nCompositionRenderInfo);

public:
	int CharPosFromPoint(const d2dw::CPointF& pt);
	int CharPosFromNearPoint(const d2dw::CPointF& pt);
	BOOL RectFromCharPos(UINT nPos, d2dw::CRectF *prc);

	UINT FineFirstEndCharPosInLine(UINT uCurPos, BOOL bFirst);
	float GetLineHeight() { return nLineHeight_; }
	float GetLineWidth() { return row_width_; }

	UINT GetLineCount() { return nLineCnt_; }
	//		UINT GetViewLineCount(){ return tm_.lineCount; } // １行でも折り返されれば、２行になる。折り返し分を含めた行数

	bool bRecalc_;
	D2D1_COLOR_F selected_halftone_color_;
private:
	void Clear();

	std::vector<LINEINFO> m_lineInfos;
	UINT nLineCnt_;
	float nLineHeight_;
	FLOAT row_width_;
	int StarCharPos_;



	

	protected :
		CComPtr<CTextEditSink> m_pTextEditSink;	
		CComPtr<CTextStore> m_pTextStore;
		CComPtr<ITfDocumentMgr> m_pDocumentMgr;
		CComPtr<ITfContext> m_pInputContext;
		COMPOSITIONRENDERINFO *pCompositionRenderInfo_;
		int nCompositionRenderInfo_;
		TfEditCookie ecTextStore_;
};


class CTextEditSink : public ITfTextEditSink
{
	public:
		CTextEditSink(CTextEditor *pEditor);

		//
		// IUnknown methods
		//
		STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
		STDMETHODIMP_(ULONG) AddRef(void);
		STDMETHODIMP_(ULONG) Release(void);

		//
		// ITfTextEditSink
		//
		STDMETHODIMP OnEndEdit(ITfContext *pic, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord);

		HRESULT _Advise(ITfContext *pic);
		HRESULT _Unadvise();


		std::function<void()> OnChanged_;

	private:
		long _cRef;
		ITfContext *_pic;
		DWORD _dwEditCookie;
		CTextEditor *_pEditor;
};
