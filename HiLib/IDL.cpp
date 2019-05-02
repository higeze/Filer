#include "IDL.h"

#include "SingletonMalloc.h"
#include "MyCom.h"

std::wstring STRRET2WSTR(STRRET& strret, LPITEMIDLIST pidl)
{
	int nLength;
	LPSTR lpmstr;
	WCHAR wcRet[MAX_PATH] = { 0 };
	switch (strret.uType) {
	case STRRET_WSTR:
		return std::wstring(strret.pOleStr);
		break;
	case STRRET_OFFSET:
		lpmstr = (LPSTR)(((char*)pidl) + strret.uOffset);
		nLength = ::MultiByteToWideChar(CP_THREAD_ACP, 0, lpmstr, -1, NULL, 0);
		::MultiByteToWideChar(CP_THREAD_ACP, 0, lpmstr, -1, wcRet, nLength);
		break;
	case STRRET_CSTR:
		lpmstr = strret.cStr;
		nLength = ::MultiByteToWideChar(CP_THREAD_ACP, 0, lpmstr, -1, NULL, 0);
		::MultiByteToWideChar(CP_THREAD_ACP, 0, lpmstr, -1, wcRet, nLength);
		//::wcscpy_s(wcRet,wcslen(wcRet),(LPCWSTR)strret.cStr);
		break;
	default:
		break;
	}
	return std::wstring(wcRet);
}


//Static member functions
LPITEMIDLIST CIDL::CreateItemIdList(UINT uSize)
{
	LPITEMIDLIST pIdl;
	pIdl = (LPITEMIDLIST)CSingletonMalloc::GetInstance()->Alloc(uSize);
	if (pIdl != NULL)::memset(pIdl, 0, uSize);
	return pIdl;
}

LPITEMIDLIST CIDL::CopyItemIdList(LPITEMIDLIST pIdl)
{
	//int nSize=pIdl->mkid.cb;
	//LPITEMIDLIST pIdlNew=CreateItemIdList(nSize+sizeof(USHORT));
	//if(pIdlNew==NULL){return NULL;}
	//::memcpy(pIdlNew,pIdl,nSize);
	//::memset((LPBYTE)pIdlNew+nSize,0,sizeof(USHORT));
	//return pIdlNew;
	return ::ILCloneFull(pIdl);
}

PITEMID_CHILD CIDL::GetNextItemId(PITEMID_CHILD pIdl)
{
	UINT uSize(pIdl->mkid.cb);
	if (uSize == 0) { return NULL; }
	pIdl = (LPITEMIDLIST)(((LPBYTE)pIdl) + uSize);
	if (pIdl->mkid.cb == 0) {
		return NULL;
	} else {
		return pIdl;
	}
}

PITEMID_CHILD CIDL::GetLastItemId(LPITEMIDLIST pidl)
{
	//if(pIdl == nullptr){return nullptr;}
	//LPITEMIDLIST pIdl1 = pIdl;
	//LPITEMIDLIST pIdl2 = (LPITEMIDLIST)(((LPBYTE)pIdl)+pIdl->mkid.cb);
	//while(pIdl2->mkid.cb){
	//	pIdl1=pIdl2;
	//	pIdl2 = (LPITEMIDLIST)(((LPBYTE)pIdl1)+pIdl1->mkid.cb);
	//}
	//return CopyItemIdList(pIdl1);
	return ::ILClone(::ILFindLastID(pidl));
	//return ::ILFindLastID(pIdl);
}

LPITEMIDLIST CIDL::GetPreviousItemIdList(LPITEMIDLIST pIdl)
{
	if (pIdl == nullptr) { return nullptr; }
	UINT uFullSize(0);
	UINT uOneSize(0);
	LPITEMIDLIST pIdlTmp = pIdl;
	while (1) {
		uOneSize = pIdlTmp->mkid.cb;
		pIdlTmp = GetNextItemId(pIdlTmp);
		if (pIdlTmp == NULL) {
			break;
		} else {
			uFullSize += uOneSize;
		}
	};
	LPITEMIDLIST pIdlPre(NULL);
	if (uFullSize) {
		pIdlPre = CreateItemIdList(uFullSize + sizeof(USHORT));
		if (pIdlPre == NULL) { return NULL; }
		::memcpy(pIdlPre, pIdl, uFullSize);
		::memset((LPBYTE)pIdlPre + uFullSize, 0, sizeof(USHORT));
	}
	return pIdlPre;
}

