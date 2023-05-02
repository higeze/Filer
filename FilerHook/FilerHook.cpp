#pragma once
#include "pch.h"

#include <cstdio>
#include <tchar.h>
#include "FilerHook.h"

//shared segment
#pragma data_seg(".shareddata")
HHOOK g_hPreviewHook;
HWND g_hWndOwner;
HWND g_hWndPreview;
void* g_pControl;


HHOOK hKeyHook = 0;
HHOOK hCallHook = 0;
HWND g_hWnd = 0;        // キーコードの送り先のウインドウハンドル
#pragma data_seg()
HINSTANCE g_hInst;

/***********************/
/* Preview Window Hook */
/***********************/
EXPORT_API_ int SetPreviewWindowHook(HWND hWndOwner, void* pControl, HWND hWndPreview)
{
    DWORD threadId = ::GetWindowThreadProcessId(hWndPreview, nullptr);
    wchar_t className[512];
    ::GetClassName(hWndPreview, className, 512);
    ::OutputDebugString(className);
    g_hPreviewHook = ::SetWindowsHookEx(WH_KEYBOARD, KeyHookProc, g_hInst, 0);

    if (g_hPreviewHook != NULL){
        g_hWndOwner = hWndOwner;
        g_hWndPreview = hWndPreview;
        g_pControl = pControl;
    } else {
        LPSTR lpError = NULL ;
        DWORD dwResult = FormatMessageA(
                                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                                NULL,
                                ::GetLastError(),
                                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                (LPSTR) &lpError,
                                0,
                                NULL);
        if ( dwResult != 0 ) {
            ::OutputDebugStringA(lpError);
            LocalFree(lpError);
        }
    }
    return 0;
}

EXPORT_API_ int ResetPreviewWindowHook()
{
    if (UnhookWindowsHookEx(g_hPreviewHook) != 0){
    }else{
    }
    return 0;
}

EXPORT_API_ LRESULT CALLBACK PreviewWindowHookProc(int code,WPARAM wParam,LPARAM lParam)
{
    LPCWPSTRUCT pCwpret = reinterpret_cast<LPCWPSTRUCT>(lParam);
    switch (pCwpret->message) {
		case WM_ACTIVATE:
			if (!LOWORD(pCwpret->wParam)) {
				break;
			}
		case WM_SHOWWINDOW:
			if (!pCwpret->wParam) {
				break;
			}
		case WM_SETFOCUS:
		{
            ::PostMessage(g_hWndOwner, ::RegisterWindowMessage(L"PreviewWindowSetFocus"), (WPARAM)g_pControl, (LPARAM)NULL);
			//auto spControl = FindHostWndControlPtr(hWnd);
			//if (spControl && spControl->GetHostHWnd() == hWnd) {
			//	SetFocusedControlPtr(spControl);
			//}
			//CD2DWControl* pControl = reinterpret_cast<CD2DWControl*>(dwRefData);
			//SetFocusedControlPtr(std::dynamic_pointer_cast<CD2DWControl>(pControl->shared_from_this()));
		}
		break;

    }
    return ::CallNextHookEx(g_hPreviewHook, code, wParam, lParam);
}

