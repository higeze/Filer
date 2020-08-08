#pragma once
#include "MyWnd.h"
#include "Direct2DWrite.h"
#include "observable.h"

struct TextboxProperty;
class D2DTextbox;
struct TextboxProperty;

class CTextboxWnd:public CWnd
{
public:
	observable<std::wstring> m_path;
	observable<bool> m_isSaved = false;
	std::wstring m_text;
	std::shared_ptr<d2dw::CDirect2DWrite> m_pDirect;
	std::unique_ptr<D2DTextbox> m_pTxtbox;
	std::shared_ptr<TextboxProperty> m_spProp;

public :
	CTextboxWnd(std::shared_ptr<TextboxProperty> spProp);
	virtual ~CTextboxWnd();

	d2dw::CDirect2DWrite* GetDirectPtr() override { return  m_pDirect.get(); }
	observable<std::wstring>& GetObsPath() { return m_path; }
	observable<bool>& GetObsIsSaved() { return m_isSaved; }
	void Open();
	void Open(const std::wstring& path);
	void Save();
	void Save(const std::wstring& path);
	bool GetIsFocused()const;
	void Update();

private:
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEraseBkGnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void OnFinalMessage(HWND hWnd) override;
};
