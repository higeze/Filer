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


CKnownFolderManager CShellFolder::s_knownFolderManager;

CShellFolder::CShellFolder() :CShellFile(), m_pShellFolder()
{
	::SHGetDesktopFolder(&m_pShellFolder);
	::SHGetDesktopFolder(&m_pParentShellFolder);
	::SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, m_childIdl.ptrptr());
	::SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, m_parentIdl.ptrptr());
	::SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, m_absoluteIdl.ptrptr());
}

CShellFolder::CShellFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl, CComPtr<IShellFolder> pShellFolder)
	:CShellFile(pParentShellFolder, parentIdl, childIdl), m_pShellFolder(pShellFolder) {}


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

std::wstring CShellFolder::GetFileNameWithoutExt()
{
	if (m_wstrFileNameWithoutExt.empty()) {
		if(::PathIsDirectory(GetPath().c_str()) || s_knownFolderManager.IsKnownFolder(m_absoluteIdl) || std::wstring(::PathFindFileName(GetPath().c_str()))[0] == L':'){
			STRRET strret;
			m_pParentShellFolder->GetDisplayNameOf(m_childIdl.ptr(), SHGDN_NORMAL, &strret);
			m_wstrFileNameWithoutExt = m_childIdl.STRRET2WSTR(strret);
		} else {
			return CShellFile::GetFileNameWithoutExt();
		}
	}
	return m_wstrFileNameWithoutExt;
}

std::wstring CShellFolder::GetFileName()
{
	if (m_wstrFileName.empty()) {
		if (::PathIsDirectory(GetPath().c_str()) || s_knownFolderManager.IsKnownFolder(m_absoluteIdl) || std::wstring(::PathFindFileName(GetPath().c_str()))[0] == L':') {
			STRRET strret;
			m_pParentShellFolder->GetDisplayNameOf(m_childIdl.ptr(), SHGDN_NORMAL, &strret);
			m_wstrFileName = m_childIdl.STRRET2WSTR(strret);
		} else {
			return CShellFile::GetFileName();
		}
	}
	return m_wstrFileName;
}

std::wstring CShellFolder::GetExt()
{
	if (m_wstrExt.empty()) {
		if (::PathIsDirectory(GetPath().c_str()) || s_knownFolderManager.IsKnownFolder(m_absoluteIdl) || std::wstring(::PathFindFileName(GetPath().c_str()))[0] == L':') {
			m_wstrExt = L"";
		} else {
			return CShellFile::GetExt();
		}
	}
	return m_wstrExt;
}


std::shared_ptr<CShellFolder> CShellFolder::GetParent()
{
	CIDL parentIDL = m_absoluteIdl.CloneParentIDL();
	CIDL grandParentIDL = parentIDL.CloneParentIDL();
	CComPtr<IShellFolder> pParentFolder;
	CComPtr<IShellFolder> pGrandParentFolder;
	//Desktop IShellFolder
	CComPtr<IShellFolder> pDesktopShellFolder;
	::SHGetDesktopFolder(&pDesktopShellFolder);
	pDesktopShellFolder->BindToObject(parentIDL.ptr(), 0, IID_IShellFolder, (void**)&pParentFolder);
	pDesktopShellFolder->BindToObject(grandParentIDL.ptr(), 0, IID_IShellFolder, (void**)&pGrandParentFolder);
	if (!pParentFolder) {
		pParentFolder = pDesktopShellFolder;
	}
	if (!pGrandParentFolder) {
		pGrandParentFolder = pDesktopShellFolder;
	}
	return std::make_shared<CShellFolder>(pGrandParentFolder, grandParentIDL, parentIDL.CloneLastID(), pParentFolder);

}

std::shared_ptr<CShellFolder> CShellFolder::Clone()const
{
	return std::make_shared<CShellFolder>(m_pParentShellFolder, m_parentIdl, m_childIdl, m_pShellFolder);
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
		if (SUCCEEDED(m_pShellFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &enumIdl)) && enumIdl) {
			CIDL nextIdl;
			ULONG ulRet(0);
			//while (1) {
			//	if (future.wait_for(std::chrono::milliseconds(1)) != std::future_status::timeout) {
			//		std::wcout << "::GetFolderSize canceled" << std::endl;
			//		return false;
			//	}
			//	::Sleep(1000);
			//}

			while (SUCCEEDED(enumIdl->Next(1, nextIdl.ptrptr(), &ulRet))) {
				if (future.wait_for(std::chrono::milliseconds(1)) != std::future_status::timeout) {
					std::wcout << "::GetFolderSize canceled" << std::endl;
					return false;
				}
				if (!nextIdl) { break; }

				auto spFile(CreateShExFileFolder(nextIdl));
				if (auto spFolder = std::dynamic_pointer_cast<CShellFolder>(spFile)) {
					if (spFolder->GetFolderSize(childSize, future)) {
						size.QuadPart += childSize.QuadPart;
					} else {
						return false;
					}
				} else {
					if (spFile->GetFileSize(childSize)) {
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

std::shared_ptr<CShellFile> CShellFolder::CreateShExFileFolder(CIDL& childIdl)
{
	CComPtr<IShellFolder> pFolder;
	CComPtr<IEnumIDList> enumIdl;
	if (SUCCEEDED(m_pShellFolder->BindToObject(childIdl.ptr(), 0, IID_IShellFolder, (void**)&pFolder)) &&
		SUCCEEDED(pFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &enumIdl))) {
		return std::make_shared<CShellFolder>(m_pShellFolder, m_absoluteIdl, childIdl, pFolder);
	} else {
		return std::make_shared<CShellFile>(m_pShellFolder, m_absoluteIdl, childIdl);
	}
}


//static
std::shared_ptr<CShellFile> CShellFolder::CreateShExFileFolder(const std::wstring& path)
{
	CShellFolder desktop;
	CIDL relativeIdl;

	ULONG         chEaten;
	ULONG         dwAttributes;
	HRESULT hr = desktop.GetShellFolderPtr()->ParseDisplayName(
		NULL,
		NULL,
		const_cast<LPWSTR>(path.c_str()),
		&chEaten,
		relativeIdl.ptrptr(),
		&dwAttributes);
	if (FAILED(hr)) { throw std::exception("CShellFolder::CreateShExFileFolder"); }

	return desktop.CreateShExFileFolder(relativeIdl);
}