EXPORT_API_ int SetHook(HWND hWnd)
{
    hKeyHook = SetWindowsHookEx(WH_KEYBOARD, KeyHookProc, g_hInst, 0);
    if (hKeyHook == NULL)
    //hCallHook = SetWindowsHookEx(WH_CALLWNDPROC, CallHookProc, hInst, 0);
    //if (hCallHook == NULL)
    {
        // フック失敗
    }
    else
    {
        // フック成功
        g_hWnd = hWnd;
    }
    return 0;
}
EXPORT_API_ int ResetHook()
{
    if (UnhookWindowsHookEx(hKeyHook) != 0)
    //if (UnhookWindowsHookEx(hCallHook) != 0)
    {
        // フック解除成功
    }
    else
    {
        // フック解除失敗
    }
    return 0;
}
EXPORT_API_ LRESULT CALLBACK KeyHookProc(int nCode, WPARAM wp, LPARAM lp)
{
    TCHAR msg[64] = { 0 };
    if (nCode < 0)    // 決まり事
        return CallNextHookEx(hKeyHook, nCode, wp, lp);
    if (nCode == HC_ACTION)
    {
        //目的のウインドウにキーボードメッセージと、キーコードの転送
        
        // どこで押してもHOOKする
        // ボタンが押された状態の時限定(離しはスルー)
        if ((lp & 0x80000000) == 0)
        {
            // 通常キー
            if ((lp & 0x20000000) == 0)
            {
                if (wp == VK_INSERT) {
                    HMENU hMenu = ::CreatePopupMenu();
                    MENUITEMINFO mii = { 0 };
                    mii.cbSize = sizeof(MENUITEMINFO);
                    mii.fMask = MIIM_TYPE | MIIM_ID;
                    mii.fType = MFT_STRING;
                    mii.fState = MFS_ENABLED;
                    mii.wID = 111;
                    mii.dwTypeData = (LPWSTR)L"AAA";
                    ::InsertMenuItemW(hMenu, ::GetMenuItemCount(hMenu), TRUE, &mii);
                    POINT pt;
                    ::GetCursorPos(&pt);
                    HWND hWnd = ::GetForegroundWindow();
                    BOOL cmd = ::TrackPopupMenu(hMenu, 
			            TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			            pt.x,
			            pt.y,
                        0,
			            hWnd,
                        NULL);

                    auto a = cmd;

                }
                //if (wp == VK_INSERT) {
                //    SYSTEMTIME st;
                //    GetSystemTime( &st);
                //    wchar_t ymd[9] = { 0 };
                //    int  size = ::GetDateFormatEx(_T("ja-JP"), 0, &st, L"yyyyMMdd", ymd, 9, 0);     // カレンダー

                //    for (auto p = ymd; *p; ++p) {
                //        keybd_event(*p, 0, 0, 0);
                //        keybd_event(*p, 0, KEYEVENTF_KEYUP, 0);
                //        ::Sleep(5);
                //    }

                //    return 1;
                //}

                // Enter以外
                //if (wp != VK_RETURN)
                //{
                //    _stprintf_s(msg, _T("%cが押されたよ(´・ω・`)！"), int(wp));
                //    MessageBox(NULL, msg, NULL, MB_OK);
                //    PostMessage(g_hWnd, WM_KEYDOWN, wp, 0);
                //}
            }
            // システムキー(Alt(+何か)、もしくはF10の時)
            else
            {
                MessageBox(NULL, TEXT("システムキーが押されたよ！"), NULL, MB_OK);
                PostMessage(g_hWnd, WM_SYSKEYDOWN, wp, 0);
            }
        }
    }
    return CallNextHookEx(hKeyHook, nCode, wp, lp);
}

EXPORT_API_ LRESULT CALLBACK CallHookProc(int nCode, WPARAM wParam, LPARAM lParam) 
{ 
    CHAR szCWPBuf[256]; 
    CHAR szMsg[16]; 
    HDC hdc; 
    static int c = 0; 
    size_t cch;
    HRESULT hResult; 
 
    if (nCode < 0)  // do not process message 
        return CallNextHookEx(hCallHook, nCode, wParam, lParam); 
    if (nCode == HC_ACTION) {
        CWPSTRUCT* pcwp = (CWPSTRUCT*)lParam;
        if (pcwp->message == WM_CONTEXTMENU) {
                MessageBox(NULL, TEXT("コンテキストメニューだよ！"), NULL, MB_OK);
                
        }

    }
     
    return CallNextHookEx(hCallHook, nCode, wParam, lParam); 
} 

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    //switch (ul_reason_for_call)
    //{
    //case DLL_PROCESS_ATTACH:
    //case DLL_THREAD_ATTACH:
    //case DLL_THREAD_DETACH:
    //case DLL_PROCESS_DETACH:
    //    break;
    //}
    //return TRUE;
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            // アタッチ
            g_hInst = hModule;
            break;
        case DLL_PROCESS_DETACH:
            // デタッチ
            break;
    }
    return TRUE;
}

