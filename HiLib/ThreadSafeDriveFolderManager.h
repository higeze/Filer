#pragma once
#include <unordered_set>
#include <mutex>

namespace shell
{
	class CThreadSafeDriveFolderManager
	{
	private:
		std::mutex m_mtx;
		std::unordered_set<std::wstring> m_driveFolderSet;
	public:
		static CThreadSafeDriveFolderManager* GetInstance()
		{
			static CThreadSafeDriveFolderManager mgr;
			return &mgr;
		}

	public:
		CThreadSafeDriveFolderManager();
		void Update();
		bool IsExist(const std::wstring& path) const;
	};
}

