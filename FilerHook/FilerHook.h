#pragma once
#include "pch.h"

#ifdef EXPORT_
#define EXPORT_API_ __declspec(dllexport)
#else
#define EXPORT_API_ __declspec(dllimport)
#endif 

EXPORT_API_ int SetPreviewWindowHook(HWND owner_hwnd, void* control_ptr, HWND preview_hwnd);
EXPORT_API_ int ResetPreviewWindowHook();
EXPORT_API_ LRESULT CALLBACK PreviewWindowHookProc(int code,WPARAM wParam,LPARAM lParam);


EXPORT_API_ LRESULT CALLBACK KeyHookProc(int, WPARAM, LPARAM);
EXPORT_API_ LRESULT CALLBACK CallHookProc(int, WPARAM, LPARAM);
EXPORT_API_ int SetHook(HWND hWnd);
EXPORT_API_ int ResetHook();
