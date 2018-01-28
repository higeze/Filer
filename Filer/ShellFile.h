#pragma once
#include "MyString.h"
#include "MyIcon.h"
#include "IDLPtr.h"

class CShellFolder;

tstring FileTime2String(FILETIME *pFileTime);
tstring Size2String(ULONGLONG size);
std::wstring ConvertCommaSeparatedNumber(ULONGLONG n, int separate_digit = 3);

class CShellFile
{
protected:
	CComPtr<IShellFolder> m_parentFolder;
	CIDLPtr m_absolutePidl;

	std::wstring m_wstrPath;
	std::wstring m_wstrName;
	std::wstring m_wstrExt;
	std::wstring m_wstrType;
	std::wstring m_wstrCreationTime;
	std::wstring m_wstrLastAccessTime;
	std::wstring m_wstrLastWriteTime;
	std::wstring m_wstrSize;

	ULARGE_INTEGER m_size;
	DWORD  m_fileAttributes;
	CIcon m_icon;
	ULONG m_ulAttributes;

public:
	CShellFile();
	CShellFile(CComPtr<IShellFolder> pfolder, CIDLPtr absolutePidl);
	CShellFile(const std::wstring& path);

	virtual ~CShellFile(){}
	
	//Getter 
	CComPtr<IShellFolder>& GetParentShellFolderPtr(){return m_parentFolder;}
	CIDLPtr& GetAbsolutePidl(){return m_absolutePidl;}

	//Lazy Evaluation Getter
	std::wstring GetPath();
	std::wstring GetName();
	std::wstring GetExt();
	std::wstring GetTypeName();
	
	std::wstring GetCreationTime();
	std::wstring GetLastAccessTime();
	std::wstring GetLastWriteTime();
	std::wstring GetSizeString();
	ULARGE_INTEGER GetSize();
	CIcon GetIcon();
	UINT GetAttributes();
	//
	bool IsShellFolder()const;
	std::shared_ptr<CShellFolder> GetShellFolder()const;
private:
	void UpdateWIN32_FIND_DATA();


};