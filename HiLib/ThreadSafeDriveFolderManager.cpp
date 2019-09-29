#include "ThreadSafeDriveFolderManager.h"

namespace shell
{
	CThreadSafeDriveFolderManager::CThreadSafeDriveFolderManager()
	{
		Update();
	}
	void CThreadSafeDriveFolderManager::Update()
	{
		std::lock_guard<std::mutex> lock(m_mtx);
		m_driveFolderMap.clear();

		CComPtr<IShellFolder> pDesktopFolder;
		::SHGetDesktopFolder(&pDesktopFolder);

		std::array<wchar_t, 64> logicalDrives;

		if (::GetLogicalDriveStrings(64, logicalDrives.data())) {
			wchar_t* pPath;
			for (pPath = logicalDrives.data(); *pPath != L'\0'; pPath += lstrlen(pPath) + 1) {
				CIDL relativeIdl;

				ULONG         chEaten;
				ULONG         dwAttributes;
				HRESULT hr = pDesktopFolder->ParseDisplayName(
					NULL,
					NULL,
					const_cast<LPWSTR>(pPath),
					&chEaten,
					relativeIdl.ptrptr(),
					&dwAttributes);
				if (FAILED(hr)) { throw std::exception("CDriveFolderManager::CDriveFolderManager"); }


				m_driveFolderMap.emplace(pPath, relativeIdl);
			}
		}
	}
	bool CThreadSafeDriveFolderManager::IsExist(const std::wstring& path) const
	{
		return m_driveFolderMap.find(path) != m_driveFolderMap.end();
	}

}
