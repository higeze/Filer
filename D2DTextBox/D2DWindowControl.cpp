#include "text_stdafx.h"
#include "D2DWindow.h" 
#include "D2DWindowControl.h"
#include "gdi32.h"
#include "D2DContextNew.h"
#include "MoveTarget.h"

#define CLASSNAME L"D2DWindow"
using namespace V4;

#pragma comment(lib,"dwrite.lib") 
#pragma comment(lib,"d2d1.lib")
#pragma comment(lib,"winmm.lib")

#define TITLE_BAR_HEIGHT 24


		
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

void D2DControl::CreateWindow(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int controlid)
{
	InnerCreateWindow( parent, pacontrol,rc,stat,name,controlid);

	OnCreate();
}
void D2DControl::InnerCreateWindow(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int controlid)
{
	_ASSERT(parent);

	parent_ = parent;
	rc_ = rc;
	stat_ = stat;
	parent_control_ = pacontrol;
	name_ = name;
	id_ = controlid;
	target_ = nullptr; // このオブジェクトと関連付けるポインタ、通常はnull
	guid_ = FString::NewGuid();

	if (parent_control_)
	{
		parent_control_->controls_.push_back(std::shared_ptr<D2DControl>(this));
	}

	if ( stat_ & DROPACCEPTOR )
	{
		parent->drag_accepters_.push_back( this );

		stat_ &= ~DROPACCEPTOR;
	}

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

		//if ( parent_->GetCapture() == this )
		//	parent_->ReleaseCapture(); // 1階層目だけはcheck


		if ( parent_control_ )
		{
			auto p = parent_control_->Detach( this );

			parent_->death_objects_.push_back(p);
		}

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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D2DControls::D2DControls()
{ 
	mouse_enter_ = false;
	scrollbar_off_ = FSizeF(0,0);
	clr_ = D2RGB(255,255,255);
}
void D2DControls::CreateWindow( D2DWindow* parent, D2DControls* paconrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	xassert( parent );
	xassert( paconrol != this );

	parent_ = parent;
	rc_ = rc;
	stat_ = stat;
	parent_control_ = paconrol;
	name_ = name;
	id_ = id;
	auto_resize_ = false;
	guid_ = FString::NewGuid();

	bwindow_size_ = (rc.Width() <= 0);
	if ( bwindow_size_ )
	{
		_ASSERT( rc.left == 0 && rc.top == 0 );
	}

	if ( parent_control_ )
		parent_control_->controls_.push_back( std::shared_ptr<D2DControl>( this ));
	else if ( parent_  )
	{
		//_ASSERT( parent_->children_  == NULL );
		//parent_->children_ = std::shared_ptr<D2DControls>( this );		
	}


	if ( stat_ & DROPACCEPTOR && parent_ )
	{
		parent->drag_accepters_.push_back( this );
		stat_ &= ~DROPACCEPTOR;
	}

	chdl_ = parent_->chandle_.CreateControlHandle( this );
		
	OnCreate();
}
void D2DControls::Clear()
{
	controls_.clear();
	
}
void D2DControls::UpdateScrollbar(D2DScrollbar* bar)
{
	if ( bar->info_.bVertical )	
		scrollbar_off_.height = bar->info_.position / bar->info_.thumb_step_c;
	else
		scrollbar_off_.width = bar->info_.position;
}


D2DControl* D2DControls::GetControl( int id )
{
	for( auto& it : controls_ )
	{
		if ( it->GetID() == id )
			return it.get();
	}
	return nullptr;
}

LRESULT D2DControls::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if ( !IsThroughMessage(message))
		return 0;

	switch( message )
	{
		case WM_DESTROY:
			SendMessageAll(d,message,wParam,lParam);
			return 0;
		break;
		case WM_D2D_RESTRUCT_RENDERTARGET:
			//CreateResource(wParam == 1 );				
			SendMessageAll(d, message, wParam, lParam);
			return 0;
		break;
	}


	
	
//	D2DContext& cxt = d->cxt_;
//	D2DMatrix mat(cxt);		
//	mat.PushTransform();
//	if ( message == WM_PAINT )
//	{		
//		mat.Offset( -scrollbar_off_.width, -scrollbar_off_.height );
//mat.Offset( rc_.left, rc_.top );
//		mat_ = mat; // 自座標(左上0,0)のmatrix
//	}
//	else
//		mat = mat_;	// mat_と同じ値にする。
	
	

	switch( message )
	{
		case WM_PAINT:
		{						
			D2DContext& cxt = d->cxt_;
			D2DMatrix mat(cxt);		
			mat_ = mat.PushTransform();
			mat.Offset( -scrollbar_off_.width, -scrollbar_off_.height );
			mat.Offset( rc_.left, rc_.top );

			if ( (stat_ & BORDER) )
			{				
				FRectF rc1 = rc_.GetContentRectZero(); //.ZeroRect();
				
				CComPtr<ID2D1SolidColorBrush> backbr = MakeBrsuh(cxt, clr_ );		
				DrawFillRect( cxt, rc1, d->cxt_.black, backbr, 1.0f );
				

				rc1.InflateRect(-1,-1);
				D2DRectFilter fil( cxt, rc1 );

				SendMessageReverseAll(d,message,wParam,lParam);					
			}
			else
				SendMessageReverseAll(d,message,wParam,lParam);	


			mat.PopTransform();
		}
		break;		
		
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MOUSEWHEEL:

			ret = SendMessageAll(d, message, wParam, lParam);
		break;
		case WM_MOUSEMOVE:
		{
			if (ENABLE(stat_))
			{
				FPointF pt3 = mat_.DPtoLP(lParam);

				if ( rc_.PtInRect( pt3 ))
				{
					if ( !mouse_enter_ )
					{
						wParam = wParam | MK_F_MOUSE_ENTER;
						mouse_enter_ = true;
					}
					ret = SendMessageAll(d, message, wParam, lParam);
				
				}
				else if ( mouse_enter_ )
				{
					wParam = wParam | MK_F_MOUSE_LEAVE;
					ret = SendMessageAll(d, message, wParam, lParam);
					mouse_enter_ = false;
				}
			}
		}
		break;
		case WM_SIZE:
		{			
			if ( bwindow_size_ ) 
			{
				FSizeF sz(lParam);
				sz = mat_.DPtoLP(sz);
				rc_.bottom = rc_.top + sz.height;
				rc_.right = rc_.left + sz.width;
			}

			if ( controls_.size() == 1 )
			{
				auto it = controls_[0];
				if ( it->auto_resize_ )
				{
					auto rc = rc_.ZeroRect();
					it->SetRect( rc );
				}
			}
			else
			{
				




			}

			//if (auto_resize_)
			//{
			//	// top,leftがある場合は、parentのサイズにあわせる
			//	auto sz = parent_control_->GetContentRect().Size();
			//	rc_.bottom = sz.height;// - rc_.top;
			//	rc_.right = sz.width;// - rc_.left;
			//}


			SendMessageAll(d,message,wParam,lParam);		
			ret = 0;
		}
		break;
		case WM_D2D_GET_CONTROLS:
		{
			if ( id_ == wParam )
			{
				D2DControls** p = (D2DControls**)lParam;

				*p = this;

				ret = 1;
			}
			else
				ret=SendMessageAll(d,message,wParam,lParam);
		}
		break;
		
		case WM_D2D_OBJECT_SUICIDE:
		{
			D2DControl* p = (D2DControl*)lParam;

			if (p->parent_control_ ==  this )
			{
				auto it = controls_.begin();
				
				for( ; it != controls_.end(); it++ )
				{
					D2DControl* d = (*it).get();
					if ( d == p )
						break;
				}

				if ( it!=controls_.end())
				{
					controls_.erase( it ); // PostMessageされたWM_D2D_OBJECT_SUICIDEにより、targetを削除する					
				}
				ret = 1;
			}
			else if (ENABLE(stat_))
				ret = SendMessageAll(d,message,wParam,lParam);	
		}
		break;
		default:
			if (ENABLE(stat_))
				ret = SendMessageAll(d,message,wParam,lParam);		
	}

	//mat.PopTransform();
	return ret;
}
LRESULT D2DControls::SendMessageReverseAll( D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT r = 0;
	for( auto it = controls_.begin(); it!=controls_.end(); ++it )// 順番：表示奥のオブジェクトから前面のオブジェクトへ
	{
		auto obj = (*it).get();
		xassert( obj != this );

		r += obj->WndProc(parent,message,wParam,lParam);

		if (r != 0 && r != LRESULT_SEND_MESSAGE_TO_OTHER_OBJECTS)
			break;

	}
	return r;


}
LRESULT D2DControls::SendMessageAll( D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT r = 0;

	for( auto it = controls_.rbegin(); it!=controls_.rend(); ++it ) // 順番：表示前面のオブジェクトから奥へ
	{
		auto obj = (*it).get();
		xassert( obj != this );
		
		int size = controls_.size();


		r += obj->WndProc(parent,message,wParam,lParam);

		if (r != 0 && r != LRESULT_SEND_MESSAGE_TO_OTHER_OBJECTS)
			break;

		xassert( size == controls_.size() ); // ここで落ちる場合は、r=1を返せ。またはループが間違っている。
	}
	return r;
}


