#include "text_stdafx.h"
#include "D2DContextEx.h"
#include <d3d11.h>
#include <dxgi1_2.h>
#include "D2DApi.h"

//#pragma comment(lib,"D2DApi.lib")

static LARGE_INTEGER __s_frequency_;


namespace V4{

SingletonD2DInstance& SingletonD2DInstance::Init()
{
	static SingletonD2DInstance st;
		
	if ( st.factory.p == NULL )
	{
		// Exeにつき１回の実行

		D2D1_FACTORY_OPTIONS options;	
		options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
		THROWIFFAILED( D2D1CreateFactory( D2D1_FACTORY_TYPE_SINGLE_THREADED,__uuidof(ID2D1Factory1),&options,(void**)&st.factory ), L"SingletonD2DInstance::Init()");
		THROWIFFAILED( DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory1), reinterpret_cast<IUnknown**>(&st.wrfactory)), L"SingletonD2DInstance::Init()");
		THROWIFFAILED( st.wrfactory->CreateTextFormat(DEFAULTFONT, 0, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, DEFAULTFONT_HEIGHT, L"", &st.text), L"SingletonD2DInstance::Init()");


		
	}

	return st;
}
void D2DContext::Init(SingletonD2DInstance& ins )
{
	insins = &ins;
}

void D2DContext::CreateRenderTargetResource( ID2D1RenderTarget* rt )
{
	// D2DWindow::CreateD2DWindowのタイミングで実行
	// D2DWindow::WM_SIZEのタイミングで実行

	rt->CreateSolidColorBrush( D2RGB(0,0,0), &black );
	rt->CreateSolidColorBrush( D2RGB(255,255,255), &white );
	rt->CreateSolidColorBrush( D2RGB(192,192,192), &gray );
	rt->CreateSolidColorBrush( D2RGB(255,0,0), &red );
	rt->CreateSolidColorBrush( D2RGB(230,230,230), &ltgray);
	rt->CreateSolidColorBrush( D2RGB(113,113,130), &bluegray);
	rt->CreateSolidColorBrush( D2RGBA(0, 0, 0, 0), &transparent);

	rt->CreateSolidColorBrush( D2RGBA(113,113,130,100), &halftone); 
	rt->CreateSolidColorBrush( D2RGBA(250,113,130,150), &halftoneRed);

	rt->CreateSolidColorBrush( D2RGBA(255,242,0,255), &tooltip);

	rt->CreateSolidColorBrush(D2RGBA(241, 243, 246, 255), &basegray);
	rt->CreateSolidColorBrush(D2RGBA(201, 203, 205, 255), &basegray_line);
	rt->CreateSolidColorBrush(D2RGBA(90,92,98, 255), &basetext);


	// DestroyRenderTargetResourceを忘れないこと
}
void D2DContext::CreateResourceOpt()
{
	// RenderTargetとは関係ないリソース作成
	
	
	ID2D1Factory* factory = insins->factory;		
	
	float dashes[] = {2.0f};

	factory->CreateStrokeStyle(
	D2D1::StrokeStyleProperties(
		D2D1_CAP_STYLE_FLAT, D2D1_CAP_STYLE_FLAT, D2D1_CAP_STYLE_ROUND, D2D1_LINE_JOIN_MITER,
		10.0f,
		D2D1_DASH_STYLE_CUSTOM,
		0.0f),
		dashes, ARRAYSIZE(dashes),
		&dot2_
	);

	float dashes2[] = {4.0f};
	factory->CreateStrokeStyle(
	D2D1::StrokeStyleProperties(
		D2D1_CAP_STYLE_FLAT, D2D1_CAP_STYLE_FLAT, D2D1_CAP_STYLE_ROUND, D2D1_LINE_JOIN_MITER,
		10.0f,
		D2D1_DASH_STYLE_CUSTOM,
		0.0f),
		dashes2, ARRAYSIZE(dashes2),
		&dot4_
	);

	text = insins->text.p;
	wfactory = insins->wrfactory.p;

	QueryPerformanceFrequency( &__s_frequency_ );

}
void D2DContext::DestroyRenderTargetResource()
{
	//UINT a = cxt.p->AddRef()-1;
	//cxt.p->Release();
	//xassert( a == 1 );

	//cxt.Release();

	//ltgray.Release();
	//black.Release();
	//white.Release();
	//red.Release();
	//gray.Release();
	//bluegray.Release();
	//transparent.Release();
	//halftone.Release();
	//halftoneRed.Release();
	//tooltip.Release();

	//basegray.Release();
	//basegray_line.Release();
	//basetext.Release();

}
void D2DContext::DestroyAll()
{
	// OnDestroyのタイミングで実行

	DestroyRenderTargetResource();

	dot2_.Release();
	dot4_.Release();
}

