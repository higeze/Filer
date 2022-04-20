#include "FileIsInUse.h"

UINT CFileIsInUseImpl::WM_FILEINUSE_CLOSEFILE = ::RegisterWindowMessageA("WM_FILEINUSE_CLOSEFILE");

CFileIsInUseImpl::CFileIsInUseImpl(): _cRef(1), _hwnd(NULL), _fut(FUT_GENERIC), _dwCapabilities(0), _dwCookie(0)
{
    _szFilePath[0]  = '\0';
}

CFileIsInUseImpl::~CFileIsInUseImpl()
{
    _RemoveFileFromROT();
}

HRESULT CFileIsInUseImpl::_Initialize(HWND hwnd, PCWSTR pszFilePath, FILE_USAGE_TYPE fut, DWORD dwCapabilities)
{
    _hwnd  = hwnd;
    _fut   = fut;
    _dwCapabilities = dwCapabilities;
    HRESULT hr = StringCchCopy(_szFilePath, ARRAYSIZE(_szFilePath), pszFilePath);
    if (SUCCEEDED(hr))
    {
        hr = _AddFileToROT();
    }
    return hr;
}

HRESULT CFileIsInUseImpl::s_CreateInstance(HWND hwnd, PCWSTR pszFilePath, FILE_USAGE_TYPE fut, DWORD dwCapabilities, REFIID riid, void **ppv)
{
    CFileIsInUseImpl *pfiu = new (std::nothrow) CFileIsInUseImpl();
    HRESULT hr = (pfiu) ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        hr = pfiu->_Initialize(hwnd, pszFilePath, fut, dwCapabilities);
        if (SUCCEEDED(hr))
        {
            hr = pfiu->QueryInterface(riid, ppv);
        }
        pfiu->Release();
    }
    return hr;
}

HRESULT CFileIsInUseImpl::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CFileIsInUseImpl, IFileIsInUse),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

ULONG CFileIsInUseImpl::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CFileIsInUseImpl::Release()
{
    ULONG cRef = InterlockedDecrement(&_cRef);
    if (!cRef)
    {
        delete this;
    }
    return cRef;
}

// IFileIsInUse

HRESULT CFileIsInUseImpl::CloseFile()
{
    // Notify main application window that we need to close
    // the file handle associated with this entry.  We do
    // not pass anything with this message since this sample
    // will only have a single file open at a time.
    SendMessage(_hwnd, CFileIsInUseImpl::WM_FILEINUSE_CLOSEFILE, (WPARAM)NULL, (LPARAM)NULL);
    _RemoveFileFromROT();
    return S_OK;
}

// IFileIsInUse

HRESULT CFileIsInUseImpl::GetAppName(PWSTR *ppszName)
{
    HRESULT hr = E_FAIL;
    WCHAR szModule[MAX_PATH];
    UINT cch = GetModuleFileName(NULL, szModule, ARRAYSIZE(szModule));
    if (cch != 0)
    {
        hr = SHStrDup(PathFindFileName(szModule), ppszName);
    }
    return hr;
}

// IFileIsInUse

HRESULT CFileIsInUseImpl::GetUsage(FILE_USAGE_TYPE *pfut)
{
    *pfut = _fut;
    return S_OK;
}

// IFileIsInUse

HRESULT CFileIsInUseImpl::GetCapabilities(DWORD *pdwCapabilitiesFlags)
{
    *pdwCapabilitiesFlags = _dwCapabilities;
    return S_OK;
}

// IFileIsInUse

HRESULT CFileIsInUseImpl::GetSwitchToHWND(HWND *phwnd)
{
    *phwnd = _hwnd;
    return S_OK;
}

HRESULT CFileIsInUseImpl::_AddFileToROT()
{
    IRunningObjectTable *prot;
    HRESULT hr = GetRunningObjectTable(NULL, &prot);
    if (SUCCEEDED(hr))
    {
        IMoniker *pmk;
        hr = CreateFileMoniker(_szFilePath, &pmk);
        if (SUCCEEDED(hr))
        {
            // Add ROTFLAGS_ALLOWANYCLIENT to make this work accross security boundaries
            hr = prot->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE | ROTFLAGS_ALLOWANYCLIENT,
                                static_cast<IFileIsInUse *>(this), pmk, &_dwCookie);
            if (hr == CO_E_WRONG_SERVER_IDENTITY)
            {
                // this failure is due to ROTFLAGS_ALLOWANYCLIENT and the fact that we don't
                // have the AppID registered for our CLSID. Try again without ROTFLAGS_ALLOWANYCLIENT
                // knowing that this means this can only work in the scope of apps running with the
                // same MIC level.
                hr = prot->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE,
                                    static_cast<IFileIsInUse *>(this), pmk, &_dwCookie);
            }
            pmk->Release();
        }
        prot->Release();
    }
    return hr;
}

HRESULT CFileIsInUseImpl::_RemoveFileFromROT()
{
    IRunningObjectTable *prot;
    HRESULT hr = GetRunningObjectTable(NULL, &prot);
    if (SUCCEEDED(hr))
    {
        if (_dwCookie)
        {
            hr = prot->Revoke(_dwCookie);
            _dwCookie = 0;
        }

        prot->Release();
    }
    return hr;
}
