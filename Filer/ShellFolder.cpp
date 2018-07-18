#include "ShellFolder.h"

//std::shared_ptr<CShellFolder> CShellFolder::CreateShellFolderFromPath(const std::wstring& path)
//{
//	ULONG         chEaten;
//	ULONG         dwAttributes;
//
//	CComPtr<IShellFolder> pDesktop;
//	::SHGetDesktopFolder(&pDesktop);
//
//	CIDLPtr pIDL;
//	HRESULT hr = NULL;
//	if (path == L"") {
//		hr = ::SHGetSpecialFolderLocation(NULL, CSIDL_PROFILE, &pIDL);
//	}
//	else {
//		hr = pDesktop->ParseDisplayName(
//			NULL,
//			NULL,
//			const_cast<LPWSTR>(path.c_str()),
//			&chEaten,
//			&pIDL,
//			&dwAttributes);
//	}
//	if (SUCCEEDED(hr))
//	{
//		CComPtr<IShellFolder> pFolder, pParentFolder;
//		::SHBindToObject(pDesktop, pIDL, 0, IID_IShellFolder, (void**)&pFolder);
//		::SHBindToObject(pDesktop, pIDL.GetPreviousIDLPtr(), 0, IID_IShellFolder, (void**)&pParentFolder);
//
//		if (!pFolder) {
//			pFolder = pDesktop;
//		}
//		if (!pParentFolder) {
//			pParentFolder = pDesktop;
//		}
//		return std::make_shared<CShellFolder>(pFolder, pParentFolder, pIDL);
//	}
//	else {
//		return std::shared_ptr<CShellFolder>();
//	}
//}

CShellFolder::CShellFolder() :CShellFile(), m_folder()
{
	::SHGetDesktopFolder(&m_folder);
}

CShellFolder::CShellFolder(CComPtr<IShellFolder> pFolder, CComPtr<IShellFolder> pParentFolder, CIDLPtr absolutePidl)
	:CShellFile(pParentFolder, absolutePidl), m_folder(pFolder) {}


CShellFolder::~CShellFolder()
{
	//std::wcout << L"~CShellFolder " + GetName() << std::endl;
	try {
		if (m_pSizeThread && m_pSizeThread->joinable()) {
			std::wcout << L"CShellFolder::~CShellFolder " + GetFileNameWithoutExt() + L" Size thread canceled" << std::endl;
			m_sizePromise.set_value();
			m_pSizeThread->join();
		}
		SignalFileSizeChanged.disconnect_all_slots();
	} catch (...) {
		std::wcout << L"CShellFolder::~CShellFile Exception" << std::endl;
		if (m_pSizeThread) m_pSizeThread->detach();
	}
}

std::pair<ULARGE_INTEGER, FileSizeStatus> CShellFolder::GetLockSize()
{
	std::lock_guard<std::mutex> lock(m_mtxSize);
	return m_size;
}

void CShellFolder::SetLockSize(std::pair<ULARGE_INTEGER, FileSizeStatus>& size)
{
	std::lock_guard<std::mutex> lock(m_mtxSize);
	m_size = size;
}

CShellFolder::CShellFolder(const std::wstring& path)
	:CShellFile(path), m_folder()
{
	if (!m_absolutePidl.m_pIDL || !m_parentFolder) { m_folder = nullptr; return; }
	CComPtr<IShellFolder> pDesktop;
	::SHGetDesktopFolder(&pDesktop);

	HRESULT hr = ::SHBindToObject(pDesktop, m_absolutePidl, 0, IID_IShellFolder, (void**)&m_folder);
	if (FAILED(hr)) { m_folder = nullptr; }
}

std::shared_ptr<CShellFolder> CShellFolder::GetParent()
{
	CIDLPtr parentIDL = this->GetAbsolutePidl().GetPreviousIDLPtr();
	CIDLPtr grandParentIDL = parentIDL.GetPreviousIDLPtr();
	CComPtr<IShellFolder> pGrandParentFolder;
	//Desktop IShellFolder
	CComPtr<IShellFolder> pDesktopShellFolder;
	::SHGetDesktopFolder(&pDesktopShellFolder);
	pDesktopShellFolder->BindToObject(grandParentIDL, 0, IID_IShellFolder, (void**)&pGrandParentFolder);

	if (!pGrandParentFolder) {
		pGrandParentFolder = pDesktopShellFolder;
	}
	return std::make_shared<CShellFolder>(this->GetParentShellFolderPtr(), pGrandParentFolder, parentIDL);

}