void D2DContextText::Init(D2DContext& cxt1, float height, LPCWSTR fontname )
{	
	line_height = 0;
	xoff = 0;
	cxt = &cxt1;
	wrfactory = cxt1.insins->wrfactory;

	textformat.Release();
	if ( HR(cxt1.insins->wrfactory->CreateTextFormat(fontname,0, DWRITE_FONT_WEIGHT_NORMAL,DWRITE_FONT_STYLE_NORMAL,DWRITE_FONT_STRETCH_NORMAL,height,L"",&textformat)))
	{
		CComPtr<IDWriteTextLayout> tl;
		cxt->wfactory->CreateTextLayout( L"T",1, textformat, 1000, 1000, &tl );
		
		DWRITE_HIT_TEST_METRICS mt;
	
		float y;
		tl->HitTestTextPosition( 0, true,&xoff,&y,&mt );

		line_height = mt.height;


		/*DWRITE_TEXT_METRICS tm;
		GetLineMetric( FSizeF(1000,1000), L"T", 1, tm );

		temp_font_height_ = tm.height;*/

	}
}
UINT D2DContextText::GetLineMetrics( const D2D1_SIZE_F& sz,  LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics, std::vector<DWRITE_LINE_METRICS>& lineMetrics )
{
	CComPtr<IDWriteTextLayout> textlayout;
	cxt->insins->wrfactory->CreateTextLayout(str,len, textformat,(FLOAT)sz.width, (FLOAT)sz.height, &textlayout ); 

	
    textlayout->GetMetrics(&textMetrics);

    lineMetrics.resize(textMetrics.lineCount);
    textlayout->GetLineMetrics(&lineMetrics.front(), textMetrics.lineCount, &textMetrics.lineCount);
    
    return textMetrics.lineCount; // 全行数
}
UINT D2DContextText::GetLineMetric( const D2D1_SIZE_F& sz,  LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics, DWRITE_LINE_METRICS& lineMetric )
{
	std::vector<DWRITE_LINE_METRICS> ar;
	
	UINT r = GetLineMetrics( sz,str,len, textMetrics, ar );
	_ASSERT( r );
	lineMetric = ar[0];
	return r;
}
UINT D2DContextText::GetLineMetric( const D2D1_SIZE_F& sz,  LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics )
{
	return GetLineMetric( sz, textformat, str,len, textMetrics ); 
}
UINT D2DContextText::GetLineMetric( const D2D1_SIZE_F& sz, IDWriteTextFormat* fmt, LPCWSTR str, int len, DWRITE_TEXT_METRICS& textMetrics )
{
	CComPtr<IDWriteTextLayout> tl;
	cxt->insins->wrfactory->CreateTextLayout(str,len, fmt,(FLOAT)sz.width, (FLOAT)sz.height, &tl ); 
    tl->GetMetrics(&textMetrics);
	return textMetrics.lineCount;
}

