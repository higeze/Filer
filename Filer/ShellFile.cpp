#include "ShellFile.h"

#include "MyIcon.h"
#include "MyString.h"
#include "MyCom.h"

std::wstring ConvertCommaSeparatedNumber(ULONGLONG n, int separate_digit)

{
    bool is_minus = n < 0;
    is_minus ? n *= -1 : 0;

    std::wstringstream ss;
    ss << n;
    std::wstring snum = ss.str();
    std::reverse(snum.begin(), snum.end());
    std::wstringstream  ss_csnum;
    for(int i = 0, len = snum.length(); i <= len;) {
        ss_csnum << snum.substr(i, separate_digit);
        if((i += separate_digit) >= len)
            break;
        ss_csnum << ',';
    }
    if(is_minus){
		ss_csnum << '-';
	}

    std::wstring cs_num = ss_csnum.str();
    std::reverse(cs_num.begin(), cs_num.end());
    return cs_num;
}

tstring FileTime2String(FILETIME* pFileTime)
{
	FILETIME ft;
	SYSTEMTIME st;

	FileTimeToLocalFileTime(pFileTime, &ft);
	FileTimeToSystemTime(&ft, &st);
	tstring str;
	wsprintf(GetBuffer(str,16),L"%04d/%02d/%02d %02d:%02d",
			st.wYear, st.wMonth, st.wDay,st.wHour, st.wMinute);
	ReleaseBuffer(str);
	return str;

}

tstring Size2String(ULONGLONG size)
{	
	return ConvertCommaSeparatedNumber(size);	
}

CShellFile::CShellFile():m_parentFolder(), m_absolutePidl(), m_icon()
{
	::SHGetDesktopFolder(&m_parentFolder);
	//ChildPild
	CIDLPtr childPidl = m_absolutePidl.GetLastIDLPtr();
	//Path
	STRRET strret;
	m_parentFolder->GetDisplayNameOf(childPidl, SHGDN_FORPARSING,&strret);
	m_wstrPath=childPidl.STRRET2WSTR(strret);
	//Name
	m_parentFolder->GetDisplayNameOf(childPidl, SHGDN_NORMAL,&strret);
	m_wstrName=childPidl.STRRET2WSTR(strret);
	//Type, Icon
	SHFILEINFO sfi={0};

//	::SHGetFileInfo((LPCTSTR)(LPITEMIDLIST)m_absolutePidl,0,&sfi,sizeof(SHFILEINFO),SHGFI_PIDL | SHGFI_TYPENAME | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);
	::SHGetFileInfo((LPCTSTR)(LPITEMIDLIST)m_absolutePidl,0,&sfi,sizeof(SHFILEINFO),SHGFI_PIDL | SHGFI_TYPENAME);

	m_wstrType=sfi.szTypeName;
//	m_icon=CIcon(sfi.hIcon);
	//Ext
	m_wstrExt=::PathFindExtension(m_wstrPath.c_str());
	//if(m_wstrExt.empty() && ::PathIsDirectory(m_wstrPath.c_str())){
	//	m_wstrExt = L"dir";
	//}
	//Win32_find_data
	WIN32_FIND_DATA wfd={0};
	if(!FAILED(SHGetDataFromIDList(m_parentFolder, childPidl,SHGDFIL_FINDDATA,&wfd,sizeof(WIN32_FIND_DATA)))){;
		m_wstrCreationTime=FileTime2String(&wfd.ftCreationTime);
		m_wstrLastAccessTime=FileTime2String(&wfd.ftLastAccessTime);
		m_wstrLastWriteTime=FileTime2String(&wfd.ftLastWriteTime);
		m_fileAttributes = wfd.dwFileAttributes;
		m_size.LowPart = wfd.nFileSizeLow;
		m_size.HighPart = wfd.nFileSizeHigh;
		if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
			m_wstrSize=L"dir";
		}else{
			m_wstrSize=Size2String(m_size.QuadPart);
		}
	}
	//Icon
	//::SHGetFileInfo((LPCTSTR)(LPITEMIDLIST)childPidl, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);
	//m_icon=CIcon(sfi.hIcon);

	//Attribute
	m_ulAttributes=SFGAO_CAPABILITYMASK | SFGAO_GHOSTED | SFGAO_LINK | SFGAO_SHARE | SFGAO_FOLDER | SFGAO_FILESYSTEM;
	LPCITEMIDLIST pIDL(childPidl);
	m_parentFolder->GetAttributesOf(1,&pIDL,&m_ulAttributes);
}

CShellFile::CShellFile(CComPtr<IShellFolder> pfolder, CIDLPtr absolutePidl):m_parentFolder(pfolder),m_absolutePidl(absolutePidl),m_icon()
{
	//ChildPild
	CIDLPtr childPidl = m_absolutePidl.GetLastIDLPtr();
	//Path
	STRRET strret;
	m_parentFolder->GetDisplayNameOf(childPidl, SHGDN_FORPARSING,&strret);
	m_wstrPath=childPidl.STRRET2WSTR(strret);
	//Name
	m_parentFolder->GetDisplayNameOf(childPidl, SHGDN_NORMAL,&strret);
	m_wstrName=childPidl.STRRET2WSTR(strret);
	//Type, Icon
	SHFILEINFO sfi={0};

//	::SHGetFileInfo((LPCTSTR)(LPITEMIDLIST)m_absolutePidl,0,&sfi,sizeof(SHFILEINFO),SHGFI_PIDL | SHGFI_TYPENAME | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);
	::SHGetFileInfo((LPCTSTR)(LPITEMIDLIST)m_absolutePidl,0,&sfi,sizeof(SHFILEINFO),SHGFI_PIDL | SHGFI_TYPENAME);

	m_wstrType=sfi.szTypeName;
//	m_icon=CIcon(sfi.hIcon);
	//Ext
	m_wstrExt=::PathFindExtension(m_wstrPath.c_str());
	//if(m_wstrExt.empty() && ::PathIsDirectory(m_wstrPath.c_str())){
	//	m_wstrExt = L"dir";
	//}
	//Win32_find_data
	WIN32_FIND_DATA wfd={0};
	if(!FAILED(SHGetDataFromIDList(m_parentFolder, childPidl,SHGDFIL_FINDDATA,&wfd,sizeof(WIN32_FIND_DATA)))){;
		m_wstrCreationTime=FileTime2String(&wfd.ftCreationTime);
		m_wstrLastAccessTime=FileTime2String(&wfd.ftLastAccessTime);
		m_wstrLastWriteTime=FileTime2String(&wfd.ftLastWriteTime);
		m_fileAttributes = wfd.dwFileAttributes;
		m_size.LowPart = wfd.nFileSizeLow;
		m_size.HighPart = wfd.nFileSizeHigh;
		if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
			m_wstrSize=L"dir";
		}else{
			m_wstrSize=Size2String(m_size.QuadPart);
		}
	}
	//Icon
	//::SHGetFileInfo((LPCTSTR)(LPITEMIDLIST)childPidl, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_ADDOVERLAYS);
	//m_icon=CIcon(sfi.hIcon);

	//Attribute
	m_ulAttributes=SFGAO_CAPABILITYMASK | SFGAO_GHOSTED | SFGAO_LINK | SFGAO_SHARE | SFGAO_FOLDER | SFGAO_FILESYSTEM;
	LPCITEMIDLIST pIDL(childPidl);
	m_parentFolder->GetAttributesOf(1,&pIDL,&m_ulAttributes);

	//Icon
//	m_nIcon=idl.GetIconIndex();
}

//int CShellFile::GetIconIndex()
//{return m_nIcon;}