std::shared_ptr<CShellFolder> CShellFolder::Clone()const
{
	return std::make_shared<CShellFolder>(m_folder, m_parentFolder, m_absolutePidl);
}

std::pair<ULARGE_INTEGER, FileSizeStatus> CShellFolder::GetSize()
{
	switch (GetLockSize().second) {
	case FileSizeStatus::None:
		SetLockSize(std::make_pair(ULARGE_INTEGER{ 0 }, FileSizeStatus::Calculating));
		if (!m_pSizeThread) {
			//auto spFile = shared_from_this();
			//std::weak_ptr<CShellFile> wpFile(spFile);

			m_sizeFuture = m_sizePromise.get_future();
			m_pSizeThread.reset(new std::thread([this]()->void {
				try {
					//if (auto sp = wpFile.lock()) {
						ULARGE_INTEGER size = { 0 };
						//if (auto spFolder = sp->CastShellFolder()) {
							if (GetFolderSize(size, m_sizeFuture)) {
								SetLockSize(std::make_pair(size, FileSizeStatus::Available));
							} else {
								SetLockSize(std::make_pair(size, FileSizeStatus::Unavailable));
							}
							SignalFileSizeChanged(this);
						//}
					//}

				} catch (...) {
					std::wcout << L"CShellFile::GetSize Exception at size thread" << std::endl;
				}
			}));
		} else {
			OutputDebugString(L"Already run");
		}
		break;
	case FileSizeStatus::Available:
	case FileSizeStatus::Unavailable:
	case FileSizeStatus::Calculating:
		break;
	}
	return GetLockSize();
}

bool CShellFolder::GetFolderSize(ULARGE_INTEGER& size, std::shared_future<void> future)
{
	try {
		//Enumerate child IDL
		size.QuadPart = 0;
		ULARGE_INTEGER childSize = { 0 };

		CComPtr<IEnumIDList> enumIdl;
		if (SUCCEEDED(m_folder->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &enumIdl)) && enumIdl) {
			CIDLPtr nextIdl;
			ULONG ulRet(0);
			//while (1) {
			//	if (future.wait_for(std::chrono::milliseconds(1)) != std::future_status::timeout) {
			//		std::wcout << "::GetFolderSize canceled" << std::endl;
			//		return false;
			//	}
			//	::Sleep(1000);
			//}

			while (SUCCEEDED(enumIdl->Next(1, &nextIdl, &ulRet))) {
				if (future.wait_for(std::chrono::milliseconds(1)) != std::future_status::timeout) {
					std::wcout << "::GetFolderSize canceled" << std::endl;
					return false;
				}
				if (!nextIdl) { break; }

				auto spFile(::CreateShExFileFolder(GetShellFolderPtr(), GetAbsolutePidl() + nextIdl));
				if (auto spFolder = std::dynamic_pointer_cast<CShellFolder>(spFile)) {
					if (spFolder->GetFolderSize(childSize, future)) {
						size.QuadPart += childSize.QuadPart;
					} else {
						return false;
					}
				} else {
					if (spFile->GetFileSize(childSize, future)) {
						size.QuadPart += childSize.QuadPart;
					} else {
						return false;
					}
				}
				childSize.QuadPart = 0;
				nextIdl.Clear();
			}
		}
	} catch (...) {
		std::wcout << "::GetFolderSize Exception" << std::endl;
		return false;
	}
	return true;

}


void CShellFolder::ResetSize()
{
	if (m_pSizeThread && m_pSizeThread->joinable()) {
		std::wcout << L"CShellFolder::ResetIcon Icon thread canceled" << std::endl;
		m_sizePromise.set_value();
		m_pSizeThread->join();
	}
	m_pSizeThread.reset();
	m_sizePromise = std::promise<void>();
	m_sizeFuture = std::future<void>();
	SetLockSize(std::make_pair(ULARGE_INTEGER{ 0 }, FileSizeStatus::None));
}



