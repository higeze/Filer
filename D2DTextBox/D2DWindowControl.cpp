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

void D2DControl::CreateWindow(D2DWindow* parent, const FRectFBoxModel& rc, int stat, LPCWSTR name, int controlid)
{
	InnerCreateWindow( parent,rc,stat,name,controlid);

	OnCreate();
}
void D2DControl::InnerCreateWindow(D2DWindow* parent, const FRectFBoxModel& rc, int stat, LPCWSTR name, int controlid)
{
	_ASSERT(parent);

	parent_ = parent;
	rc_ = rc;
	stat_ = stat;
	name_ = name;
	id_ = controlid;
	target_ = nullptr; // このオブジェクトと関連付けるポインタ、通常はnull
	guid_ = FString::NewGuid();

	//if ( stat_ & DROPACCEPTOR )
	//{
	//	parent->drag_accepters_.push_back( this );

	//	stat_ &= ~DROPACCEPTOR;
	//}

	CHDL id = parent_->chandle_.CreateControlHandle( this );
	chdl_ = id;
}

void D2DControl::DestroyControl()
{
	if ( !(stat_ & STAT::DEAD ) )
	{		
		//TRACE( L"me=%x parent=%x HWND=%x %d\n", this, parent_, parent_->hWnd_, (IsWindow(parent_->hWnd_)?1:0) );
		SendMessage( WM_D2D_DESTROY_CONTROL, 0, (LPARAM)this);
		
	
		stat_ &= ~STAT::VISIBLE;
		stat_ |= STAT::DEAD;

		parent_->chandle_.DeleteControlHandle( chdl_ );
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



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void D2DStatic::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
//{
//	D2DControl::CreateWindow( parent,rc,stat,name, id );
// 
//	brush_fore_ = D2RGB(0,0,0);
//	brush_back_ = D2RGB(255,255,255 );
//	
//
//	SetText( name );
//
//}
//
//void D2DStatic::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, LPCWSTR name )
//{
//	CreateWindow( parent, pacontrol, rc, STAT::VISIBLE, name, -1 );
//}
//void D2DStatic::SetText( LPCWSTR str, int align )
//{
//	name_ = str;
//
//	if ( align > -1 )
//		alignment_ = align;
//
//	SetFont( parent_->cxt_.textformat );
//}
//void D2DStatic::SetFont( IDWriteTextFormat* fmt )
//{
//	FRectF rc = rc_.GetContentRect(); //Zero(); 
//	rc = rc.ZeroRect();
//	//rc.left += 5;
//	xassert( alignment_ == 0 ||alignment_ == 1 ||alignment_ == 2 );
//	//stext_.CreateLayoutEx( parent_->cxt_.cxtt,fmt, rc, name_, lstrlen(name_), alignment_ ); // 単行のみ
//}
//
//LRESULT D2DStatic::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
//{
//	if ( !IsThroughMessage(message))
//		return 0;
//
//	LRESULT ret = 0;
//	switch( message )
//	{
//		case WM_PAINT:
//		{
//			auto cxt = d->cxt_;
//			FRectF rccnt = rc_.GetContentRect();
//
//			auto br = MakeBrsuh(cxt,brush_fore_);
//		}
//		break;
//		case WM_D2D_JS_SETTEXT:
//			if ( id_ == wParam )
//			{
//				VARIANT* pv = (VARIANT*)lParam;
//				if ( pv->vt == VT_BSTR )
//				{
//					SetText(pv->bstrVal);				
//					ret = 1;
//				}
//			}
//		break;
//	}
//
//
//	return ret;
//}

//#pragma region  D2DScrollbar
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//D2DScrollbar::D2DScrollbar()
//{
//	info_.button_height = 16;
//	info_.stat = D2DScrollbarInfo::STAT::NONE;
//	info_.position = 0;
//	info_.total_height = 0;
//	info_.bVertical = true;
//	info_.auto_resize = true;
//	target_control_ = NULL;
//
//}
//void D2DScrollbar::CreateWindowEx(D2DWindow* parent, D2DControl* target, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id)
//{
//	parent_ = parent;
//	target_control_ = target;
//	rc_ = rc;
//	stat_ = stat;
//	name_ = name;
//	id_ = id;
//
//	OnCreate();
//
//}
//void D2DScrollbar::Clear()
//{
//	info_.position = 0;
//}
//void D2DScrollbar::OnCreate()
//{
//	_ASSERT(0 <= rc_.Width());
//	_ASSERT(0 <= rc_.Height());
//
//	info_.bVertical = (rc_.Width() <= rc_.Height());
//	info_.total_height = (info_.bVertical ? rc_.Height() : rc_.Width());
//
//}
//void D2DScrollbar::Show(bool visible)
//{
//	stat_ = (visible ? stat_ | VISIBLE : stat_ & ~VISIBLE);
//}
//LRESULT D2DScrollbar::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
//{
//	LRESULT ret = 0;
//
//
//	if ((stat_ & VISIBLE) == false)
//		return ret;
//
//	bool bVisible = true;
//
//	if (info_.bVertical)
//		bVisible = ((info_.total_height > rc_.Height()) || (stat_ & ALWAYSDRAW));
//	else
//		bVisible = ((info_.total_height > rc_.Width()) || (stat_ & ALWAYSDRAW));
//
//
//	if (!bVisible) return ret;
//
//
//	switch (message) {
//	case WM_PAINT:
//	{
//		//auto cxt = d->cxt_;
//		//info_.rc = rc_.GetContentRect();
//
//		//cxt.m_pDirect->GetHwndRenderTarget()->GetTransform(&mat_);
//		//DrawScrollbar(cxt.m_pDirect->GetHwndRenderTarget(), info_);
//	}
//	break;
//
//	case WM_LBUTTONDOWN:
//	{
//		auto cxt = d->cxt_;
//
//		FPointF pt2;// = mat_.DPtoLP(FPointF(lParam)); // 親座標で変換
//		info_.ptprv = pt2;
//		int stat = D2DScrollbarInfo::STAT::NONE;
//		if (rc_.PtInRect(pt2)) {
//			//d->SetCapture(this, &pt2);
//
//			FRectF rc1 = ScrollbarRect(info_, 1);
//			FRectF rc2 = ScrollbarRect(info_, 2);
//			FRectF rc = ScrollbarRect(info_, 3);
//			if (rc.PtInRect(pt2))
//				stat = D2DScrollbarInfo::STAT::ONTHUMB;
//			else if (rc1.PtInRect(pt2))
//				stat = D2DScrollbarInfo::STAT::ONBTN1;
//			else if (rc2.PtInRect(pt2))
//				stat = D2DScrollbarInfo::STAT::ONBTN2;
//			else {
//				float thumb_height_c = info_.thumb_rc.bottom - info_.thumb_rc.top;
//
//				FRectF rcc1(rc1.left, rc1.bottom, rc1.right, rc1.bottom + info_.position);
//				FRectF rcc2(rc1.left, rc1.bottom + info_.position + thumb_height_c, rc1.right, rc2.top);
//				if (rcc1.PtInRect(pt2))
//					stat = D2DScrollbarInfo::STAT::ONSPC1;
//				else if (rcc2.PtInRect(pt2))
//					stat = D2DScrollbarInfo::STAT::ONSPC2;
//			}
//			stat |= D2DScrollbarInfo::STAT::CAPTURED;
//
//			ret = 1;
//		}
//
//		if (info_.stat != stat) {
//			info_.stat = stat;
//		}
//
//	}
//	break;
//	case WM_MOUSEMOVE:
//	{
//		FPointF pt2;// = mat_.DPtoLP(FPointF(lParam));
//
//		int stat = D2DScrollbarInfo::STAT::NONE;
//		if (rc_.PtInRect(pt2) /*|| d->GetCapture() == this*/) {
//			stat = D2DScrollbarInfo::STAT::ONTHUMB;
//
//			FRectF rc1 = ScrollbarRect(info_, 1);
//			FRectF rc2 = ScrollbarRect(info_, 2);
//			FRectF rc = ScrollbarRect(info_, 3);
//			//if ( rc.PtInRect(pt2 ) ) 
//			//	stat = D2DScrollbarInfo::STAT::ONTHUMB;
//			if (rc1.PtInRect(pt2))
//				stat = D2DScrollbarInfo::STAT::ONBTN1;
//			else if (rc2.PtInRect(pt2))
//				stat = D2DScrollbarInfo::STAT::ONBTN2;
//		}
//
//		float off = 0;
//		// キャプチャーしていたらスクロールは有効とみなす
//		if (/*d->GetCapture() == this &&*/ stat & D2DScrollbarInfo::STAT::ONTHUMB) {
//			FPointF prev = info_.ptprv;// d->CapturePoint( pt2 );
//
//			if (info_.bVertical) {
//				off = pt2.y - prev.y;
//				if (off) {
//					float move_area = rc_.GetContentRect().Height() - info_.button_height * 2;
//					float thumb_height_c = info_.thumb_rc.bottom - info_.thumb_rc.top;
//
//					if (info_.position + off + thumb_height_c <= move_area) {
//						info_.position = max(0, info_.position + off);
//					}
//				}
//			} else {
//				off = pt2.x - prev.x;
//				if (off) {
//					float move_area = rc_.Width() - info_.button_height * 2;
//					float thumb_height_c = info_.thumb_rc.right - info_.thumb_rc.left;
//
//					if (info_.position + off + thumb_height_c < move_area)
//						info_.position = max(0, info_.position + off);
//				}
//			}
//
//			if (off) {
//				if (target_control_)
//					target_control_->UpdateScrollbar(this);
//
//				d->redraw_ = 1;
//			}
//
//			stat |= D2DScrollbarInfo::STAT::CAPTURED;
//			ret = 1;
//		} else {
//			stat &= ~D2DScrollbarInfo::STAT::CAPTURED;
//		}
//
//		if (info_.stat != stat) {
//			info_.stat = stat;
//			d->redraw_ = 1;
//		}
//		info_.ptprv = pt2;
//	}
//	break;
//	case WM_CAPTURECHANGED:
//	case WM_LBUTTONUP:
//	{
//		//if (d->GetCapture() == this) {
//		//	d->ReleaseCapture();
//			info_.stat &= ~D2DScrollbarInfo::STAT::CAPTURED;
//
//			float off = 0;
//			if (info_.stat == D2DScrollbarInfo::STAT::ONBTN1)
//				off = -OffsetOnBtn(1);
//			else if (info_.stat == D2DScrollbarInfo::STAT::ONBTN2)
//				off = OffsetOnBtn(1);
//			else if (info_.stat == D2DScrollbarInfo::STAT::ONSPC1)
//				off = -OffsetOnBtn(2);
//			else if (info_.stat == D2DScrollbarInfo::STAT::ONSPC2)
//				off = OffsetOnBtn(2);
//
//			if (off) {
//				float move_area = 0, thumb_height_c = 0;
//				if (info_.bVertical) {
//					move_area = rc_.GetContentRect().Height() - info_.button_height * 2;
//					thumb_height_c = info_.thumb_rc.bottom - info_.thumb_rc.top;
//				} else {
//					move_area = rc_.Width() - info_.button_height * 2;
//					thumb_height_c = info_.thumb_rc.right - info_.thumb_rc.left;
//				}
//
//				info_.position = min(move_area - thumb_height_c, max(0, info_.position + off));
//				if (target_control_)
//					target_control_->UpdateScrollbar(this);
//			}
//			ret = 1;
//		//} else {
//		//	info_.stat &= ~D2DScrollbarInfo::STAT::CAPTURED;
//		//	info_.stat = D2DScrollbarInfo::STAT::NONE;
//		//}
//
//
//	}
//	break;
//	case WM_SIZE:
//	{
//		if (info_.auto_resize) {
//
//			FRectF rc = target_control_->GetRect().GetBorderRect().ZeroRect();  //GetContentRect();
//			FSizeF sz = rc_.Size();
//
//			if (info_.bVertical) {
//				rc_.SetPoint(rc.right - sz.width, rc.top);
//				rc_.SetSize(sz.width, rc.bottom - rc.top);
//			} else {
//				rc_.SetPoint(rc.left, rc.bottom - sz.height);
//				rc_.SetSize(rc.Width(), sz.height);
//			}
//		}
//	}
//	break;
//	}
//	return ret;
//}
//
//float D2DScrollbar::OffsetOnBtn(int typ)
//{
//	if (typ == 1)
//		return  10;
//	else if (typ == 2)
//		return 20;
//
//	return 0;
//}
//void D2DScrollbar::SetTotalSize(float cy)
//{
//	xassert(0 <= cy);
//	info_.total_height = cy;
//}
//
//#pragma endregion
