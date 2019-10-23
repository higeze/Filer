#include "text_stdafx.h"
#include "D2DContextEx.h"
#include <d3d11.h>
#include <dxgi1_2.h>
#include "D2DApi.h"


static LARGE_INTEGER __s_frequency_;


namespace V4{


void D2DContext::Init()
{
	QueryPerformanceFrequency( &__s_frequency_ );
}

UINT D2DContext::GetLineMetrics( const D2D1_SIZE_F& sz,  LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics, std::vector<DWRITE_LINE_METRICS>& lineMetrics )
{
	CComPtr<IDWriteTextLayout> textlayout;
	m_pDirect->GetDWriteFactory()->CreateTextLayout(str,len, textformat,(FLOAT)sz.width, (FLOAT)sz.height, &textlayout ); 

	
    textlayout->GetMetrics(&textMetrics);

    lineMetrics.resize(textMetrics.lineCount);
    textlayout->GetLineMetrics(&lineMetrics.front(), textMetrics.lineCount, &textMetrics.lineCount);
    
    return textMetrics.lineCount; // 全行数
}
UINT D2DContext::GetLineMetric( const D2D1_SIZE_F& sz,  LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics, DWRITE_LINE_METRICS& lineMetric )
{
	std::vector<DWRITE_LINE_METRICS> ar;
	
	UINT r = GetLineMetrics( sz,str,len, textMetrics, ar );
	_ASSERT( r );
	lineMetric = ar[0];
	return r;
}
UINT D2DContext::GetLineMetric( const D2D1_SIZE_F& sz,  LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics )
{
	return GetLineMetric( sz, textformat, str,len, textMetrics ); 
}
UINT D2DContext::GetLineMetric( const D2D1_SIZE_F& sz, IDWriteTextFormat* fmt, LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics )
{
	CComPtr<IDWriteTextLayout> tl;
	m_pDirect->GetDWriteFactory()->CreateTextLayout(str,len, fmt,(FLOAT)sz.width, (FLOAT)sz.height, &tl ); 
    tl->GetMetrics(&textMetrics);
	return textMetrics.lineCount;
}

HRESULT D2DContext::CreateFont(LPCWSTR fontnm, float height, IDWriteTextFormat** ret )
{
	return m_pDirect->GetDWriteFactory()->CreateTextFormat( fontnm,0, DWRITE_FONT_WEIGHT_NORMAL,DWRITE_FONT_STYLE_NORMAL,DWRITE_FONT_STRETCH_NORMAL,height,L"", ret );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//void DrawCenterText( D2DContextText& cxt, ID2D1Brush* clr, FRectF& rc, LPCWSTR str, int len, int align )
//{	
//	DWRITE_TEXT_METRICS tm;
//	cxt.GetLineMetric( rc.Size(), str, len, tm );
//
//	FRectF rcX(rc);
//	float center = rcX.top + (rcX.bottom-rcX.top)/2.0f;
//	rcX.top = center - tm.height/2;
//	rcX.bottom = rcX.top + tm.height;
//
//	
//	if ( align == 2 )
//		rcX.left = rcX.right-tm.width;
//	else if ( align == 1 )
//	{
//		rcX.left = (rcX.right+rcX.left)/2.0f - tm.width / 2.0f;
//		rcX.right = rcX.left + tm.width;
//	}
//
//	cxt.cxt->insins->m_pDirect->GetHwndRenderTarget()->DrawText( str, len, cxt.textformat, rcX, clr, D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP );
//}
//void TestDrawFillRectEx( D2DContext& cxt,const D2D1_RECT_F& rc, ID2D1Brush* wakuclr,ID2D1Brush* fillclr )
//{	
//	FRectF rcc(rc);
//	rcc.InflateRect(-1,-1);
//
//	cxt.insins->m_pDirect->GetHwndRenderTarget()->DrawRectangle( rcc, wakuclr, 2.0f );
//				
//	cxt.insins->m_pDirect->GetHwndRenderTarget()->FillRectangle( rcc, fillclr );
//}
//
void DrawFillRect( D2DContext& cxt, const D2D1_RECT_F& rc, ID2D1Brush* wakuclr,ID2D1Brush* fillclr, float width )
{
	_ASSERT( width > 0 );
		
	//DrawFillRectEx( cxt.cxt, rc, wakuclr, fillclr, width );// Line is FillRectangle.

	DRAWFillRect(cxt.m_pDirect->GetHwndRenderTarget(), rc, wakuclr, fillclr, width);
}
//
//
//void DrawFillRectTypeS( D2DContext& cxt, const D2D1_RECT_F& rc, ID2D1Brush* fillclr )
//{
//	FRectF yrc(rc);
//	yrc.InflateRect(-1,-1);
//	cxt.insins->m_pDirect->GetHwndRenderTarget()->FillRectangle( yrc, fillclr );
//}



///TSF////////////////////////////////////////////////////////////////////////////////////////////

static bool bCaret = false;
static LARGE_INTEGER gtm,pregtm;

// activeを黒色から即スタート
void CaretActive()
{
	bCaret = true;
	QueryPerformanceCounter(&pregtm);
}
bool DrawCaret(D2DContext& cxt, const FRectF& rc )
{
	QueryPerformanceCounter(&gtm);
						
	float zfps = (float)(gtm.QuadPart-pregtm.QuadPart) / (float)__s_frequency_.QuadPart;

	if  ( zfps  > 0.4f )
	{
		pregtm = gtm;
		bCaret = !bCaret;
	}
	else
	{	
		cxt.m_pDirect->GetHwndRenderTarget()->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
		cxt.m_pDirect->GetHwndRenderTarget()->FillRectangle( rc, ( bCaret ? cxt.black : cxt.white ));
		cxt.m_pDirect->GetHwndRenderTarget()->SetAntialiasMode( D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////


CComPtr<ID2D1SolidColorBrush> MakeBrsuh( D2DContext& cxt, D2D1_COLOR_F clr )
{
	CComPtr<ID2D1SolidColorBrush> br;
	cxt.m_pDirect->GetHwndRenderTarget()->CreateSolidColorBrush( clr, &br );
	return br;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void SingleLineText::CreateLayoutEx(D2DContextText& cxt,IDWriteTextFormat* fmt, const FRectF& rc, LPCWSTR str, int len, int align )
//{	
//	DWRITE_TEXT_METRICS tm;
//
//	auto prv = fmt->GetWordWrapping();
//	fmt->SetWordWrapping( DWRITE_WORD_WRAPPING_NO_WRAP ); //改行なしへ、
//
//	cxt.GetLineMetric( rc.Size(), fmt, str, len, tm );
//
//	FRectF rcX(rc);
//	float center = rcX.top + (rcX.bottom-rcX.top)/2.0f;
//	rcX.top = center - tm.height/2;
//	rcX.bottom = rcX.top + tm.height;
//
//	
//	if ( align == 2 )
//		rcX.left = rcX.right-tm.width;
//	else if ( align == 1 )
//	{
//		rcX.left = (rcX.right+rcX.left)/2.0f - tm.width / 2.0f;
//		rcX.right = rcX.left + tm.width;
//	}
//
//	if ( textlayout.p )
//		textlayout.Release();
//
//	ptLineText = rcX.LeftTop();
//	FSizeF sz = rcX.Size();
//	cxt.cxt->insins->m_pDirect->GetDWriteFactory()->CreateTextLayout(str,len, fmt ,sz.width, sz.height, &textlayout ); 
//
//	fmt->SetWordWrapping( prv );
//
//
//}
//void SingleLineText::CreateLayout(D2DContextText& cxt, const FRectF& rc, LPCWSTR str, int len, int align )
//{
//	xassert( rc.top == 0 ); // for vertical line calc.
//	CreateLayoutEx(cxt,cxt.textformat, rc, str, len, align );
//   
//}
//void SingleLineText::DrawText(D2DContext& cxt, ID2D1Brush* foreclr )
//{
//	xassert( textlayout.p );
//
//	cxt.m_pDirect->GetHwndRenderTarget()->DrawTextLayout( ptLineText,textlayout,foreclr );
//}

////////////////////////////////////////////////////////////////////////////////////////
FRectF FRectFV( _variant_t& x,_variant_t& y,_variant_t& cx,_variant_t& cy )
{
	float fx; x.ChangeType( VT_R4 ); fx = x.fltVal;
	float fy; y.ChangeType( VT_R4 ); fy = y.fltVal;
	float fcx; cx.ChangeType( VT_R4 ); fcx = cx.fltVal+fx;
	float fcy; cy.ChangeType( VT_R4 ); fcy = cy.fltVal+fy;

	return FRectF(fx,fy,fcx,fcy );
}
FSizeF FSizeFV( _variant_t& cx,_variant_t& cy )
{
	float fcx; cx.ChangeType( VT_R4 ); fcx = cx.fltVal;
	float fcy; cy.ChangeType( VT_R4 ); fcy = cy.fltVal;

	return FSizeF( fcx,fcy );

}
FPointF FPointFV( _variant_t& cx,_variant_t& cy )
{
	float fcx; cx.ChangeType( VT_R4 ); fcx = cx.fltVal;
	float fcy; cy.ChangeType( VT_R4 ); fcy = cy.fltVal;
	return FPointF( fcx,fcy );
}
FString FStringV( _variant_t& s )
{
	s.ChangeType( VT_BSTR );

	return s.bstrVal;
}

};