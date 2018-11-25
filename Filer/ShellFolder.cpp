#include "ShellFolder.h"
#include "ShellZipFolder.h"
#include "KnownFolder.h"
#include "DriveFolder.h"
#include "FileIconCache.h"
#include "ConsoleTimer.h"
#include "ApplicationProperty.h"

extern std::shared_ptr<CApplicationProperty> g_spApplicationProperty;

CShellFolder::CShellFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl, CComPtr<IShellFolder> pShellFolder)
	:CShellFile(pParentShellFolder, parentIdl, childIdl), m_pShellFolder(pShellFolder) {}


CShellFolder::~CShellFolder()
{
	//std::wcout << L"~CShellFolder " + GetFileName() << std::endl;
	try {
		if (m_pSizeThread && m_pSizeThread->joinable()) {
			BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::~CShellFolder Set Cancel : " + GetPath();
			m_cancelSize.store(true);
			m_pSizeThread->join();
		}
		SignalFileSizeChanged.disconnect_all_slots();
	} catch (...) {
		BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::~CShellFile Exception Thread detached";
		if (m_pSizeThread) m_pSizeThread->detach();
	}
}

CComPtr<IShellFolder> CShellFolder::GetShellFolderPtr()
{
	if (!m_pShellFolder) {
		if (FAILED(m_pParentShellFolder->BindToObject(m_childIdl.ptr(), 0, IID_IShellFolder, (void**)&m_pShellFolder))){
			throw std::exception("CShellFolder::GetShellFolderPtr");
		}
	}
	return m_pShellFolder;
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

std::pair<std::shared_ptr<CIcon>, FileIconStatus> CShellFolder::GetIcon()
{
	if (GetLockIcon().second == FileIconStatus::None) {
		SetLockIcon(std::make_pair(CFileIconCache::GetInstance()->GetFolderIcon(), FileIconStatus::Available));
	}
	return GetLockIcon();
}


std::wstring CShellFolder::GetFileNameWithoutExt()
{
	if (m_wstrFileNameWithoutExt.empty()) {
		if(::PathIsDirectory(GetPath().c_str()) || GetPath()[0] == L':'){
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
		if (::PathIsDirectory(GetPath().c_str()) || GetPath()[0] == L':') {
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
		if (::PathIsDirectory(GetPath().c_str()) ||  std::wstring(::PathFindFileName(GetPath().c_str()))[0] == L':') {
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
		if (::PathIsNetworkPath(GetPath().c_str())){
			SetLockSize(std::make_pair(ULARGE_INTEGER{ 0 }, FileSizeStatus::Unavailable));
		} else {
			SetLockSize(std::make_pair(ULARGE_INTEGER{ 0 }, FileSizeStatus::Calculating));

			if (!m_pSizeThread) {
				m_pSizeThread.reset(new std::thread([this]()->void {
					try {
						boost::timer tim;

						ULARGE_INTEGER size = { 0 };
						if (GetFolderSize(size, m_cancelSize, tim)) {
							SetLockSize(std::make_pair(size, FileSizeStatus::Available));
						} else {
							SetLockSize(std::make_pair(size, FileSizeStatus::Unavailable));
						}
						SignalFileSizeChanged(this);
					} catch (...) {
						BOOST_LOG_TRIVIAL(trace) << L"CShellFile::GetSize Exception at size thread";
					}
				}));
			} else {
				OutputDebugString(L"Already run");
			}
		}
		break;
	case FileSizeStatus::Available:
	case FileSizeStatus::Unavailable:
	case FileSizeStatus::Calculating:
		break;
	}
	return GetLockSize();
}

bool CShellFolder::GetFolderSize(ULARGE_INTEGER& size, std::atomic<bool>& cancel, boost::timer& tim)
{
	if (cancel.load()) {
		BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::GetFolderSize Canceled at top : " + GetPath();
		return false;
	} else if (tim.elapsed() > 0.5) {
		BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::GetFolderSize Timer elapsed at top : " + GetPath();
		return false;
	}
	try {
		//Enumerate child IDL
		size.QuadPart = 0;
		ULARGE_INTEGER childSize = { 0 };

		CComPtr<IEnumIDList> enumIdl;
		if (SUCCEEDED(GetShellFolderPtr()->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &enumIdl)) && enumIdl) {
			CIDL nextIdl;
			ULONG ulRet(0);
			while (SUCCEEDED(enumIdl->Next(1, nextIdl.ptrptr(), &ulRet))) {
				if (cancel.load()) {
					BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::GetFolderSize Canceled in while : " + GetPath();
					return false;
				} else if (tim.elapsed() > 0.5) {
					BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::GetFolderSize Timer elapsed in while : " + GetPath();
					return false;
				}
				if (!nextIdl) { break; }

				auto spFile(CreateShExFileFolder(nextIdl));
				if (auto spFolder = std::dynamic_pointer_cast<CShellFolder>(spFile)) {
					if (spFolder->GetFolderSize(childSize, cancel, tim)) {
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
		BOOST_LOG_TRIVIAL(trace) << "Exception CShellFolder::GetFolderSize " << GetPath();
		return false;
	}
	return true;

}


void CShellFolder::ResetSize()
{
	if (m_pSizeThread && m_pSizeThread->joinable()) {
		BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::ResetIcon Icon thread canceled";
		//m_sizePromise.set_value();
		m_cancelSize.store(true);
		m_pSizeThread->join();
	}
	m_pSizeThread.reset();
	m_cancelSize.store(false);
	SetLockSize(std::make_pair(ULARGE_INTEGER{ 0 }, FileSizeStatus::None));
}

std::shared_ptr<CShellFile> CShellFolder::CreateShExFileFolder(CIDL& childIdl)
{
	boost::timer tim;
//	CONSOLETIMER_IF(g_spApplicationProperty->m_bDebug, L"CreateShExFileFolder")

	CComPtr<IShellFolder> pFolder;
	CComPtr<IEnumIDList> enumIdl;	
	STRRET strret;
	GetShellFolderPtr()->GetDisplayNameOf(childIdl.ptr(), SHGDN_FORPARSING, &strret);
	std::wstring path =  childIdl.STRRET2WSTR(strret);
	std::wstring ext = ::PathFindExtension(path.c_str());

	if (!childIdl) {
		return CKnownFolderManager::GetInstance()->GetDesktopFolder();
	} else if (auto spKnownFolder = CKnownFolderManager::GetInstance()->GetKnownFolderByPath(path)) {
		return spKnownFolder;
	} else if (auto spDriveFolder = CDriveManager::GetInstance()->GetDriveFolderByPath(path)) {
		return spDriveFolder;
	} else if (boost::iequals(ext, ".zip")) {
		return std::make_shared<CShellZipFolder>(GetShellFolderPtr(), m_absoluteIdl, childIdl);
	} else if (
		//::PathIsDirectory(path.c_str()) ||
		(SUCCEEDED(GetShellFolderPtr()->BindToObject(childIdl.ptr(), 0, IID_IShellFolder, (void**)&pFolder)) &&
		SUCCEEDED(pFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FOLDERS, &enumIdl)))) {
		return std::make_shared<CShellFolder>(GetShellFolderPtr(), m_absoluteIdl, childIdl, pFolder);
	} else {
		return std::make_shared<CShellFile>(GetShellFolderPtr(), m_absoluteIdl, childIdl);
	}

	if (tim.elapsed() > 0.1) {
		BOOST_LOG_TRIVIAL(trace) << L"CShellFolder::CreateShExFileFolder Over0.1 "<< path;
	}
}


//static
std::shared_ptr<CShellFile> CShellFolder::CreateShExFileFolder(const std::wstring& path)
{
	auto desktop(CKnownFolderManager::GetInstance()->GetDesktopFolder());
	CIDL relativeIdl;

	ULONG         chEaten;
	ULONG         dwAttributes;
	HRESULT hr = desktop->GetShellFolderPtr()->ParseDisplayName(
		NULL,
		NULL,
		const_cast<LPWSTR>(path.c_str()),
		&chEaten,
		relativeIdl.ptrptr(),
		&dwAttributes);
	if (FAILED(hr)) {
		return desktop;
	}

	return desktop->CreateShExFileFolder(relativeIdl);
}





