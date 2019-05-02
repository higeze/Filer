#include "ShellZipFolder.h"

CShellZipFolder::CShellZipFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl)
	:CShellFolder(pParentShellFolder, parentIdl, childIdl){}

std::wstring CShellZipFolder::GetFileNameWithoutExt()
{
	return CShellFile::GetFileNameWithoutExt();
}

std::wstring CShellZipFolder::GetFileName()
{
	return CShellFile::GetFileName();
}

std::wstring CShellZipFolder::GetExt()
{
	return CShellFile::GetExt();
}

//std::pair<std::shared_ptr<CIcon>, FileIconStatus> CShellZipFolder::GetIcon(std::function<void(CShellFile*)>& changedAction)
//{
//	return CShellFile::GetIcon(changedAction);
//}

std::pair<FILETIME, FileTimeStatus> CShellZipFolder::GetLastWriteTime(std::shared_ptr<FileTimeArgs>& spArgs, std::function<void()> changed)
{
	return CShellFile::GetLastWriteTime(spArgs);
}

std::pair<ULARGE_INTEGER, FileSizeStatus> CShellZipFolder::GetSize(std::shared_ptr<FileSizeArgs>& spArgs, std::function<void()> changed)
{
	return CShellFile::GetSize(spArgs);
}

//bool CShellZipFolder::GetFolderSize(ULARGE_INTEGER& size, std::atomic<bool>& cancel, boost::timer& tim, int limit)
//{
//	if (cancel.load()) {
//		BOOST_LOG_TRIVIAL(trace) << L"CShellZipFolder::GetFolderSize Canceled at top : " + GetPath();
//		return false;
//	} else if (limit > 0 && tim.elapsed() > limit / 1000.0) {
//		BOOST_LOG_TRIVIAL(trace) << L"CShellZipFolder::GetFolderSize Timer elapsed at top : " + GetPath();
//		return false;
//	}
//	if (CShellFile::GetFileSize(size)) {
//		return true;
//	} else {
//		return false;
//	}
//}
//
//bool CShellZipFolder::GetFolderLastWriteTime(FILETIME& time, std::atomic<bool>& cancel, boost::timer& tim, int limit, bool ignoreFolderTime)
//{
//	if (cancel.load()) {
//		BOOST_LOG_TRIVIAL(trace) << L"CShellZipFolder::GetFolderLastWriteTime Canceled at top : " + GetPath();
//		return true;
//	} else if (limit > 0 && tim.elapsed() > limit / 1000.0) {
//		BOOST_LOG_TRIVIAL(trace) << L"CShellZipFolder::GetFolderLastWriteTime Timer elapsed at top : " + GetPath();
//		return true;
//	}
//	if (!CShellFile::GetFileLastWriteTime(time)) {
//		time = FILETIME{ 0 };
//	}
//	return true;
//}

void CShellZipFolder::SetFileNameWithoutExt(const std::wstring& wstrNameWoExt)
{
	return CShellFile::SetFileNameWithoutExt(wstrNameWoExt);
}

void CShellZipFolder::SetExt(const std::wstring& wstrExt)
{
	return CShellFile::SetExt(wstrExt);
}



