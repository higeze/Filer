#pragma once
#include "MyString.h"
#include "MyIcon.h"
#include "IDLPtr.h"

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

	virtual ~CShellFile(){}
	
	CComPtr<IShellFolder>& GetParentShellFolderPtr(){return m_parentFolder;}

	CIDLPtr& GetAbsolutePidl(){return m_absolutePidl;}
	CComPtr<IShellFolder> GetParentShellFolder()const{return m_parentFolder;}

	std::wstring GetPath()const{return m_wstrPath;}
	
	std::wstring GetName()const{return m_wstrName;}
	void SetName(const std::wstring& wstrName){m_wstrName = wstrName;}
	
	std::wstring GetExt()const{return m_wstrExt;}
	void SetExt(const std::wstring& wstrExt){m_wstrExt = wstrExt;}
	
	std::wstring GetCreationTime()const{return m_wstrCreationTime;}
	void SetCreationTime(const std::wstring& wstrCreationTime){m_wstrCreationTime = wstrCreationTime;}

	std::wstring GetLastAccessTime()const{return m_wstrLastAccessTime;}
	void SetLastAccessTime(const std::wstring& wstrLastAccessTime){m_wstrLastAccessTime = wstrLastAccessTime;}

	std::wstring GetLastWriteTime()const{return m_wstrLastWriteTime;}
	void SetLastWriteTime(const std::wstring& wstrLastWriteTime){m_wstrLastWriteTime = wstrLastWriteTime;}

	std::wstring GetSizeString()const{return m_wstrSize;}

	ULARGE_INTEGER GetSize()const{return m_size;}

	CIcon GetIcon(){
		if((HICON)m_icon==NULL){
			SHFILEINFO sfi={0};
			::SHGetFileInfo((LPCTSTR)(LPITEMIDLIST)m_absolutePidl,0,&sfi,sizeof(SHFILEINFO),SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);
			m_icon=CIcon(sfi.hIcon);

			//CComPtr<IExtractIcon> pEI;
			//UINT reserved = 0;
			//LPITEMIDLIST lastPIDL = m_absolutePidl.FindLastID();
			//HRESULT hr = m_parentFolder->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST*)(&lastPIDL), IID_IExtractIcon, &reserved, (LPVOID*)&pEI);
			//if(SUCCEEDED(hr)){
			//	std::wstring buff;
			//	::GetBuffer(buff, MAX_PATH);
			//	UINT flags =0;
			//	int index = 0;
			//	hr = pEI->GetIconLocation(GIL_FORSHELL, (PWSTR)buff.data(), MAX_PATH, &index, &flags);
			//	HICON smallIcon = NULL;
			//	pEI->Extract(buff.data(), index, NULL, &smallIcon, MAKELONG(32, 16));
			//	m_icon = CIcon(smallIcon);
			//}
		}
		return m_icon;
	}

	UINT CShellFile::GetAttributes()const{return m_ulAttributes;}

};