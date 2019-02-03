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

std::pair<std::shared_ptr<CIcon>, FileIconStatus> CShellZipFolder::GetIcon()
{
	return CShellFile::GetIcon();
}

std::pair<FILETIME, FileTimeStatus> CShellZipFolder::GetLastWriteTime(std::shared_ptr<FileTimeArgs>& spArgs)
{
	return CShellFile::GetLastWriteTime(spArgs);
}

std::pair<ULARGE_INTEGER, FileSizeStatus> CShellZipFolder::GetSize(std::shared_ptr<FileSizeArgs>& spArgs)
{
	return CShellFile::GetSize(spArgs);
}

