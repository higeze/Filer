#pragma once

#include "D2DContextEx.h"

using namespace V4;
namespace TSF {

struct COMPOSITIONRENDERINFO 
{
    int nStart;
    int nEnd;
    TF_DISPLAYATTRIBUTE da;
};
//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

struct CHARINFO 
{
	d2dw::CRectF rc;
    float GetWidth() {return rc.right - rc.left;}
};

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

struct LINEINFO 
{
    int nPos;
    int nCnt;
    CHARINFO *prgCharInfo;
};


//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

class CTextLayout
{
	public:
		CTextLayout();
		virtual ~CTextLayout();

		BOOL Layout(D2DContext& hdc, const WCHAR *psz,  int nCnt, const d2dw::CSizeF& sz, bool bSingleLine, int nSelEnd,int& StarCharPos, IDWriteTextFormat* fmt);		
		BOOL Render(D2DContext& hdc, const d2dw::CRectF& rc, const WCHAR *psz,  int nCnt, int nSelStart, int nSelEnd,bool bSelTrail,const COMPOSITIONRENDERINFO *pCompositionRenderInfo, UINT nCompositionRenderInfo);

	public :
		int CharPosFromPoint(const d2dw::CPointF& pt);
		int CharPosFromNearPoint(const d2dw::CPointF& pt);
		BOOL RectFromCharPos(UINT nPos, d2dw::CRectF *prc);
    
		UINT FineFirstEndCharPosInLine(UINT uCurPos, BOOL bFirst);
		float GetLineHeight() {return nLineHeight_;}
		float GetLineWidth(){ return row_width_; }

		UINT GetLineCount(){ return nLineCnt_; }
//		UINT GetViewLineCount(){ return tm_.lineCount; } // １行でも折り返されれば、２行になる。折り返し分を含めた行数

		bool bRecalc_;
		D2D1_COLOR_F selected_halftone_color_;
	private:
		void Clear();

		LINEINFO *prgLines_;
		UINT nLineCnt_;
		float nLineHeight_;
		FLOAT row_width_;
		bool bSingleLine_;
		int StarCharPos_;
};

};