void D2DControls::Attach( std::shared_ptr<D2DControl> ctrl )
{
	_ASSERT( ctrl->parent_control_ == NULL );

	controls_.push_back( ctrl );
	ctrl->parent_control_ = this;

}


std::shared_ptr<D2DControl> D2DControls::Detach(D2DControl* target)
{
	for( auto it = controls_.begin(); it != controls_.end(); ++it )
	{
		if ( (*it).get() == target )
		{
			std::shared_ptr<D2DControl> ret = (*it);

			controls_.erase( it );

			ret->parent_control_ = NULL;

			return ret;
		}
	}

	_ASSERT( false );

	return NULL;
}
bool D2DControls::IsLast( D2DControl* obj ) const
{
	return ( controls_.rbegin()->get() == obj);

}
void D2DControls::MeToLast()
{
	std::vector<std::shared_ptr<D2DControl>>& ls = this->parent_control_->controls_;
	std::shared_ptr<D2DControl> me;
	

	for(auto it=ls.begin(); it != ls.end(); it++ )
	{
		if ( this == (*it).get() )
		{
			me = (*it);
			ls.erase( it );	 // remove
			break;
		}		
	}
	
	ls.push_back( me ); // add to last.


	//vector_idx.second = ls.size()-1;
}
LRESULT D2DControls::KeyProc(D2DControl* sender, UINT message, WPARAM wParam, LPARAM lParam)
{ 
	LRESULT ret = 0;
	switch( message )
	{
		// TABキーでコントロール間を移動
		
		case WM_KEYDOWN:
		{
			if ( wParam == VK_TAB )
			{
				auto& ls = controls_; //  created object order.

				bool bl = false;
				for( int i = 0; i < 2; i++ ) // first->last->first and break.
				{
					
					int j = 0;
					
					for( auto& it : ls )
					{
						auto nm = it->GetName();
						//TRACE( L"%x name=%s \n", this, (LPCWSTR)nm.c_str() );
						
						if ( it.get() == sender )
						{
							
							//if ( parent_->GetCapture() == it.get())
							//{
							//	parent_->ReleaseCapture();							
							//}

							

							TRACE( L"%x UNACTIVE %s ", this, (LPCWSTR)nm );
							
							it->StatActive(false);
							bl = true;
						}
						else if ( bl )
						{
							// messageをitへ集中させる

				
							if ( !dynamic_cast<D2DStatic*>( it.get()) )
							{
								//parent_->SetCapture(it.get());	
								
								

								TRACE( L"ACTIVE %s \n", (LPCWSTR)nm );

								if ( nm == L"btn" )
								{
										int a = 0;
								}

																			
								it->StatActive(true);
								ret = 1;
								return ret;						
							}
						}

						j++;
					}
				}
			}
		}
		break;
	}

	return ret;

	return 0; 
}

