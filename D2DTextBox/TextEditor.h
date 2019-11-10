#pragma once

#include "TextLayout.h"
#include "D2DWindow.h"

namespace TSF {
class CTextStore;
class CTextEditSink;
//class CTextContainer;

class CTextEditor 
{
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

		void Render(V4::D2DContext& hdc);
		void CalcRender(V4::D2DContext& hdc);
		int GetSelectionStart() {return m_selStart;}
		int GetSelectionEnd() {return m_selEnd;}

		void InitTSF();
		void UninitTSF();
 
		void InvalidateRect();
		float GetLineHeight() {return layout_.GetLineHeight();}
		CTextLayout *GetLayout() {return &layout_;}
		int CurrentCaretPos();
		void ClearCompositionRenderInfo();
		BOOL AddCompositionRenderInfo(int nStart, int nEnd, TF_DISPLAYATTRIBUTE *pda);
		void OnTextChange(const std::wstring& text);
		IBridgeTSFInterface* bri_;
//		D2D1_MATRIX_3X2_F mat_;

		void SetFocus();

		
		void Reset( IBridgeTSFInterface* rect_size );
	

	protected :
		
		CTextLayout layout_;
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


class CTextEditorCtrl : public CTextEditor
{
	public :
		CTextEditorCtrl(D2DTextbox* pTextbox):CTextEditor(pTextbox){}
		void Create();

		LRESULT  WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam);
		
		
		CTextEditor& GetEditor(){ return *this; }

		void OnEditChanged();

	private :
		//void Move(int x, int y, int nWidth, int nHeight);
		void OnSetFocus(WPARAM wParam, LPARAM lParam);
		void OnPaint(V4::D2DContext& hdc);
		BOOL OnKeyDown(WPARAM wParam, LPARAM lParam);
		void OnLButtonDown(WPARAM wParam, LPARAM lParam);
		void OnLButtonUp(WPARAM wParam, LPARAM lParam);
		void OnMouseMove(WPARAM wParam, LPARAM lParam);

	private:
		UINT _uSelDragStart;

};

// mainframe.cpp‚É‚ ‚é


};
