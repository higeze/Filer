//#include "stdafx.h"
#include "Favorite.h"
#include "ThreadPool.h"
#include "ShellFileFactory.h"
#include "ConsoleTimer.h"
#include "MyString.h"



CFavorite::CFavorite()
	:m_path(), m_shortName(){}

CFavorite::CFavorite(std::wstring path, std::wstring shortName)
	:m_path(path), m_shortName(shortName), m_spFile(){}

CFavorite::~CFavorite()
{
	if (m_futureFile.valid() /*&& m_futureWatch.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready*/) {
		m_futureFile.get();
	}
}


std::shared_ptr<CShellFile>& CFavorite::GetShellFile(const std::function<void()>& changed)
{
	if (!GeLockShellFile()) {
		//Thread stated, not finished
		if (m_futureFile.valid() && m_futureFile.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
			//Do nothing
		//Thread finished
		} else if (m_futureFile.valid() && m_futureFile.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
			SetLockShellFile(m_futureFile.get());
		//Thread not started
		} else {
			m_futureFile = CThreadPool::GetInstance()->enqueue([](const std::wstring& path, const std::function<void()>& changed) {
				CONSOLETIMER(wstr2str(path));
				auto ret = CShellFileFactory::GetInstance()->CreateShellFilePtr(path);
				changed();
				return ret;
				}, GetPath(), changed);
		}
	}

	return GeLockShellFile();
}