void D2DControls::DestroyControl()
{
	if ( !(stat_ & STAT::DEAD ) )
	{		
		std::list<D2DControl*> ls;
	
		for(auto& it : controls_ )
			ls.push_back(it.get());
		
		for(auto& it : ls )
		{
			int st = it->GetStat();

			if ( !(st & STAT::DEAD ) )
				it->DestroyControl();
		}


		
		D2DControl::DestroyControl();
	}
}
#pragma endregion // D2DControl

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void D2DStatic::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	D2DControl::CreateWindow( parent,pacontrol,rc,stat,name, id );
 
	brush_fore_ = D2RGB(0,0,0);
	brush_back_ = D2RGB(255,255,255 );
	

	SetText( name );

}

void D2DStatic::CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, LPCWSTR name )
{
	CreateWindow( parent, pacontrol, rc, STAT::VISIBLE, name, -1 );
}
void D2DStatic::SetText( LPCWSTR str, int align )
{
	name_ = str;

	if ( align > -1 )
		alignment_ = align;

	SetFont( parent_->cxt_.cxtt.textformat );
}
void D2DStatic::SetFont( IDWriteTextFormat* fmt )
{
	FRectF rc = rc_.GetContentRect(); //Zero(); 
	rc = rc.ZeroRect();
	rc.left += 5;
	xassert( alignment_ == 0 ||alignment_ == 1 ||alignment_ == 2 );
	stext_.CreateLayoutEx( parent_->cxt_.cxtt,fmt, rc, name_, lstrlen(name_), alignment_ ); // 単行のみ
}

