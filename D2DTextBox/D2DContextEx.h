#pragma once
#include "text_stdafx.h"
#include <dxgi1_2.h>
#include "Direct2DWrite.h"

class D2DWindow;


struct D2DContext
{	
	D2DWindow* pWindow;


	CComPtr<ID2D1StrokeStyle> dot4_;
	CComPtr<ID2D1StrokeStyle> dot2_;

	LPVOID free_space;

	void Init();

	UINT GetLineMetrics(const D2D1_SIZE_F& sz, LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics, std::vector<DWRITE_LINE_METRICS>& lineMetrics);
	UINT GetLineMetric(const D2D1_SIZE_F& sz, LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics, DWRITE_LINE_METRICS& lineMetric);
	UINT GetLineMetric(const D2D1_SIZE_F& sz, LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics);
	UINT GetLineMetric(const D2D1_SIZE_F& sz, IDWriteTextFormat* fmt, LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics);

	CComPtr<IDWriteTextFormat> textformat;
};

///////////////////////////////////////////////////////////////////////////////////////////
class D2DError 
{
	public :
		explicit D2DError( HRESULT hr, LPCWSTR msg, UINT line, LPCSTR fnm):hr_(hr),msg_(msg),line_(line),fnm_(fnm )
		{
//			TRACE(L"%s :%d行 HR=%x %s\n", (LPCWSTR)fnm_, line_,hr_,(LPCWSTR)msg_);		
		}

	public :
		CComBSTR msg_;
		CComBSTR fnm_;
		UINT line_;
		HRESULT hr_;

};
inline void ThrowIfFailed( HRESULT hr, LPCWSTR msg, UINT line, LPCSTR fnm )
{
	if (FAILED(hr))
	{
		throw D2DError( hr, msg, line,fnm );
		//General access denied error 0x80070005 
	}
}
#define THROWIFFAILED(hr,msg) ThrowIfFailed(hr,msg, __LINE__, __FILE__)

//inline HRESULT NoThrowIfFailed(HRESULT hr)
//{
//    if (FAILED(hr))
//    {
//        // Set a breakpoint on this line to catch Win32 API errors.
////		TRACE( L"NoThrowIfFailed %x\n", hr );        
//    }
//
//	return hr;
//}

//void CaretActive(); // D2DContextEx.cpp

//////////////////////////////////////////////////////////////////////////////////////
// Variant
d2dw::CRectF FRectFV( _variant_t& x,_variant_t& y,_variant_t& cx,_variant_t& cy );
d2dw::CSizeF FSizeFV( _variant_t& cx,_variant_t& cy );
d2dw::CPointF FPointFV( _variant_t& cx,_variant_t& cy );
std::wstring FStringV( _variant_t& s );


