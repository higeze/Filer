#include "ShellZipFolder.h"

const std::wstring& CShellZipFolder::GetDispName() const
{
	return CShellFile::GetDispName();
}

const std::wstring& CShellZipFolder::GetDispNameWithoutExt() const
{
	return CShellFile::GetDispNameWithoutExt();
}

const std::wstring& CShellZipFolder::GetDispExt() const
{
	return CShellFile::GetDispExt();
}

//std::pair<std::shared_ptr<CIcon>, FileIconStatus> CShellZipFolder::GetIcon(std::function<void(CShellFile*)>& changedAction)
//{
//	return CShellFile::GetIcon(changedAction);
//}

std::pair<FileTimes, FileTimeStatus> CShellZipFolder::GetFileTimes(const FileTimeArgs& args, std::function<void()> changed)
{
	return CShellFile::GetFileTimes(args);
}

std::pair<ULARGE_INTEGER, FileSizeStatus> CShellZipFolder::GetSize(const FileSizeArgs& args, std::function<void()> changed)
{
	return CShellFile::GetSize(args);
}

void CShellZipFolder::SetFileNameWithoutExt(const std::wstring& wstrNameWoExt, HWND hWnd)
{
	return CShellFile::SetFileNameWithoutExt(wstrNameWoExt, hWnd);
}

void CShellZipFolder::SetExt(const std::wstring& wstrExt, HWND hWnd)
{
	return CShellFile::SetExt(wstrExt, hWnd);
}