LRESULT D2DStatic::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	if ( !IsThroughMessage(message))
		return 0;

	LRESULT ret = 0;
	switch( message )
	{
		case WM_PAINT:
		{
			auto cxt = d->cxt_;
			D2DMatrix mat( cxt );				
			mat.PushTransform();
			mat_ = mat;

			//FRectF rc1 = rc_.GetBorderRect(); //.ZeroRect();
			//D2DRectFilter f(d->cxt_, rc1);

			//if ( stat_ & BORDER )
			//{				
			//	DrawFillRect( d->cxt_, rc1, d->cxt_.black, d->cxt_.transparent,   1.0f );
			//}

			FRectF rccnt = rc_.GetContentRect();
			mat.Offset( rccnt.left, rccnt.top );	

			auto br = MakeBrsuh(cxt,brush_fore_);
			stext_.DrawText( cxt, br );

			mat.PopTransform();
			
		}
		break;
		case WM_D2D_JS_SETTEXT:
			if ( id_ == wParam )
			{
				VARIANT* pv = (VARIANT*)lParam;
				if ( pv->vt == VT_BSTR )
				{
					SetText(pv->bstrVal);				
					ret = 1;
				}
			}
		break;
	}


	return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void D2DTopControls::CreateWindow(D2DWindow* d, int stat, LPCWSTR name, int id )
{
	CreateWindow(d,NULL,FRectFBoxModel(0,0,-1,-1),stat,name,id );

}
void D2DTopControls::CreateWindow(D2DWindow* d, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id )
{
	xassert( pacontrol == NULL );
	D2DControls::CreateWindow(d, pacontrol, rc, stat, name, id);
	
	zoom_ =1.0f;
	bAutoSize_ = true;

	back_.br = nullptr;
	back_.color = D2RGB(255,255,255 );
	guid_ = FString::NewGuid();
	
	CreateResource(true);
	
}
FRectF D2DTopControls::CalcAutoSize( const GDI32::FSize& sz )
{
	FRectF rc = rc_;

	if ( calc_auto_size_ )
		rc = calc_auto_size_( rc, sz );
	else
	{
		rc.SetSize(sz);
	}
	return rc;
}
void D2DTopControls::OnCreate()
{
	WndProc(parent_,WM_D2D_APP_ON_CREATE, 0,0);		
}

void D2DTopControls::BackColor(D2D1_COLOR_F clr)
{
	back_.color = clr;
	back_.br = MakeBrsuh(parent_->cxt_, back_.color);

}

