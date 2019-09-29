#pragma once
#include <unordered_map>
#include <mutex>
#include "IDL.h"

namespace shell
{
	class CThreadSafeDriveFolderManager
	{
	private:
		std::mutex m_mtx;
		std::unordered_map<std::wstring, CIDL> m_driveFolderMap;
	public:
		static CThreadSafeDriveFolderManager* GetInstance()
		{
			static CThreadSafeDriveFolderManager mgr;
			return &mgr;
		}

	public:
		CThreadSafeDriveFolderManager();
		std::unordered_map<std::wstring, CIDL>& GetPathIdlMap() { return m_driveFolderMap; }

		void Update();
		bool IsExist(const std::wstring& path) const;
	};
}

