// Direct2dTemplate.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "Direct2dTemplate.h"
#include "D2DWindow.h"
#include "D2DWindowControl.h"
#include "gdi32.h"
#include "d2dcontextnew.h"
#include "TextEditor.h"
#include "EntryD2D.h"
using namespace GDI32;

#define WINDOW_TITLE L"D2DMainFrame H version"
#define MAX_LOADSTRING 100
#define st __s_d2dmainframe

static HWND ghMainFrameWnd;

// グローバル変数:
static HINSTANCE __hInst;						// 現在のインターフェイス
TCHAR __szTitle[MAX_LOADSTRING];				// タイトル バーのテキスト
TCHAR __szWindowClass[MAX_LOADSTRING];			// メイン ウィンドウ クラス名

#define IDLE_TIME (5*1000)
#define IDLE_TIMER_ID 99

V4::D2DWindow window; 

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
void				ExitInstance();

LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;

	CoInitialize(0);
	_tsetlocale ( 0, _T("japanese") ); 	 
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	// グローバル文字列を初期化しています。
	LoadString(hInstance, IDS_APP_TITLE, __szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_DIRECT2DTEMPLATE, __szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// アプリケーションの初期化を実行します:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DIRECT2DTEMPLATE));

	// メイン メッセージ ループ:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	ExitInstance();
	return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW |CS_DBLCLKS;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DIRECT2DTEMPLATE));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)GetStockObject(GRAY_BRUSH);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= __szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	D2DTextbox::AppTSFInit();

   __hInst = hInstance; // グローバル変数にインスタンス処理を格納します。

   HWND hMainFrameWnd = CreateWindowW(__szWindowClass, __szTitle, WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
      CW_USEDEFAULT, 0, 1.5*900, 800, NULL, NULL, hInstance, NULL);

   if (!hMainFrameWnd )
   {
      return FALSE;
   }

   ShowWindow(hMainFrameWnd, nCmdShow);
   UpdateWindow(hMainFrameWnd);

   ghMainFrameWnd = hMainFrameWnd;
   return TRUE;

}
void ExitInstance()
{
	D2DTextbox::AppTSFExit();

	CoUninitialize();

}

struct D2DMainFrame
{
	CComPtr<IDWriteFactory> wrfactory;
	CComPtr<ID2D1Factory>  factory;
	CComPtr<IDWriteTextFormat> textformat;
	CComPtr<ID2D1HwndRenderTarget> cxt;

	CComPtr<ID2D1SolidColorBrush> br[4];
	CComPtr<ID2D1SolidColorBrush> black, white;

	FRectF btn[3];
	UINT btnStat;
	LPCWSTR title;
	
	enum COLORS{ MOUSE_FLOAT,CLOSEBTN,ACTIVECAPTION,CLOSE_MOUSE_FLOAT };
};

static D2DMainFrame __s_d2dmainframe;
void D2DInitial(HWND hWnd1)
{
	HRESULT hr;
	D2D1_FACTORY_OPTIONS options;	
	options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
	hr = D2D1CreateFactory( D2D1_FACTORY_TYPE_SINGLE_THREADED,__uuidof(ID2D1Factory),&options,(void**)&st.factory );
	_ASSERT(hr==S_OK);

	hr = DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&st.wrfactory));
	_ASSERT(hr==S_OK);

	hr = st.wrfactory->CreateTextFormat( L"Arial",0,DWRITE_FONT_WEIGHT_NORMAL,DWRITE_FONT_STYLE_NORMAL,DWRITE_FONT_STRETCH_NORMAL,14,L"",&st.textformat);
	_ASSERT(hr==S_OK);

	hr = st.factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd1, D2D1::SizeU(1,1), D2D1_PRESENT_OPTIONS_NONE), &st.cxt);
	_ASSERT(hr==S_OK);

	st.cxt->CreateSolidColorBrush( ColorF(ColorF::Black), &st.black );
	st.cxt->CreateSolidColorBrush( ColorF(ColorF::White), &st.white );
	st.cxt->CreateSolidColorBrush( D2RGBA(54,101,179,255), &st.br[D2DMainFrame::MOUSE_FLOAT] );
	st.cxt->CreateSolidColorBrush( D2RGBA(199,80,80,255), &st.br[D2DMainFrame::CLOSEBTN] );
	st.cxt->CreateSolidColorBrush( D2RGBA(144,169,184,255), &st.br[D2DMainFrame::ACTIVECAPTION] );
	st.cxt->CreateSolidColorBrush( D2RGBA(224,67,67,255), &st.br[D2DMainFrame::CLOSE_MOUSE_FLOAT] );

	st.btn[0] = FRectF(0,0,26,20); // MINI BUTTON
	st.btn[1] = FRectF(0,0,27,20); // MAX BUTTON
	st.btn[2] = FRectF(0,0,45,20); // CLOSE BUTTON

	st.btnStat = 0;

	st.title = WINDOW_TITLE;
	
	st.factory.Release();
	st.wrfactory.Release();

}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 選択されたメニューの解析:
		switch (wmId)
		{
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		
		break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_CREATE:
	{
		D2DInitial(hWnd);

		RECT rc = { 0 };
		rc.left = 30;
		rc.top = 30;
		rc.right = 400 + 30;
		rc.bottom = 100 + 30;
		//::GetClientRect(hWnd,&rc);

		UINT id[] = { IDB_PNG1 };
		HWND hwnd = window.CreateD2DWindow( 0, hWnd, WS_CHILD|WS_VISIBLE, rc, id, _countof(id) );
		
		auto IdleMessage = [](HWND hwnd, UINT msg, UINT_PTR id, DWORD time )
		{
			if ( IDLE_TIMER_ID == id )
				SendMessage(hwnd, WM_D2D_IDLE, 0, 0);
		};

		::SetTimer( hWnd, IDLE_TIMER_ID, IDLE_TIME, IdleMessage );
	}
	break;
	case WM_SIZE:
	{
		UINT cx =  LOWORD(lParam);
		UINT cy =  HIWORD(lParam);	
		st.cxt->Resize( D2D1::SizeU(cx,cy));

		FSize sz(lParam);
		::MoveWindow( window.hWnd_, 30, 30, 400, 100, TRUE );
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	break;
	case WM_PAINT:
	{
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
	break;
	default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
