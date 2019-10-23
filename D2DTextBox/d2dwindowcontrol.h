#pragma once

#include "D2DMisc2.h"
#include "D2DContextEx.h"
#include "D2Dcontextnew.h"
#include "D2DWindow.h"
#include "ItemLoopArray.h"
#include "D2DWindowMessage.h"
#include "D2DWindowMessageStruct.h"
#include "TextContainer.h"	// CTextContainer
#include "IBridgeTSFInterface.h"
#include "gdi32.h"
#include "ControlHandle.h"
//#include "msxmlex6.h"
#include "faststack.h"
#include "TextEditor.h"


#undef CreateWindow

//#define COLOR_MOUSE_MOVE D2RGBA(153,217,234,255)
//#define COLOR_SELECTED D2RGBA(132,137,227,100)


#define COLOR_DEF_FLOAT_SELECT	D2RGBA(54,101,179,100)
#define COLOR_DEF_SELECTED		D2RGBA(54,101,179,255)
#define COLOR_DEF_BORDER		D2RGB(0,0,0)
#define COLOR_DEF_FORE			D2RGBA(54,101,179,255)

namespace TSF {
class CTextEditorCtrl;
class LayoutLineInfo;
};

namespace V4 {

typedef std::function<LRESULT(D2DControl*,UINT,WPARAM,LPARAM)> OnWndProcExtDelegate;

class D2DControl;
class D2DControls;
class D2DWindow;
class D2DScrollbar;
class D2DControls;
class D2DControl : public D2DCaptureObject
{
	public :
		D2DControl();
		virtual ~D2DControl();

		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam) = 0;

		virtual void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int local_id=-1 );
		
		int Stat( int new_stat);

		int GetStat()  const{ return stat_; }
		void SetStat(int s){ stat_ = s; }

		const FRectFBoxModel& GetRect() const { return rc_; }
		virtual void SetRect( const FRectF& rc );
		virtual void SetRect( const FPointF& pt, const FSizeF& sz );

		virtual void UpdateScrollbar(D2DScrollbar* bar){};
		bool IsThroughMessage( UINT message );
		
		virtual void UpdateStatus( D2DControls* p){}
		virtual D2DControl* UnPack(){ return this; }
		virtual void StatActive(bool bActive);
		virtual bool IsAutoResize(){ return auto_resize_; }
		virtual void DestroyControl();
	

		void Enable(bool bEnable);
		void Visible(bool bVisible);
		bool IsEnable()  const;
		bool IsVisible()  const;
	

		virtual void OnHostage( D2DControls* p, bool block ){}
	
		
		CHDL GetCHDL() const { return chdl_; } // Get Control Handle
		D2DMat Matrix() const { return mat_; }
		FString GetName() const { return name_; }		
		int GetID() const { return id_; }
		
		void SetName(LPCWSTR nm) { name_ = nm; }
		void SetGuid( LPCWSTR id ){ guid_ = id; }
	public :
		D2DWindow* parent_;				
		D2DControls* parent_control_;
		void* target_;
		bool auto_resize_;

	protected :
		virtual void OnCreate(){}

		LRESULT SendMessage( UINT msg, WPARAM w,LPARAM l ){ return ::SendMessage( parent_->m_hWnd, msg, w, l ); }
		void InnerCreateWindow(D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int controlid);

	protected :
		D2DMat mat_; 		
		int stat_;
		FRectFBoxModel rc_;
		CHDL chdl_;
		FString name_;
		int id_;
		std::wstring guid_;	
};

class D2DControls : public D2DControl
{
	public :
		D2DControls();

		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual LRESULT KeyProc(D2DControl* sender, UINT message, WPARAM wParam, LPARAM lParam);

		virtual void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );
		virtual void UpdateScrollbar(D2DScrollbar* bar);
		virtual void MeToLast();
		virtual FRectF GetContentRect(){ return rc_.GetContentRect(); }
		virtual bool IsAutoResize(){ return auto_resize_; }
		virtual void DestroyControl();
		void Clear();
		virtual void BackColor(D2D1_COLOR_F clr){ clr_ = clr; }
		virtual void Bind(void* p){};
		virtual void OnCreate(){};

		bool IsLast( D2DControl* obj ) const;
		void Lineup(bool vertical=true ); // childを整列させる

		std::shared_ptr<D2DControl> Detach( D2DControl* );
		void Attach( std::shared_ptr<D2DControl> ctrl );

		D2DControl* GetControl( int id );

	public :
		std::vector<std::shared_ptr<D2DControl>> controls_;		
		FSizeF scrollbar_off_;			
		
	protected :
		bool bwindow_size_;		
		bool mouse_enter_;
		D2D1_COLOR_F clr_; // backcolor

	public :
		LRESULT SendMessageAll( D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		LRESULT SendMessageReverseAll( D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);		
};

struct SolidColor
{
	D2D1_COLOR_F color;
	CComPtr<ID2D1SolidColorBrush> br;
};



/////////////////////////////////////////////////////////////
//
// Controlsの親玉、D2DWindowの第一Controls
//
/////////////////////////////////////////////////////////////
//class D2DTopControls : public D2DControls
//{
//	public :
//		D2DTopControls(){}
//		virtual ~D2DTopControls(){}
//		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
//		virtual void CreateWindow(D2DWindow* parent, D2DControls* must_be_null, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1  );
//		void CreateWindow(D2DWindow* d, int stat, LPCWSTR name, int id=-1 );
//		
//		void CreateResource(bool bCreate);
//
//		virtual void OnCreate();
//		virtual void BackColor(D2D1_COLOR_F clr);
//
//		std::function<FRectF(FRectF, GDI32::FSize)> calc_auto_size_;
//		
//
//		OnWndProcExtDelegate OnWndProcExt_;
//	protected :
//		FRectF CalcAutoSize( const GDI32::FSize& sz );
//
//		FString error_msg_;
//		float zoom_;
//		bool bAutoSize_;
//		
//		SolidColor back_;
//		
//};