UINT CIDL::GetItemIdListSize(LPITEMIDLIST pIdl)
{
	UINT uiRet = 0;
	if (pIdl == NULL) { return 0; }

	uiRet = sizeof(USHORT);

	do {
		uiRet += (int)pIdl->mkid.cb;
		pIdl = GetNextItemId(pIdl);
	} while (pIdl != NULL);
	return uiRet;
}

LPITEMIDLIST CIDL::ConcatItemIdList(LPITEMIDLIST pIdl1, LPITEMIDLIST pIdl2)
{
	//UINT uSize1(0),uSize2(0);
	//LPITEMIDLIST pIdlRet;
	//if(pIdl1==NULL && pIdl2==NULL){
	//	return NULL;
	//}else if(pIdl1==NULL){
	//	return CopyItemIdList(pIdl2);
	//}else if(pIdl2==NULL){
	//	return CopyItemIdList(pIdl1);
	//}else{
	//	uSize1=GetItemIdListSize(pIdl1);
	//	uSize2=GetItemIdListSize(pIdl2);
	//	pIdlRet=CreateItemIdList(uSize1+uSize2-sizeof(USHORT));
	//	if(pIdlRet){
	//		::memcpy(pIdlRet,pIdl1,uSize1-sizeof(USHORT));
	//		::memcpy((LPBYTE)(pIdlRet)+uSize1-sizeof(USHORT),pIdl2,uSize2-sizeof(USHORT));
	//		::memset((LPBYTE)(pIdlRet)+uSize1+uSize2-2*sizeof(USHORT),0,sizeof(USHORT));
	//	}else{
	//	
	//	}
	//}
	//return pIdlRet;
	return ::ILCombine(pIdl1, pIdl2);
}

LPITEMIDLIST CIDL::GetItemIdList(LPCWSTR lpwstrPath)
{
	if (*lpwstrPath == NULL)return NULL;

	HRESULT hRes;
	ULONG ulSize;
	ULONG ulAttributes;
	LPITEMIDLIST pIdl;
	LPSHELLFOLDER pDesktopFolder;

	if (::SHGetDesktopFolder(&pDesktopFolder) != NOERROR)return NULL;

	hRes = pDesktopFolder->ParseDisplayName(NULL, NULL, const_cast<LPWSTR>(lpwstrPath), &ulSize, &pIdl, &ulAttributes);
	pDesktopFolder->Release();

	if (hRes != NOERROR)pIdl = NULL;
	return pIdl;
}

std::wstring CIDL::STRRET2WSTR(STRRET& strret)const
{
	return ::STRRET2WSTR(strret, m_pIDL);
}

CIDL CIDL::operator + ( const CIDL& idl ) const
{
	if(!(*this) && !idl){
		return CIDL();
	}else if(!(*this)){
		return CIDL(::ILCloneFull(idl.ptr()));
	}else if(!idl){
		return CIDL(::ILCloneFull(m_pIDL));
	}
	return CIDL(::ILCombine(m_pIDL, idl.ptr()));
}

//CIDL CIDL::operator - (const CIDL& idl) const
//{
//
//	if (::ILIsParent(m_pIDL, idl.ptr(), false)) {
//		LPITEMIDLIST pidl = ::ILCloneFull(m_pIdl);
//		while(::ILIsEqual())
//		return CIDL();
//	} else if (!(*this)) {
//		return CIDL(::ILCloneFull(idl.ptr()));
//	} else if (!idl) {
//		return CIDL(::ILCloneFull(m_pIDL));
//	}
//	return CIDL(::ILCombine(m_pIDL, idl.ptr()));
//}


