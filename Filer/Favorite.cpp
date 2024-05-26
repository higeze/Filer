//#include "stdafx.h"
#include "Favorite.h"
#include "ShellFile.h"
#include "ShellFileFactory.h"
#include "Debug.h"
#include "MyString.h"
#include "ThreadPool.h"

CFavorite::CFavorite()
	:m_Path(), m_ShortName(), m_spFile(nullptr), m_spCancel(std::make_shared<bool>(false)){}

CFavorite::CFavorite(std::wstring path, std::wstring shortName)
	:m_Path(path), m_ShortName(shortName), m_spFile(nullptr),m_spCancel(std::make_shared<bool>(false)){}

CFavorite::~CFavorite()
{
	if (m_futureFile.valid() && m_futureFile.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
		*m_spCancel = true;
		m_futureFile.get();
	}
}


std::shared_ptr<CShellFile>& CFavorite::GetShellFile(const std::function<void()>& changed)
{
	if (!GeLockShellFile()) {
		//Thread started, not finished
		if (m_futureFile.valid() && m_futureFile.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
			//Do nothing
		//Thread finished
		} else if (m_futureFile.valid() && m_futureFile.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
			SetLockShellFile(m_futureFile.get());
		//Thread not started
		} else {
			auto fun = [](std::shared_ptr<bool> spCancel, const std::wstring& path, const std::function<void()>& changed)->std::shared_ptr<CShellFile>
			{
				LOG_SCOPED_TIMER_1("CreateShellFilerPtr:" + wstr2str(path));
				auto ret = CShellFileFactory::GetInstance()->CreateShellFilePtr(path);
				if (!(*spCancel)) {
					changed();
				}
				return ret;
			};
			m_futureFile = CThreadPool::GetInstance()->enqueue(
				FILE_LINE_FUNC,
				0,
				fun,
				m_spCancel, 
				GetPath(),
				changed);
		}
	}

	return GeLockShellFile();
}




