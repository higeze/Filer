#include "ShellZipFolder.h"

CShellZipFolder::CShellZipFolder(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl)
	:CShellFolder(pParentShellFolder, parentIdl, childIdl){}


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

std::pair<FileTimes, FileTimeStatus> CShellZipFolder::GetFileTimes(const std::shared_ptr<FileTimeArgs>& spArgs, std::function<void()> changed)
{
	return CShellFile::GetFileTimes(spArgs);
}

std::pair<ULARGE_INTEGER, FileSizeStatus> CShellZipFolder::GetSize(const std::shared_ptr<FileSizeArgs>& spArgs, std::function<void()> changed)
{
	return CShellFile::GetSize(spArgs);
}

void CShellZipFolder::SetFileNameWithoutExt(const std::wstring& wstrNameWoExt, HWND hWnd)
{
	return CShellFile::SetFileNameWithoutExt(wstrNameWoExt, hWnd);
}

void CShellZipFolder::SetExt(const std::wstring& wstrExt, HWND hWnd)
{
	return CShellFile::SetExt(wstrExt, hWnd);
}


