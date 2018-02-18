#include "IDLPtr.h"

#include "SingletonMalloc.h"
#include "MyCom.h"

//Static member functions
LPITEMIDLIST CIDLPtr::CreateItemIdList(UINT uSize)
{
	LPITEMIDLIST pIdl;
	pIdl=(LPITEMIDLIST)CSingletonMalloc::GetInstance()->Alloc(uSize);
	if(pIdl!=NULL)::memset(pIdl,0,uSize);
	return pIdl;
}

LPITEMIDLIST CIDLPtr::CopyItemIdList(LPITEMIDLIST pIdl)
{
	//int nSize=pIdl->mkid.cb;
	//LPITEMIDLIST pIdlNew=CreateItemIdList(nSize+sizeof(USHORT));
	//if(pIdlNew==NULL){return NULL;}
	//::memcpy(pIdlNew,pIdl,nSize);
	//::memset((LPBYTE)pIdlNew+nSize,0,sizeof(USHORT));
	//return pIdlNew;
	return ::ILCloneFull(pIdl);
}

PITEMID_CHILD CIDLPtr::GetNextItemId(PITEMID_CHILD pIdl)
{
	UINT uSize(pIdl->mkid.cb);
	if(uSize==0){return NULL;}
	pIdl=(LPITEMIDLIST)(((LPBYTE)pIdl)+uSize);
	if(pIdl->mkid.cb==0){
		return NULL;
	}else{
		return pIdl;
	}
}

PITEMID_CHILD CIDLPtr::GetLastItemId(LPITEMIDLIST pidl)
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

LPITEMIDLIST CIDLPtr::GetPreviousItemIdList(LPITEMIDLIST pIdl)
{
	if(pIdl==nullptr){return nullptr;}
	UINT uFullSize(0);
	UINT uOneSize(0);
	LPITEMIDLIST pIdlTmp=pIdl;
	while(1){
		uOneSize=pIdlTmp->mkid.cb;
		pIdlTmp=GetNextItemId(pIdlTmp);
		if(pIdlTmp==NULL){
			break;
		}else{
			uFullSize+=uOneSize;
		}
	};
	LPITEMIDLIST pIdlPre(NULL);
	if(uFullSize){
		pIdlPre=CreateItemIdList(uFullSize+sizeof(USHORT));
		if(pIdlPre==NULL){return NULL;}
		::memcpy(pIdlPre,pIdl,uFullSize);
		::memset((LPBYTE)pIdlPre+uFullSize,0,sizeof(USHORT));
	}
	return pIdlPre;
}

UINT CIDLPtr::GetItemIdListSize(LPITEMIDLIST pIdl)
{
	UINT uiRet=0;
	if(pIdl==NULL){return 0;}

	uiRet=sizeof(USHORT);

	do{
		uiRet+=(int)pIdl->mkid.cb;
		pIdl=GetNextItemId(pIdl);
	}while(pIdl!=NULL);
	return uiRet;
}

LPITEMIDLIST CIDLPtr::ConcatItemIdList(LPITEMIDLIST pIdl1,LPITEMIDLIST pIdl2)
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

//std::shared_ptr<CShellItemId> CIDL::ConcatItemIdPtr(std::shared_ptr<CShellItemId> pIdl1,std::shared_ptr<CShellItemId> pIdl2)
//{
//	if(!pIdl1.get() && !pIdl2.get()){
//		return std::shared_ptr<CShellItemId>();
//	}else if(!pIdl1.get()){
//		return pIdl2;
//	}else if(!pIdl2.get()){
//		return pIdl1;
//	}
//	return std::make_shared<CShellItemId>(ConcatItemIdList(pIdl1->m_pIdl,pIdl2->m_pIdl));
//}

const CIDLPtr CIDLPtr::operator + ( const CIDLPtr& pidl ) const
{
	if(IsEmpty() && pidl.IsEmpty()){
		return CIDLPtr();
	}else if(IsEmpty()){
		return CIDLPtr(CopyItemIdList(pidl));
	}else if(pidl.IsEmpty()){
		return CIDLPtr(CopyItemIdList(m_pIDL));
	}
	return CIDLPtr(ConcatItemIdList(m_pIDL,(LPITEMIDLIST)pidl));
}


LPITEMIDLIST CIDLPtr::GetItemIdList(LPCWSTR lpwstrPath)
{
	if(*lpwstrPath==NULL)return NULL;

	HRESULT hRes;
	ULONG ulSize;
	ULONG ulAttributes;
	LPITEMIDLIST pIdl;
	LPSHELLFOLDER pDesktopFolder;

	if(::SHGetDesktopFolder(&pDesktopFolder)!=NOERROR)return NULL;

	hRes=pDesktopFolder->ParseDisplayName(NULL,NULL,const_cast<LPWSTR>(lpwstrPath),&ulSize,&pIdl,&ulAttributes);
	pDesktopFolder->Release();

	if(hRes!=NOERROR)pIdl=NULL;
	return pIdl;
}




