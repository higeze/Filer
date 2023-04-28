#pragma once
#include "MyWnd.h"
#include "FileIsInUse.h"
#include "PreviewHandlerFrame.h"
#include "ReactiveProperty.h"
#include "getter_macro.h"
#include "Debug.h"

class CPreviewWnd :public CWnd
{
private:
	CComPtr<IFileIsInUse> m_pFileIsInUse;
	CComPtr<IPreviewHandler> m_pPreviewHandler;
	DECLARE_LAZY_GETTER(CComPtr<IPreviewHandlerFrame>, PreviewHandlerFramePtr);

public:
	//ReactiveProperty
	ReactiveWStringProperty Path;
	//Constructor/Destructor
	CPreviewWnd();
	~CPreviewWnd();

	void Open(const std::wstring& path);
	void Open();
	void Close();

	//LRESULT OnCreate(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//LRESULT OnClose(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//LRESULT OnDestroy(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = TRUE; }
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};