//LPITEMIDLIST CIDL::GetItemIdList(LPCWSTR lpwstrPath)
//{
//	if(*lpwstrPath==NULL)return NULL;
//
//	HRESULT hRes;
//	ULONG ulSize;
//	ULONG ulAttributes;
//	LPITEMIDLIST pIdl;
//	LPSHELLFOLDER pDesktopFolder;
//
//	if(::SHGetDesktopFolder(&pDesktopFolder)!=NOERROR)return NULL;
//
//	hRes=pDesktopFolder->ParseDisplayName(NULL,NULL,const_cast<LPWSTR>(lpwstrPath),&ulSize,&pIdl,&ulAttributes);
//	pDesktopFolder->Release();
//
//	if(hRes!=NOERROR)pIdl=NULL;
//	return pIdl;
//}
//
//


//Constructor
CIDL::CIDL(LPITEMIDLIST pidl):m_pIDL(pidl){}
CIDL::CIDL(LPCWSTR lpszPath):m_pIDL(::ILCreateFromPath(lpszPath)){}
CIDL::CIDL(const CIDL& idlPtr):m_pIDL(nullptr)
{
	if(idlPtr){
		m_pIDL = ::ILCloneFull(idlPtr.m_pIDL);	
	}
}

//Destructor
CIDL::~CIDL()
{
	if(m_pIDL){
		::ILFree(m_pIDL);
		m_pIDL = nullptr;
	}
}

//Operator=
CIDL& CIDL::operator=(const CIDL& idlPtr)
{
	if(*this!=idlPtr){
		m_pIDL = ::ILCloneFull(idlPtr.m_pIDL);
	}
	return *this;
}

bool CIDL::operator==(const CIDL& idlPtr) const
{
	return ::ILIsEqual(m_pIDL, idlPtr.m_pIDL);
}

bool CIDL::operator!=(const CIDL& idlPtr) const
{
	return !operator==(idlPtr);
}

//Attach
void CIDL::Attach(LPITEMIDLIST pIDL)
{
	if (m_pIDL){
		::ILFree(m_pIDL);
		m_pIDL = nullptr;
	}
	m_pIDL = pIDL;
}
LPITEMIDLIST CIDL::Detach()
{
	LPITEMIDLIST pTempIDL = m_pIDL;
	m_pIDL = nullptr;
	return pTempIDL;
}

void CIDL::Clear()
{
	if (m_pIDL) {
		::ILFree(m_pIDL);
		m_pIDL = nullptr;
	}
}
//Method


CIDL CIDL::CloneFull()const
{
	return CIDL(::ILCloneFull(m_pIDL));
	//return CIDL(CopyItemIdList(m_pIDL));
}

LPITEMIDLIST CIDL::FindLastID()const
{
	return ::ILFindLastID(m_pIDL);
}

CIDL CIDL::CloneLastID()const
{
	return CIDL(::ILClone((LPCITEMIDLIST)FindLastID()));
	//return GetLastItemId(m_pIDL);
}

CIDL CIDL::CloneParentIDL()const
{
	auto pidl = ::ILCloneFull(m_pIDL);
	::ILRemoveLastID(pidl);
	return CIDL(pidl);
	//return CIDL(GetPreviousItemIdList(m_pIDL));
}
//
//void CIDL::Create(UINT uSize)
//{
//	Attach(CreateItemIdList(uSize));
//}
//	
//
//UINT CIDL::GetSize()
//{
//	return GetItemIdListSize(m_pIDL);
//}
//
//void CIDL::SetSpecialFolderLocation(HWND hWnd, int nFolder)
//{
//	PIDLIST_ABSOLUTE pidl_abs=nullptr;
//	throw_if_failed(::SHGetSpecialFolderLocation(hWnd, nFolder, &pidl_abs));
//	Attach(pidl_abs);
//}
//
