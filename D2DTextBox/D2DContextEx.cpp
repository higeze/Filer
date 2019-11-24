#include "text_stdafx.h"
#include "D2DContextEx.h"
#include <d3d11.h>
#include <dxgi1_2.h>
#include "D2DWindow.h"


void D2DContext::Init()
{
}

UINT D2DContext::GetLineMetrics( const D2D1_SIZE_F& sz,  LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics, std::vector<DWRITE_LINE_METRICS>& lineMetrics )
{
	CComPtr<IDWriteTextLayout> textlayout;
	pWindow->m_pDirect->GetDWriteFactory()->CreateTextLayout(str,len, textformat,(FLOAT)sz.width, (FLOAT)sz.height, &textlayout ); 

	
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
	pWindow->m_pDirect->GetDWriteFactory()->CreateTextLayout(str,len, fmt,(FLOAT)sz.width, (FLOAT)sz.height, &tl );
    tl->GetMetrics(&textMetrics);
	return textMetrics.lineCount;
}



	///TSF////////////////////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////////////////////////

//d2dw::CRectF FRectFV( _variant_t& x,_variant_t& y,_variant_t& cx,_variant_t& cy )
//{
//	float fx; x.ChangeType( VT_R4 ); fx = x.fltVal;
//	float fy; y.ChangeType( VT_R4 ); fy = y.fltVal;
//	float fcx; cx.ChangeType( VT_R4 ); fcx = cx.fltVal+fx;
//	float fcy; cy.ChangeType( VT_R4 ); fcy = cy.fltVal+fy;
//
//	return d2dw::CRectF(fx,fy,fcx,fcy );
//}
//d2dw::CSizeF FSizeFV( _variant_t& cx,_variant_t& cy )
//{
//	float fcx; cx.ChangeType( VT_R4 ); fcx = cx.fltVal;
//	float fcy; cy.ChangeType( VT_R4 ); fcy = cy.fltVal;
//
//	return d2dw::CSizeF( fcx, fcy );
//
//}
//d2dw::CPointF FPointFV( _variant_t& cx,_variant_t& cy )
//{
//	float fcx; cx.ChangeType( VT_R4 ); fcx = cx.fltVal;
//	float fcy; cy.ChangeType( VT_R4 ); fcy = cy.fltVal;
//	return d2dw::CPointF( fcx,fcy );
//}
//std::wstring FStringV( _variant_t& s )
//{
//	//s.ChangeType( VT_BSTR );
//
//	return std::wstring(s.bstrVal);
//}
