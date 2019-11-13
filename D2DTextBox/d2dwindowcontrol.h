#pragma once

#include "D2DContextEx.h"
#include "D2DWindow.h"
#include "D2DWindowMessage.h"
#include "D2DWindowMessageStruct.h"
#include "IBridgeTSFInterface.h"
#include "TextEditor.h"
#include "CellProperty.h"

class CTextEditor;
class LayoutLineInfo;

	typedef std::function<LRESULT(D2DControl*, UINT, WPARAM, LPARAM)> OnWndProcExtDelegate;

	class D2DControl;
	class D2DWindow;

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
		virtual void DestroyControl();


		void Enable(bool bEnable);
		void Visible(bool bVisible);
		bool IsEnable()  const;
		bool IsVisible()  const;

		virtual void OnHostage(D2DControls* p, bool block) {}
	public:
		D2DWindow* m_pWnd;

	protected:
		int stat_;
	};

	class D2DTextbox : public D2DControl, public IBridgeTSFInterface
	{
	public:

		std::function<void(const std::wstring&)> m_changed;
		std::shared_ptr<CellProperty> m_pProp;

		static bool AppTSFInit();
		static void AppTSFExit();

		D2DTextbox(D2DWindow* pWnd, const std::shared_ptr<CellProperty>& pProp,std::function<void(const std::wstring&)> changed);
		~D2DTextbox() { delete ctrl_; }
	public:
		virtual LRESULT WndProc(D2DWindow* parent, UINT message, WPARAM wParam, LPARAM lParam);
		void CreateWindow(D2DWindow* parent, int stat, LPCWSTR name);

		// IBridgeInterface///////////////////////////////////////////
		d2dw::CRectF GetClientRect() const;
		d2dw::CRectF GetContentRect() const;

		// functions ////////////////////////////////////////
		std::wstring GetText();
		int InsertText(LPCWSTR str, int pos = -1, int strlen = -1);

		void SetText(LPCWSTR str);
		void SetText(VARIANT value);
		void Clear() { SetText(L""); }
		std::wstring FilterInputString(LPCWSTR s, UINT len);
	private:

		int OnKeyDown(D2DWindow* d, UINT message, WPARAM wParam, LPARAM lParam);
		BOOL Clipboard(HWND hwnd, TCHAR ch);
		int TabCountCurrentRow();
	public:
		bool bActive_;
		CTextEditor* ctrl_;
	};