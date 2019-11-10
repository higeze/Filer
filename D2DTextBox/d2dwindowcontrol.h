#pragma once

#include "D2DContextEx.h"
#include "D2DWindow.h"
#include "ItemLoopArray.h"
#include "D2DWindowMessage.h"
#include "D2DWindowMessageStruct.h"
#include "IBridgeTSFInterface.h"
#include "faststack.h"
#include "TextEditor.h"
#include "CellProperty.h"


//#undef CreateWindow


//#define COLOR_DEF_FLOAT_SELECT	D2RGBA(54,101,179,100)
//#define COLOR_DEF_SELECTED		D2RGBA(54,101,179,255)
//#define COLOR_DEF_BORDER		D2RGB(0,0,0)
//#define COLOR_DEF_FORE			D2RGBA(54,101,179,255)

namespace TSF {
class CTextEditorCtrl;
class LayoutLineInfo;
};

namespace V4
{

	typedef std::function<LRESULT(D2DControl*, UINT, WPARAM, LPARAM)> OnWndProcExtDelegate;

	class D2DControl;
	class D2DControls;
	class D2DWindow;
	class D2DScrollbar;
	class D2DControl : public D2DCaptureObject
	{
	public:
		D2DControl();
		virtual ~D2DControl();

		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam) = 0;

		virtual void CreateWindow(D2DWindow* parent, int stat, LPCWSTR name);

		int Stat(int new_stat);

		int GetStat()  const { return stat_; }
		void SetStat(int s) { stat_ = s; }
		bool IsThroughMessage(UINT message);

		virtual void UpdateStatus(D2DControls* p) {}
		virtual D2DControl* UnPack() { return this; }
		virtual void StatActive(bool bActive);
		virtual bool IsAutoResize() { return auto_resize_; }
		virtual void DestroyControl();


		void Enable(bool bEnable);
		void Visible(bool bVisible);
		bool IsEnable()  const;
		bool IsVisible()  const;

		virtual void OnHostage(D2DControls* p, bool block) {}
	public:
		D2DWindow* parent_;
		void* target_;
		bool auto_resize_;

	protected:
		virtual void OnCreate() {}

		LRESULT SendMessage(UINT msg, WPARAM w, LPARAM l) { return ::SendMessage(parent_->m_hWnd, msg, w, l); }
		void InnerCreateWindow(D2DWindow* parent, int stat, LPCWSTR name);

	protected:
		int stat_;
	};

	class D2DTextbox : public D2DControl, public IBridgeTSFInterface
	{
	public:

		std::function<void(const std::wstring&)> m_changed;
		D2DWindow* m_pWnd;
		std::shared_ptr<CellProperty> m_pProp;

		static bool AppTSFInit();
		static void AppTSFExit();

		enum TYP { SINGLELINE = 0x1, MULTILINE = 0x2, PASSWORD = 0x4, RIGHT = 0x8, CENTER = 0x10, VCENTER = 0x20 };

		D2DTextbox(D2DWindow* pWindow, const std::shared_ptr<CellProperty>& pProp, TYP typ, std::function<void(const std::wstring&)> changed);
		~D2DTextbox() { delete ctrl_; }
	public:
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		void CreateWindow(D2DWindow* parent, int stat, LPCWSTR name);

		//virtual void SetRect( const d2dw::CRectF& rc );

		// IBridgeInterface///////////////////////////////////////////
		d2dw::CRectF GetClientRect() const;
		d2dw::CRectF GetContentRect() const;


		// functions ////////////////////////////////////////
		std::wstring GetText();
		int InsertText(LPCWSTR str, int pos = -1, int strlen = -1);

		void SetText(LPCWSTR str);
		void SetText(VARIANT value);
		void SetViewText(LPCWSTR str);
		TYP GetType() { return typ_; }
		void SetReadOnly(bool bReadOnly);
		//void SetSize( const d2dw::CSizeF& sz );
		void Clear() { SetText(L""); }

		void TabEnable();
		//void ActiveSw();

		std::wstring FilterInputString(LPCWSTR s, UINT len);

		///////////////////////////////////////////////////////////////////
		std::function<HRESULT(D2DTextbox*, UINT, WPARAM, LPARAM)> EventMessageHanler_;

		std::function<LRESULT(D2DTextbox*, UINT key)> OnPushKey_;

		std::function<void(D2DTextbox*)> OnEnter_; // activeになった時、Enterキーではない
		std::function<void(D2DTextbox*)> OnLeave_; // activeでなくなった時

		std::function<bool(D2DTextbox*, const std::wstring&)> OnValidation_;

		virtual void StatActive(bool bActive);
	public:
	protected:

	private:

		int OnKeyDown(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam);
		BOOL Clipboard(HWND hwnd, TCHAR ch);
		int TabCountCurrentRow();
	public:
		TYP typ_;
		bool bActive_;

		TSF::CTextEditorCtrl* ctrl_;
	};
}