LRESULT D2DTopControls::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	if ( !IsThroughMessage(message))
		return 0;

	switch( message )
	{
		case WM_D2D_RESTRUCT_RENDERTARGET:
			CreateResource( wParam == 1);
			SendMessageAll(d, message, wParam, lParam);
			return 0;
		break;
		case WM_DESTROY:			
			SendMessageAll(d, message, wParam, lParam);
			Clear();
			return 0;
		break;						
	}
	


	D2DMatrix mat(d->cxt_);
	mat.PushTransform();
	mat.Offset( -scrollbar_off_.width, -scrollbar_off_.height );
	mat.Offset( rc_.left, rc_.top );

	mat._11 = zoom_ ;
	mat._22 = zoom_ ;
	
	mat.SetTransform();

	mat_ = mat; // 自座標のマット

	FPointF pt21 = mat.DPtoLP( FPointF(lParam));

	switch( message )
	{
		case WM_PAINT:
		{			
			D2DContext& cxt = d->cxt_;

			auto rcx = rc_.ZeroRect();
			
			D2DRectFilter f(cxt, rcx );

			
			DrawFill(cxt,rcx, (ID2D1Brush*)back_.br);

			SendMessageReverseAll(d,message,wParam,lParam);	

		}
		break;	
		case WM_D2D_JS_ERROR:
		{
			error_msg_ = (BSTR)lParam;

			ret = 1;
			

		}
		break;
		
		case WM_LBUTTONDOWN:		
		{
			if (ENABLE(stat_))
			{
				if ( rc_.ZeroPtInRect(pt21)) //if ( rc_.ZeroRect().PtInRect( pt21 ))
					SendMessageAll(d,message,wParam,lParam);
				else
					SendMessageAll(d,WM_D2D_NCLBUTTONDOWN,wParam,lParam);
			}
		}
		break;
		case WM_LBUTTONDBLCLK:
		{
			if (ENABLE(stat_))
			{
				if ( rc_.ZeroPtInRect(pt21)) //if ( rc_.ZeroRect().PtInRect( pt21 ))
					SendMessageAll(d,message,wParam,lParam);
			}
		}
		break;
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:		
		case WM_LBUTTONUP:
		{
			if (ENABLE(stat_))
			{
				if ( rc_.ZeroPtInRect(pt21)) //if ( rc_.ZeroRect().PtInRect( pt21 ))
					SendMessageAll(d,message,wParam,lParam);
			}
		}
		break;
		case WM_MOUSEMOVE:
		{
			if (ENABLE(stat_))
			{
				//if ( rc_.ZeroRect().PtInRect( pt21 ))

				if ( rc_.ZeroPtInRect(pt21))
				{
					if ( !mouse_enter_ )
					{
						wParam = wParam | MK_F_MOUSE_ENTER;
						mouse_enter_ = true;
					}
					SendMessageAll(d,message,wParam,lParam);
				
				}
				else if ( mouse_enter_ )
				{
					wParam = wParam | MK_F_MOUSE_LEAVE;
					SendMessageAll(d,message,wParam,lParam);				
					mouse_enter_ = false;
				}
			}
		}
		break;
		case WM_SIZE:
		{
			if ( bAutoSize_ )
			{
				rc_ = CalcAutoSize(GDI32::FSize(lParam));
			}
			
			SendMessageAll(d,message,wParam,lParam);		
		}
		break;
		case WM_DESTROY:
			SendMessageAll(d,message,wParam,lParam);		
		break;

	
		case WM_D2D_GET_CONTROLS:
		{
			if ( id_ == wParam )
			{
				D2DControls** p = (D2DControls**)lParam;
				*p = this;
				ret = 1;
			}
			else if (ENABLE(stat_))
				SendMessageAll(d,message,wParam,lParam);
		}
		break;
		case WM_D2D_TEST:
//			V4::MessageBox(this,L"hoi",L"hoi", MB_OK ); 
			ret = 1;
		break;
		case WM_D2D_RESTRUCT_RENDERTARGET:
		{
			CreateResource(wParam == 1);

			SendMessageAll(d, message, wParam, lParam);
		}
		break;



		default:
			if (ENABLE(stat_))
				ret = SendMessageAll(d,message,wParam,lParam);	

			if ( message <= WM_D2D_APP_USER )
			{
				if ( OnWndProcExt_ )
					ret = OnWndProcExt_(this,message,wParam,lParam);
			}

				
	}

	mat.PopTransform();
	return ret;
}
void D2DTopControls::CreateResource(bool bCreate)
{
	back_.br.Release();

	if ( bCreate )
	{
		back_.br = parent_->GetSolidColor(back_.color);

//		back_.br = parent_->m_pDirect->GetColorBrush(d2dw::CColorF(0.f, 0.f, 0.f, 1.f));
	}		
}


