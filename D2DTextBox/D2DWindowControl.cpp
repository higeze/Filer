#include "text_stdafx.h"
#include "D2DWindow.h" 
#include "D2DWindowControl.h"
#include "gdi32.h"
#include "MoveTarget.h"

#define CLASSNAME L"D2DWindow"
using namespace V4;

#pragma comment(lib,"dwrite.lib") 
#pragma comment(lib,"d2d1.lib")
#pragma comment(lib,"winmm.lib")

#define TITLE_BAR_HEIGHT 24

#pragma region D2DControl
D2DControl::D2DControl()
{
	// You must create this on Heap, OnStack is NG.
	_ASSERT( _CrtIsValidHeapPointer(this));
}
D2DControl::~D2DControl()
{

}
int D2DControl::Stat( int new_stat)
{
	if ( new_stat < 0 )
		return stat_;
	
	stat_ = new_stat;
	parent_->redraw_ = 1;

	return stat_;
}
void D2DControl::Enable(bool bEnable)
{
	stat_ = (bEnable ? (stat_ & ~STAT::DISABLE) : (stat_ | STAT::DISABLE) );
}
void D2DControl::Visible(bool bVisible)
{
	stat_ = (bVisible ? (stat_ | STAT::VISIBLE) : (stat_ & ~STAT::VISIBLE) );
}
bool D2DControl::IsEnable() const
{
	return !(stat_ & STAT::DISABLE);
}
bool D2DControl::IsVisible()   const
{
	return (stat_ & STAT::VISIBLE);
}
void D2DControl::SetRect( const FRectF& rc )
{ 
	rc_.SetFRectF(rc); 
}
void D2DControl::SetRect( const FPointF& pt, const FSizeF& sz )
{ 
	rc_.SetFRectF( FRectF(pt,sz)); 
}

void D2DControl::StatActive(bool bActive)
{
	if ( bActive )
		stat_ |= FOCUS;
	else
		stat_ &= ~FOCUS;
}

void D2DControl::CreateWindow(D2DWindow* parent, const FRectFBoxModel& rc, int stat, LPCWSTR name)
{
	InnerCreateWindow( parent,rc,stat,name);

	OnCreate();
}
void D2DControl::InnerCreateWindow(D2DWindow* parent, const FRectFBoxModel& rc, int stat, LPCWSTR name)
{
	_ASSERT(parent);

	parent_ = parent;
	rc_ = rc;
	stat_ = stat;
	name_ = name;
	target_ = nullptr; // このオブジェクトと関連付けるポインタ、通常はnull
	guid_ = FString::NewGuid();
}

void D2DControl::DestroyControl()
{
	if ( !(stat_ & STAT::DEAD ) )
	{		
		//TRACE( L"me=%x parent=%x HWND=%x %d\n", this, parent_, parent_->hWnd_, (IsWindow(parent_->hWnd_)?1:0) );
		SendMessage( WM_D2D_DESTROY_CONTROL, 0, (LPARAM)this);
		
	
		stat_ &= ~STAT::VISIBLE;
		stat_ |= STAT::DEAD;
	}
}
bool D2DControl::IsThroughMessage( UINT message )
{
	bool bUi = (WM_MOUSEFIRST <= message && message <= WM_MOUSELAST) || (WM_KEYFIRST <= message && message <= WM_KEYLAST);
	
	
	if ( VISIBLE(stat_))
	{
		if (!ENABLE(stat_) && bUi)
			return false;
	
		return true;
	}

	// 非表示なのでUIメッセージを無視する
	if ( WM_PAINT == message || WM_D2D_PAINT==message || bUi )
		return false;

	// 有効なメッセージ
	return true;
}