class D2DScrollbar : public D2DControl
{
	public :
		D2DScrollbar();
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		virtual void OnCreate();

		// Textboxで使用
		void CreateWindowEx(D2DWindow* parent, D2DControl* target, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );

		float OffsetOnBtn( int typ );
		void SetTotalSize( float size );

		void Show( bool visible );
		void Clear();
		
		virtual void SetRect(const FRectF& rc) 
		{ 
			rc_ = rc; 
		}
		
		D2DScrollbarInfo info_;
		D2DControl* target_control_;		
		
};

struct D2DStaticColor
{
	ID2D1Brush* forecolor;
	ID2D1Brush* backcolor;
	ID2D1Brush* wakucolor;
	ID2D1Brush* active_wakucolor;
};

class D2DTextbox : public D2DControl, public IBridgeTSFInterface
{
	public :

		std::function<void(const std::wstring&)> m_changed;

		static bool AppTSFInit();
		static void AppTSFExit();
		//static void CreateInputControl(D2DWindow* parent);
		//static std::map<HWND,TSF::CTextEditorCtrl*> s_text_inputctrl;

		enum TYP { SINGLELINE=0x1, MULTILINE=0x2, PASSWORD=0x4, RIGHT=0x8, CENTER=0x10,VCENTER=0x20 };

		D2DTextbox(HWND hWnd, TYP typ, std::function<void(const std::wstring&)> changed);
		~D2DTextbox() { delete ctrl_; }
	public :
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );

		virtual void SetRect( const FRectF& rc );

		// IBridgeInterface///////////////////////////////////////////
		virtual FRectF GetClientRect();
		virtual IDWriteTextFormat* GetFormat();
		virtual FRectFBoxModel GetClientRectEx();


		// functions ////////////////////////////////////////
		FString GetText();
		int InsertText( LPCWSTR str, int pos=-1, int strlen=-1 );

		void SetFont( CComPtr<IDWriteTextFormat> fmt );

		
		void SetText(LPCWSTR str);
		void SetText(VARIANT value);
		void SetViewText(LPCWSTR str);
		TYP GetType(){ return typ_; }
		void SetReadOnly(bool bReadOnly );
		void SetSize( const FSizeF& sz );
		void Clear(){ SetText(L""); }

		void TabEnable();
		void ActiveSw();

		//virtual D2DTextbox* Clone(D2DControls* pacontrol);

		std::wstring FilterInputString( LPCWSTR s, UINT len );

		// scroll //////////////////////////////////////////////
		float RowHeight();
		UINT RowCount( bool all );
		virtual void UpdateScrollbar(D2DScrollbar* bar);

		///////////////////////////////////////////////////////////////////
		std::function<HRESULT(D2DTextbox*,UINT,WPARAM,LPARAM)> EventMessageHanler_;

		std::function<LRESULT(D2DTextbox*,UINT key)> OnPushKey_;

		std::function<void(D2DTextbox*)> OnEnter_; // activeになった時、Enterキーではない
		std::function<void(D2DTextbox*)> OnLeave_; // activeでなくなった時
		
		std::function<bool(D2DTextbox*,const FString&)> OnValidation_;

		virtual void StatActive(bool bActive);

		OnWndProcExtDelegate OnWndProcExt_;
	public :
		D2D1_COLOR_F clr_fore_;
		D2D1_COLOR_F clr_back_;
		D2D1_COLOR_F clr_border_;
		D2D1_COLOR_F clr_active_border_;
		
	protected :
		void CalcRender( bool bLayoutUpdate );

	private :
		
		int OnKeyDown(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam);
		BOOL Clipboard( HWND hwnd, TCHAR ch );
		int TabCountCurrentRow();
		bool TryTrimingScrollbar();
	public :
		TYP typ_;
		bool bActive_;

		TSF::CTextEditorCtrl* ctrl_;
		TSF::CTextContainer ct_;
		FPointF offpt_;
		
		float font_height_;
		bool bUpdateScbar_;
		
		CComPtr<IDWriteTextFormat> fmt_;
		CComPtr<IDWriteTextLayout> text_layout_;

		std::shared_ptr<D2DScrollbar> scbar_; // vertical scrollbar
		D2DMat matEx_; 
};

class D2DStatic : public D2DControl
{
	public :
		D2DStatic():alignment_(0)
		{
			brush_fore_ = ColorF(ColorF::Black);
			brush_back_ = ColorF(ColorF::White);
		
		}
		void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, int stat, LPCWSTR name, int id=-1 );
		
		void CreateWindow( D2DWindow* parent, D2DControls* pacontrol, const FRectFBoxModel& rc, LPCWSTR name );

		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		
		
		
		FString GetText(){ return name_; }
		void SetText( LPCWSTR str, int align=-1 );
		void SetAlignment( int md ){ alignment_ = md; }

		void SetFont( IDWriteTextFormat* fmt );
	public :
		D2D1_COLOR_F brush_fore_;
		D2D1_COLOR_F brush_back_;

	private :
		
		//SingleLineText stext_;
		int alignment_;
};

void SetCursor( HCURSOR h );

};