HRESULT D2DContext::CreateFont(LPCWSTR fontnm, float height, IDWriteTextFormat** ret )
{
	return wfactory->CreateTextFormat( fontnm,0, DWRITE_FONT_WEIGHT_NORMAL,DWRITE_FONT_STYLE_NORMAL,DWRITE_FONT_STRETCH_NORMAL,height,L"", ret );	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void DrawCenterText( D2DContextText& cxt, ID2D1Brush* clr, FRectF& rc, LPCWSTR str, int len, int align )
{	
	DWRITE_TEXT_METRICS tm;
	cxt.GetLineMetric( rc.Size(), str, len, tm );

	FRectF rcX(rc);
	float center = rcX.top + (rcX.bottom-rcX.top)/2.0f;
	rcX.top = center - tm.height/2;
	rcX.bottom = rcX.top + tm.height;

	
	if ( align == 2 )
		rcX.left = rcX.right-tm.width;
	else if ( align == 1 )
	{
		rcX.left = (rcX.right+rcX.left)/2.0f - tm.width / 2.0f;
		rcX.right = rcX.left + tm.width;
	}

	cxt.cxt->cxt->DrawText( str, len, cxt.textformat, rcX, clr, D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP );
}
void TestDrawFillRectEx( D2DContext& cxt,const D2D1_RECT_F& rc, ID2D1Brush* wakuclr,ID2D1Brush* fillclr )
{	
	ID2D1RenderTarget* cxt_ = cxt.cxt;

	FRectF rcc(rc);
	rcc.InflateRect(-1,-1);

	cxt_->DrawRectangle( rcc, wakuclr, 2.0f );	
				
	cxt_->FillRectangle( rcc, fillclr );	
}

void DrawFillRect( D2DContext& cxt, const D2D1_RECT_F& rc, ID2D1Brush* wakuclr,ID2D1Brush* fillclr, float width )
{
	_ASSERT( width > 0 );
		
	//DrawFillRectEx( cxt.cxt, rc, wakuclr, fillclr, width );// Line is FillRectangle.

	DRAWFillRect(cxt.cxt, rc, wakuclr, fillclr, width);
}


void DrawFillRectTypeS( D2DContext& cxt, const D2D1_RECT_F& rc, ID2D1Brush* fillclr )
{
	FRectF yrc(rc);
	yrc.InflateRect(-1,-1);
	cxt.cxt->FillRectangle( yrc, fillclr );	
}



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
		cxt.cxt->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
		cxt.cxt->FillRectangle( rc, ( bCaret ? cxt.black : cxt.white ));
		cxt.cxt->SetAntialiasMode( D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////


CComPtr<ID2D1SolidColorBrush> MakeBrsuh( D2DContext& cxt, D2D1_COLOR_F clr )
{
	CComPtr<ID2D1SolidColorBrush> br;
	cxt.cxt->CreateSolidColorBrush( clr, &br );	
	return br;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SingleLineText::CreateLayoutEx(D2DContextText& cxt,IDWriteTextFormat* fmt, const FRectF& rc, LPCWSTR str, int len, int align )
{	
	DWRITE_TEXT_METRICS tm;

	auto prv = fmt->GetWordWrapping();
	fmt->SetWordWrapping( DWRITE_WORD_WRAPPING_NO_WRAP ); //改行なしへ、

	cxt.GetLineMetric( rc.Size(), fmt, str, len, tm );

	FRectF rcX(rc);
	float center = rcX.top + (rcX.bottom-rcX.top)/2.0f;
	rcX.top = center - tm.height/2;
	rcX.bottom = rcX.top + tm.height;

	
	if ( align == 2 )
		rcX.left = rcX.right-tm.width;
	else if ( align == 1 )
	{
		rcX.left = (rcX.right+rcX.left)/2.0f - tm.width / 2.0f;
		rcX.right = rcX.left + tm.width;
	}

	if ( textlayout.p )
		textlayout.Release();

	ptLineText = rcX.LeftTop();
	FSizeF sz = rcX.Size();
	cxt.cxt->insins->wrfactory->CreateTextLayout(str,len, fmt ,sz.width, sz.height, &textlayout ); 

	fmt->SetWordWrapping( prv );


}
void SingleLineText::CreateLayout(D2DContextText& cxt, const FRectF& rc, LPCWSTR str, int len, int align )
{
	xassert( rc.top == 0 ); // for vertical line calc.
	CreateLayoutEx(cxt,cxt.textformat, rc, str, len, align );
   
}
void SingleLineText::DrawText(D2DContext& cxt, ID2D1Brush* foreclr )
{
	xassert( textlayout.p );

	cxt.cxt->DrawTextLayout( ptLineText,textlayout,foreclr );	
}

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

bool CreateBitmapPartBrush( D2DContext& cxtbase, const FSizeF& size, DrawFunction drawfunc, OUT ID2D1BitmapBrush **ppBitmapBrush )
{
	// 画面のbitmapを作成、画面のハードコピー, FillRectで表示(0,0から引きつめられる)
	// cxtのリソースをすべて再作成する必要あり。
	
	D2DContext cxt;
	ID2D1RenderTarget* pRenderTarget = cxtbase.cxt;
    CComPtr<ID2D1BitmapRenderTarget> pCompatibleRenderTarget;
    
	// create cmpati render target.
	HRESULT hr = pRenderTarget->CreateCompatibleRenderTarget( size, &pCompatibleRenderTarget );

    if (SUCCEEDED(hr))
    {
        if (SUCCEEDED(hr))
        {
            pCompatibleRenderTarget->BeginDraw();
			{
				// change rendertarget.

				D2D1_MATRIX_3X2_F mat = Matrix3x2F::Identity();
				pCompatibleRenderTarget->SetTransform(mat);

				cxt.cxt = pCompatibleRenderTarget;

				drawfunc( cxt );
			}
            pCompatibleRenderTarget->EndDraw();

			// Create bitmap
            CComPtr<ID2D1Bitmap> pGridBitmap;
            hr = pCompatibleRenderTarget->GetBitmap(&pGridBitmap);
            if (SUCCEEDED(hr))
            {
                // Choose the tiling mode for the bitmap brush.
                D2D1_BITMAP_BRUSH_PROPERTIES brushProperties =
                    D2D1::BitmapBrushProperties(D2D1_EXTEND_MODE_WRAP, D2D1_EXTEND_MODE_WRAP);

                // Create the bitmap brush.
                hr = pRenderTarget->CreateBitmapBrush(pGridBitmap, brushProperties, ppBitmapBrush);
            }
        }
    }

    return ( hr == S_OK );
}









/////////////////////////////////////////////////////////////////////////////////
//std::map<std::wstring, CComPtr<ID2D1SolidColorBrush>> ColorBank::bank_;
//
//CComPtr<ID2D1SolidColorBrush> ColorBank::SolidColorBrush( D2DContext& cxt, LPCWSTR name, D2D1_COLOR_F defaultColor )
//{	
//	auto it = bank_.find( name );
//	
//	if ( it == bank_.end())
//	{
//		CComPtr<ID2D1SolidColorBrush> br;
//		cxt.cxt->CreateSolidColorBrush( defaultColor, &br );
//		
//		bank_[name] = br;
//
//		it = bank_.find( name );
//	}
//	
//	return it->second;	
//}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
void D2DStringLayout::Draw( D2DContext& cxt, ID2D1Brush* br )
{
	cxt.cxt->DrawTextLayout( pt_, tl_, br );
}
D2DContextBase D2DStringLayout::CreateFont( D2DContext& cxt, LPCWSTR fontname, float height, DWRITE_FONT_WEIGHT bold )
{
	D2DContextBase ret;
	ret.cxt = cxt.cxt;
	auto hr = cxt.insins->wrfactory->CreateTextFormat(fontname,0, bold,DWRITE_FONT_STYLE_NORMAL,DWRITE_FONT_STRETCH_NORMAL,height,L"",&ret.text);
	xassert( HR(hr));

	ret.wfactory = cxt.insins->wrfactory;
			
	return ret;
}


DWRITE_TEXT_METRICS D2DStringLayout::GetMetrics()
{
	xassert( tl_ );	
	DWRITE_TEXT_METRICS tm;
	tl_->GetMetrics(&tm);
	return tm;
}
bool D2DStringLayout::CreateCenterText( D2DContextBase& cxt, const FRectF& rc, LPCWSTR str, int len, int align )
{	
	if ( !IsFirst())
		Clear();

	CComPtr<IDWriteTextLayout> tl;
	FSizeF sz = rc.GetSize();
	if (HR(cxt.wfactory->CreateTextLayout(str,len, cxt.text,sz.width, sz.height, &tl )))
	{
		DWRITE_TEXT_METRICS tm;

		tl->GetMetrics(&tm);

		FRectF rcX(rc);
		float center = rcX.top + (rcX.bottom-rcX.top)/2.0f;
		rcX.top = center - tm.height/2;
		rcX.bottom = rcX.top + tm.height;

	
		if ( align == 2 )
			rcX.left = rcX.right-tm.width;
		else if ( align == 1 )
		{
			rcX.left = (rcX.right+rcX.left)/2.0f - tm.width / 2.0f;
			rcX.right = rcX.left + tm.width;
		}
		
		tl_ = tl;
		pt_ = rcX.LeftTop();		
	}
	else 
		return false;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////




};