#pragma region  D2DScrollbar
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D2DScrollbar::D2DScrollbar()
{
	info_.button_height = 16;
	info_.stat = D2DScrollbarInfo::STAT::NONE;
	info_.position = 0;
	info_.total_height = 0;
	info_.bVertical = true;
	info_.auto_resize = true;
	target_control_ = NULL;

}
void D2DScrollbar::CreateWindowEx(D2DWindow* parent, D2DControl* target, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id)
{
	parent_ = parent;
	target_control_ = target;
	rc_ = rc;
	stat_ = stat;
	name_ = name;
	id_ = id;

	parent_control_ = nullptr;


	OnCreate();

}
void D2DScrollbar::Clear()
{
	info_.position = 0;
}
void D2DScrollbar::OnCreate()
{
	_ASSERT(0 <= rc_.Width());
	_ASSERT(0 <= rc_.Height());

	info_.bVertical = (rc_.Width() <= rc_.Height());
	info_.total_height = (info_.bVertical ? rc_.Height() : rc_.Width());

}
void D2DScrollbar::Show(bool visible)
{
	stat_ = (visible ? stat_ | VISIBLE : stat_ & ~VISIBLE);
}
LRESULT D2DScrollbar::WndProc(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;


	if ((stat_ & VISIBLE) == false)
		return ret;

	bool bVisible = true;

	if (info_.bVertical)
		bVisible = ((info_.total_height > rc_.Height()) || (stat_ & ALWAYSDRAW));
	else
		bVisible = ((info_.total_height > rc_.Width()) || (stat_ & ALWAYSDRAW));


	if (!bVisible) return ret;


	switch (message) {
	case WM_PAINT:
	{
		auto cxt = d->cxt_;
		info_.rc = rc_.GetContentRect();

		cxt.cxt->GetTransform(&mat_);
		DrawScrollbar(cxt.cxt, info_);
	}
	break;

	case WM_LBUTTONDOWN:
	{
		auto cxt = d->cxt_;

		FPointF pt2 = mat_.DPtoLP(FPointF(lParam)); // 親座標で変換
		info_.ptprv = pt2;
		int stat = D2DScrollbarInfo::STAT::NONE;
		if (rc_.PtInRect(pt2)) {
			//d->SetCapture(this, &pt2);

			FRectF rc1 = ScrollbarRect(info_, 1);
			FRectF rc2 = ScrollbarRect(info_, 2);
			FRectF rc = ScrollbarRect(info_, 3);
			if (rc.PtInRect(pt2))
				stat = D2DScrollbarInfo::STAT::ONTHUMB;
			else if (rc1.PtInRect(pt2))
				stat = D2DScrollbarInfo::STAT::ONBTN1;
			else if (rc2.PtInRect(pt2))
				stat = D2DScrollbarInfo::STAT::ONBTN2;
			else {
				float thumb_height_c = info_.thumb_rc.bottom - info_.thumb_rc.top;

				FRectF rcc1(rc1.left, rc1.bottom, rc1.right, rc1.bottom + info_.position);
				FRectF rcc2(rc1.left, rc1.bottom + info_.position + thumb_height_c, rc1.right, rc2.top);
				if (rcc1.PtInRect(pt2))
					stat = D2DScrollbarInfo::STAT::ONSPC1;
				else if (rcc2.PtInRect(pt2))
					stat = D2DScrollbarInfo::STAT::ONSPC2;
			}
			stat |= D2DScrollbarInfo::STAT::CAPTURED;

			ret = 1;
		}

		if (info_.stat != stat) {
			info_.stat = stat;
		}

	}
	break;
	case WM_MOUSEMOVE:
	{
		FPointF pt2 = mat_.DPtoLP(FPointF(lParam));

		int stat = D2DScrollbarInfo::STAT::NONE;
		if (rc_.PtInRect(pt2) /*|| d->GetCapture() == this*/) {
			stat = D2DScrollbarInfo::STAT::ONTHUMB;

			FRectF rc1 = ScrollbarRect(info_, 1);
			FRectF rc2 = ScrollbarRect(info_, 2);
			FRectF rc = ScrollbarRect(info_, 3);
			//if ( rc.PtInRect(pt2 ) ) 
			//	stat = D2DScrollbarInfo::STAT::ONTHUMB;
			if (rc1.PtInRect(pt2))
				stat = D2DScrollbarInfo::STAT::ONBTN1;
			else if (rc2.PtInRect(pt2))
				stat = D2DScrollbarInfo::STAT::ONBTN2;
		}

		float off = 0;
		// キャプチャーしていたらスクロールは有効とみなす
		if (/*d->GetCapture() == this &&*/ stat & D2DScrollbarInfo::STAT::ONTHUMB) {
			FPointF prev = info_.ptprv;// d->CapturePoint( pt2 );

			if (info_.bVertical) {
				off = pt2.y - prev.y;
				if (off) {
					float move_area = rc_.GetContentRect().Height() - info_.button_height * 2;
					float thumb_height_c = info_.thumb_rc.bottom - info_.thumb_rc.top;

					if (info_.position + off + thumb_height_c <= move_area) {
						info_.position = max(0, info_.position + off);
					}
				}
			} else {
				off = pt2.x - prev.x;
				if (off) {
					float move_area = rc_.Width() - info_.button_height * 2;
					float thumb_height_c = info_.thumb_rc.right - info_.thumb_rc.left;

					if (info_.position + off + thumb_height_c < move_area)
						info_.position = max(0, info_.position + off);
				}
			}

			if (off) {
				if (target_control_)
					target_control_->UpdateScrollbar(this);

				d->redraw_ = 1;
			}

			stat |= D2DScrollbarInfo::STAT::CAPTURED;
			ret = 1;
		} else {
			stat &= ~D2DScrollbarInfo::STAT::CAPTURED;
		}

		if (info_.stat != stat) {
			info_.stat = stat;
			d->redraw_ = 1;
		}
		info_.ptprv = pt2;
	}
	break;
	case WM_CAPTURECHANGED:
	case WM_LBUTTONUP:
	{
		//if (d->GetCapture() == this) {
		//	d->ReleaseCapture();
			info_.stat &= ~D2DScrollbarInfo::STAT::CAPTURED;

			float off = 0;
			if (info_.stat == D2DScrollbarInfo::STAT::ONBTN1)
				off = -OffsetOnBtn(1);
			else if (info_.stat == D2DScrollbarInfo::STAT::ONBTN2)
				off = OffsetOnBtn(1);
			else if (info_.stat == D2DScrollbarInfo::STAT::ONSPC1)
				off = -OffsetOnBtn(2);
			else if (info_.stat == D2DScrollbarInfo::STAT::ONSPC2)
				off = OffsetOnBtn(2);

			if (off) {
				float move_area = 0, thumb_height_c = 0;
				if (info_.bVertical) {
					move_area = rc_.GetContentRect().Height() - info_.button_height * 2;
					thumb_height_c = info_.thumb_rc.bottom - info_.thumb_rc.top;
				} else {
					move_area = rc_.Width() - info_.button_height * 2;
					thumb_height_c = info_.thumb_rc.right - info_.thumb_rc.left;
				}

				info_.position = min(move_area - thumb_height_c, max(0, info_.position + off));
				if (target_control_)
					target_control_->UpdateScrollbar(this);
			}
			ret = 1;
		//} else {
		//	info_.stat &= ~D2DScrollbarInfo::STAT::CAPTURED;
		//	info_.stat = D2DScrollbarInfo::STAT::NONE;
		//}


	}
	break;
	case WM_SIZE:
	{
		if (info_.auto_resize && parent_control_) {
			FRectF rc = parent_control_->GetContentRect();
			FSizeF sz = rc_.Size();

			if (info_.bVertical) {
				rc_.SetPoint(rc.right - sz.width, rc.top - 2);
				rc_.SetSize(sz.width, rc.bottom - rc.top);
			} else {
				rc_.SetPoint(rc.left, rc.bottom - sz.height);
				rc_.SetSize(rc.Width(), sz.height);
			}
		} else if (info_.auto_resize) {

			FRectF rc = target_control_->GetRect().GetBorderRect().ZeroRect();  //GetContentRect();
			FSizeF sz = rc_.Size();

			if (info_.bVertical) {
				rc_.SetPoint(rc.right - sz.width, rc.top);
				rc_.SetSize(sz.width, rc.bottom - rc.top);
			} else {
				rc_.SetPoint(rc.left, rc.bottom - sz.height);
				rc_.SetSize(rc.Width(), sz.height);
			}
		}
	}
	break;
	}
	return ret;
}

float D2DScrollbar::OffsetOnBtn(int typ)
{
	if (typ == 1)
		return  10;
	else if (typ == 2)
		return 20;

	return 0;
}
void D2DScrollbar::SetTotalSize(float cy)
{
	xassert(0 <= cy);
	info_.total_height = cy;
}

#pragma endregion
