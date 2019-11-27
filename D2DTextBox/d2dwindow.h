#pragma once

//#include "D2DContextEx.h"
#include "D2DWindowMessage.h"
#include "D2DWindowMessageStruct.h"
#include "Direct2DWrite.h"

#undef CreateWindow


#define ENABLE( stat ) (!(stat&STAT::DISABLE))
#define VISIBLE( stat ) (stat&STAT::VISIBLE)
#define DEAD( stat ) (stat&STAT::DEAD)
#define BORDER( stat ) (stat&STAT::BORDER)

class CellProperty;


// DISABLE:表示はされる、マウス等はうけつけない 
// DEATH:表示されない、すべてのメッセージは無視される
// DEAD:死体、参照カウンタの影響でメモリ上では生きでも、Controlとしては死体を意味する
enum STAT{ VISIBLE=0x1,MOUSEMOVE=0x2,CLICK=0x4,CAPTURED=0x8,SELECTED=0x10, DISABLE=0x20,READONLY=0x40, DROPACCEPTOR=0x80, DROPPING=0x100,BORDER=0x200, ALWAYSDRAW=0x400, DEAD=0x800, FOCUS=0x1000, DEBUG1=0x2000 };

class D2DWindow;
class D2DTextbox;

// Object mode
enum MOUSE_MODE { NONE,MOVE,RESIZE,COLUM_RESIZE,ROW_RESIZE,CHILD_CTRL };

class D2DWindow
{
public:
	std::shared_ptr<CellProperty> m_spProp;
	std::function<std::wstring()> m_getter;
	std::function<void(const std::wstring&)> m_setter;
	std::function<void(const std::wstring&)> m_changed;
	std::function<void()> m_final;
	std::wstring m_strInit;
	std::shared_ptr<d2dw::CDirect2DWrite> m_pDirect;
	std::unique_ptr<D2DTextbox> m_pTxtbox;

	HWND m_hWnd;
	int redraw_;

	std::shared_ptr<d2dw::CDirect2DWrite>& GetDirectPtr() { return  m_pDirect; }

	public :
		D2DWindow(
			std::shared_ptr<CellProperty> spProp,
			std::function<std::wstring()> getter,
			std::function<void(const std::wstring&)> setter,
			std::function<void(const std::wstring&)> changed,
			std::function<void()> final);
		virtual ~D2DWindow();

		LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnEraseBkGnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);



		HWND CreateD2DWindow( DWORD WSEX_STYLE, HWND parent, DWORD WS_STYLE, RECT rc, UINT* img_resource_id=nullptr, int img_cnt=0 );		
		LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);		
};
