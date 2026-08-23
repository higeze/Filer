#include "ThreadSafeKnownFolderManager.h"
#include "ShellFunction.h"
#include "ThreadPool.h"

namespace shell
{
	CThreadSafeKnownFolderManager::CThreadSafeKnownFolderManager()
	{
		Update();
	}
	void CThreadSafeKnownFolderManager::Update()
	{
		APTTYPE apt;
		APTTYPEQUALIFIER qual;
		CoGetApartmentType(&apt, &qual);
		CThreadSafeComPtr<IShellFolder> pDesktopFolder;
		::SHGetDesktopFolder(&pDesktopFolder);

		CShellThread::Run(FILE_LINE_FUNC, 1, [&]->void {
			std::lock_guard<std::mutex> lock(m_mtx);
			m_knownFolderIdIdlMap.clear();

			CThreadSafeComPtr<IKnownFolderManager> pMgr;
			if (FAILED(pMgr.CoCreateInstance(CLSID_KnownFolderManager, NULL, CLSCTX_INPROC_SERVER))) { return; }

			CThreadSafeComPtr<IShellFolder> pDesktopFolder;
			::SHGetDesktopFolder(&pDesktopFolder);

			KNOWNFOLDERID* pknownid;
			UINT count(0);
			if (FAILED(pMgr.Call(&IKnownFolderManager::GetFolderIds, &pknownid, &count))) { return; }
			for (UINT i = 0; i < count; i++) {
				CThreadSafeComPtr<IKnownFolder> pFolder;
				if (SUCCEEDED(pMgr.Call(&IKnownFolderManager::GetFolder, pknownid[i], &pFolder))) {
					//IDL
					APTTYPE aptType;
					APTTYPEQUALIFIER aptQualifier;
					HRESULT hr = ::CoGetApartmentType(&aptType, &aptQualifier);
					CIDL idl;
					if (SUCCEEDED(pFolder.Call(&IKnownFolder::GetIDList, KF_FLAG_DEFAULT, idl.ptrptr()))) {

						//Path Hint:pFolder->GetPath couldn't get GUID
						std::wstring path = shell::GetDisplayNameOf(pDesktopFolder, idl, SHGDN_FORPARSING);

						if (pknownid[i] == FOLDERID_Desktop) {
							idl.Clear();
							::SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, idl.ptrptr());
							m_knownFolderIdIdlMap.insert_or_assign(path, std::make_tuple(pknownid[i], std::move(idl)));
						} else {
							m_knownFolderIdIdlMap.emplace(path, std::make_tuple(pknownid[i], std::move(idl)));
						}
					}
				}
			}
		}).get();
	}
	bool CThreadSafeKnownFolderManager::IsExist(const std::wstring& path) const
	{
		return m_knownFolderIdIdlMap.find(path) != m_knownFolderIdIdlMap.end();
	}
}