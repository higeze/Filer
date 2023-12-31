// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#if defined _M_IX86
# pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
# pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
# pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
# pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#include "FilerApplication.h"
#include "Debug.h"

void QuickAccessEnumeration()
{
		CCoInitializer coinit(COINIT_APARTMENTTHREADED);
		COleInitializer oleinit;
		{
			auto spKnownFolder = CKnownFolderManager::GetInstance()->GetKnownFolderById(FOLDERID_Recent);
			CComPtr<IEnumIDList> pEnumIdl;
			::OutputDebugStringW(L"[FOLDERID_Recent]\r\n");
			if (SUCCEEDED(spKnownFolder->GetShellFolderPtr()->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &pEnumIdl)) && pEnumIdl) {
				CIDL nextIdl;
				while (SUCCEEDED(pEnumIdl->Next(1, nextIdl.ptrptr(), NULL)) && nextIdl) {
					std::shared_ptr<CShellFile> spFile = spKnownFolder->CreateShExFileFolder(nextIdl);
					::OutputDebugStringW((spFile->GetPath() + L"\n").c_str());

					CComPtr<IShellLink> pShellLink;
					FAILED_CONTINUE(pShellLink.CoCreateInstance(CLSID_ShellLink));
					CComPtr<IPersistFile> pPersistFile;
					FAILED_CONTINUE(pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile));
					FAILED_CONTINUE(pPersistFile->Load(spFile->GetPath().c_str(), STGM_READ));
					FAILED_CONTINUE(pShellLink->Resolve(NULL, SLR_NONE));
					std::wstring targetPath;
					WIN32_FIND_DATA fd;
					FAILED_CONTINUE(pShellLink->GetPath(::GetBuffer(targetPath, MAX_PATH), MAX_PATH, &fd, SLGP_UNCPRIORITY));
					::ReleaseBuffer(targetPath);
					::OutputDebugStringW((L">>" + targetPath + L"\n").c_str());
					nextIdl.Clear();
				}
			}
		}
		{
			std::vector<std::pair<std::wstring, std::wstring>> map = {
				std::make_pair(L"Quick Access", L"{679F85CB-0220-4080-B29B-5540CC05AAB6}"),
				std::make_pair(L"Frequent Places Folder", L"{3936E9E4-D92C-4EEE-A85A-BC16D5EA0819}")};

			for (const auto& pair : map) {
				CComPtr<IUnknown> pUk;
				CLSID clsid;
				CLSIDFromString(CComBSTR(pair.second.c_str()), &clsid);
				FAILED_CONTINUE(pUk.CoCreateInstance(clsid));

				CComPtr<IShellFolder2> pShellFolder;
				FAILED_CONTINUE(pUk->QueryInterface(IID_PPV_ARGS(&pShellFolder)));

				CComPtr<IEnumIDList> enumIdl;
				::OutputDebugStringW((L"[" + pair.first + L"]\r\n").c_str());
				if (SUCCEEDED(pShellFolder->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_INCLUDESUPERHIDDEN, &enumIdl)) && enumIdl) {
					CIDL nextIdl;
					while (true) {
						SUCCEEDED(enumIdl->Next(1, nextIdl.ptrptr(), NULL));
						if (!nextIdl) { break; }
						STRRET strret;
						pShellFolder->GetDisplayNameOf(nextIdl.ptr(), SHGDN_FORPARSING, &strret);
						::OutputDebugStringW((nextIdl.strret2wstring(strret) + L"\r\n").c_str());
						nextIdl.Clear();
					}
				}
			}
		}
		//{
		//	//std::vector<std::pair<std::wstring, std::wstring>> map = {
		//	//	std::make_pair(L"Recent Items Instance Folder", L"{4564b25e-30cd-4787-82ba-39e73a750b14}"),
		//	//	std::make_pair(L"Recent Places Folder", L"{22877a6d-37a1-461a-91b0-dbda5aaebc99}")};

		//	//for (const auto& pair : map) {
		//	//	CComPtr<IUnknown> pUk;
		//	//	CLSID clsid;
		//	//	CLSIDFromString(CComBSTR(pair.second.c_str()), &clsid);
		//	//	FAILED_CONTINUE(pUk.CoCreateInstance(clsid));

		//	//	CComPtr<IShellFolder2> pShellFolder;
		//	//	FAILED_CONTINUE(pUk->QueryInterface(IID_PPV_ARGS(&pShellFolder)));

		//	//	CIDL folderIdl;
		//	//	STRRET strret;
		//	//	FAILED_CONTINUE(pShellFolder->GetDisplayNameOf(folderIdl.ptr(), SHGDN_FORPARSING, &strret));
		//	//	::OutputDebugString(folderIdl.strret2wstring(strret).c_str());

		//		//CComPtr<IShellLink> pShellLink;
		//		//if (SUCCEEDED(pShellFolder->QueryInterface(IID_PPV_ARGS(&pShellLink)))) {

		//		//}


		//		//CComPtr<IPersistFolder3> pPersistFolder;
		//		//FAILED_CONTINUE(pShellFolder->QueryInterface(IID_PPV_ARGS(&pPersistFolder)));
		//		//PERSIST_FOLDER_TARGET_INFO pfti = {0};
		//		//FAILED_CONTINUE(pPersistFolder->GetFolderTargetInfo(&pfti));
		//		//CIDL folderIdl(pfti.pidlTargetFolder);
		//		////FAILED_CONTINUE(pPersistFolder->GetCurFolder(folderIdl.ptrptr()));

		//		//CComPtr<IShellLink> pShellLink;
		//		//FAILED_CONTINUE(pShellLink.CoCreateInstance(CLSID_ShellLink));
		//		//CComPtr<IPersistFile> pPersistFile;
		//		//FAILED_CONTINUE(pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile));
		//		//std::wstring folderPath;
		//		//FALSE_CONTINUE(::SHGetPathFromIDListW(folderIdl.ptr(), ::GetBuffer(folderPath, MAX_PATH)));
		//		//::ReleaseBuffer(folderPath);
		//		//FAILED_CONTINUE(pPersistFile->Load(folderPath.c_str(), STGM_READ));
		//		//FAILED_CONTINUE(pShellLink->Resolve(NULL, SLR_NONE));
		//		//std::wstring targetPath;
		//		//WIN32_FIND_DATA fd = {0};
		//		//FAILED_CONTINUE(pShellLink->GetPath(::GetBuffer(targetPath, MAX_PATH), MAX_PATH, &fd, SLGP_UNCPRIORITY));
		//		//::ReleaseBuffer(targetPath);
		//		//::OutputDebugStringW((L">>" + targetPath + L"\r\n").c_str());

		//		//CComPtr<IEnumIDList> enumIdl;
		//		//::OutputDebugStringW((L"[" + pair.first + L"]\r\n").c_str());
		//		//if (SUCCEEDED(pShellFolder->EnumObjects(NULL, 0, &enumIdl)) && enumIdl) {
		//		//	CIDL nextIdl;
		//		//	while (true) {
		//		//		SUCCEEDED(enumIdl->Next(1, nextIdl.ptrptr(), NULL));
		//		//		if (!nextIdl) { break; }
		//		//		STRRET strret;
		//		//		pShellFolder->GetDisplayNameOf(nextIdl.ptr(), SHGDN_FORPARSING, &strret);
		//		//		::OutputDebugStringW((nextIdl.strret2wstring(strret) + L"\r\n").c_str());
		//		//		nextIdl.Clear();
		//		//	}
		//		//}
		//	}

		//}
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR     lpCmdLine,
                     int       nCmdShow)
{
	CFilerApplication::GetInstance()->Init();
	auto ret = CFilerApplication::GetInstance()->Run();
	CFilerApplication::GetInstance()->Term();

	return ret;
}