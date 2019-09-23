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
		m_driveFolderSet.clear();
		std::array<wchar_t, 64> logicalDrives;

		if (::GetLogicalDriveStrings(64, logicalDrives.data())) {
			wchar_t* pPath;
			for (pPath = logicalDrives.data(); *pPath != L'\0'; pPath += lstrlen(pPath) + 1) {
				m_driveFolderSet.emplace(pPath);
			}
		}
	}
	bool CThreadSafeDriveFolderManager::IsExist(const std::wstring& path) const
	{
		return m_driveFolderSet.find(path) != m_driveFolderSet.end();
	}

}
