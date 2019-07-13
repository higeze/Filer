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

std::pair<FileTimes, FileTimeStatus> CShellZipFolder::GetFileTimes(std::shared_ptr<FileTimeArgs>& spArgs, std::function<void()> changed)
{
	return CShellFile::GetFileTimes(spArgs);
}

std::pair<ULARGE_INTEGER, FileSizeStatus> CShellZipFolder::GetSize(std::shared_ptr<FileSizeArgs>& spArgs, std::function<void()> changed)
{
	return CShellFile::GetSize(spArgs);
}

void CShellZipFolder::SetFileNameWithoutExt(const std::wstring& wstrNameWoExt)
{
	return CShellFile::SetFileNameWithoutExt(wstrNameWoExt);
}

void CShellZipFolder::SetExt(const std::wstring& wstrExt)
{
	return CShellFile::SetExt(wstrExt);
}



