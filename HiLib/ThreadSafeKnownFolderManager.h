#pragma once
#include "IDL.h"
#include <unordered_set>
#include <mutex>

namespace shell
{
	class CThreadSafeKnownFolderManager
	{
	private:
		std::mutex m_mtx;
		std::unordered_map<std::wstring, std::tuple<KNOWNFOLDERID, CIDL>> m_knownFolderIdIdlMap;
	public:
		static CThreadSafeKnownFolderManager* GetInstance()
		{
			static CThreadSafeKnownFolderManager mgr;
			return &mgr;
		}

		std::unordered_map<std::wstring, std::tuple<KNOWNFOLDERID, CIDL>>& GetIdIdlMap()
		{
			return m_knownFolderIdIdlMap;
		}

	public:
		CThreadSafeKnownFolderManager();
		void Update();
		bool IsExist(const std::wstring& path) const;
	};
}