//Constructor
CIDLPtr::CIDLPtr(LPITEMIDLIST pIDL):m_pIDL(pIDL){}
CIDLPtr::CIDLPtr(LPCWSTR lpszPath):m_pIDL(GetItemIdList(lpszPath)){}
CIDLPtr::CIDLPtr(const CIDLPtr& idlPtr):m_pIDL(nullptr)
{
	if(idlPtr){
		m_pIDL = CopyItemIdList(idlPtr.m_pIDL);	
	}
}

//Destructor
CIDLPtr::~CIDLPtr()
{
	if(m_pIDL){
		CSingletonMalloc::GetInstance()->Free(m_pIDL);
		m_pIDL = nullptr;
	}
}

//Operator=
CIDLPtr& CIDLPtr::operator=(const CIDLPtr& idlPtr)
{
	if(*this!=idlPtr){
		m_pIDL = CopyItemIdList(idlPtr.m_pIDL);
	}
	return *this;
}

bool CIDLPtr::operator==(const CIDLPtr& idlPtr) const
{
	return m_pIDL == idlPtr.m_pIDL;
}

bool CIDLPtr::operator!=(const CIDLPtr& idlPtr) const
{
	return !operator==(idlPtr);
}

//Attach
void CIDLPtr::Attach(LPITEMIDLIST pIDL)
{
	if (m_pIDL){
		CSingletonMalloc::GetInstance()->Free(m_pIDL);
		m_pIDL = nullptr;
	}
	m_pIDL = pIDL;
}
LPITEMIDLIST CIDLPtr::Detach()
{
	LPITEMIDLIST pTempIDL = m_pIDL;
	m_pIDL = nullptr;
	return pTempIDL;
}

void CIDLPtr::Clear()
{
	if (m_pIDL) {
		CSingletonMalloc::GetInstance()->Free(m_pIDL);
		m_pIDL = nullptr;
	}
}
//Method

void CIDLPtr::Append(CIDLPtr pidl)
{
	Attach(ConcatItemIdList(m_pIDL,(LPITEMIDLIST)pidl));
}

CIDLPtr CIDLPtr::Clone()const
{
	return CIDLPtr(CopyItemIdList(m_pIDL));
}

LPITEMIDLIST CIDLPtr::FindLastID()
{
	return ::ILFindLastID(m_pIDL);
}

CIDLPtr CIDLPtr::GetLastIDLPtr()const
{
	return CIDLPtr(GetLastItemId(m_pIDL));
}


CIDLPtr CIDLPtr::GetPreviousIDLPtr()const
{
	return CIDLPtr(GetPreviousItemIdList(m_pIDL));
}

std::wstring CIDLPtr::STRRET2WSTR(STRRET& strret)const
{
	int nLength;
	LPSTR lpmstr;
	WCHAR wcRet[MAX_PATH]={0};
	switch(strret.uType){
		case STRRET_WSTR:
			return std::wstring(strret.pOleStr);
			break;
		case STRRET_OFFSET:
			lpmstr=(LPSTR)(((char*)m_pIDL)+strret.uOffset);
			nLength=::MultiByteToWideChar(CP_THREAD_ACP,0,lpmstr,-1,NULL,0);
			::MultiByteToWideChar(CP_THREAD_ACP,0,lpmstr,-1,wcRet,nLength);
			break;
		case STRRET_CSTR:
			lpmstr=strret.cStr;
			nLength=::MultiByteToWideChar(CP_THREAD_ACP,0,lpmstr,-1,NULL,0);
			::MultiByteToWideChar(CP_THREAD_ACP,0,lpmstr,-1,wcRet,nLength);
			//::wcscpy_s(wcRet,wcslen(wcRet),(LPCWSTR)strret.cStr);
			break;
		default:
			break;
	}
	return std::wstring(wcRet);
}

void CIDLPtr::Create(UINT uSize)
{
	Attach(CreateItemIdList(uSize));
}
	

UINT CIDLPtr::GetSize()
{
	return GetItemIdListSize(m_pIDL);
}

void CIDLPtr::SetSpecialFolderLocation(HWND hWnd, int nFolder)
{
	PIDLIST_ABSOLUTE pidl_abs=nullptr;
	throw_if_failed(::SHGetSpecialFolderLocation(hWnd, nFolder, &pidl_abs));
	Attach(pidl_abs);
}

