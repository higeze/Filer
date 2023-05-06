#pragma once
#include "MyWnd.h"
#include "FileIsInUse.h"
#include "PreviewHandlerFrame.h"
#include "ReactiveProperty.h"
#include "getter_macro.h"
#include "Debug.h"
#include <atlbase.h>
#include <atlcom.h>

class CPreviewWnd :public CWnd
{
private:
	std::wstring m_path;
	CComPtr<IFileIsInUse> m_pFileIsInUse;
	CComPtr<IPreviewHandler> m_pPreviewHandler;
	DECLARE_LAZY_GETTER(CComPtr<IPreviewHandlerFrame>, PreviewHandlerFramePtr)

public:
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