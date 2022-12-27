#pragma once

#include <windows.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <new>
#include <shlobj.h>
#include "Unknown.h"

// Default usage type flag to use with our IFileIsInUse implementation
#define FUT_DEFAULT FUT_EDITING

// Default capability flags to use with our IFileIsInUse implementation
#define OF_CAP_DEFAULT  OF_CAP_CANCLOSE | OF_CAP_CANSWITCHTO

// window message to inform main window to close a file


// this class implements the interface necessary to negotiate with the explorer
// when it hits sharing violations due to the file being open

class CFileIsInUseImpl : public CUnknown<IFileIsInUse>
{
public:
    static UINT WM_FILEINUSE_CLOSEFILE;
    static CComPtr<IFileIsInUse> CreateInstance(HWND hwnd, PCWSTR pszFilePath, FILE_USAGE_TYPE fut, DWORD dwCapabilities);
private:
    static HRESULT s_CreateInstance(HWND hwnd, PCWSTR pszFilePath, FILE_USAGE_TYPE fut, DWORD dwCapabilities, REFIID riid, void **ppv);

public:
    // IFileIsInUse
    IFACEMETHODIMP GetAppName(PWSTR *ppszName);
    IFACEMETHODIMP GetUsage(FILE_USAGE_TYPE *pfut);
    IFACEMETHODIMP GetCapabilities(DWORD *pdwCapabilitiesFlags);
    IFACEMETHODIMP GetSwitchToHWND(HWND *phwnd);
    IFACEMETHODIMP CloseFile();

private:
    CFileIsInUseImpl();
    ~CFileIsInUseImpl();

    HRESULT _Initialize(HWND hwnd, PCWSTR pszFilePath, FILE_USAGE_TYPE fut, DWORD dwCapabilities);
    HRESULT _AddFileToROT();
    HRESULT _RemoveFileFromROT();

    WCHAR _szFilePath[MAX_PATH];
    HWND _hwnd;
    DWORD _dwCapabilities;
    DWORD _dwCookie;
    FILE_USAGE_TYPE _fut;
};
