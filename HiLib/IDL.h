#pragma once
#include <windows.h>
#include "Debug.h"

class CIDL
{
public:
	LPITEMIDLIST m_pIDL;
	//	[&](LPITEMIDLIST pidl){
	//		CSingletonMalloc::GetInstance()->Free(pidl);
	//}> m_spIDL
public:
	//Constructor
	explicit CIDL(LPITEMIDLIST pIdl=nullptr);
	CIDL(LPCWSTR lpszPath);
	CIDL(const CIDL& idlPtr);
	CIDL(CIDL&&);
	//Operator
	CIDL& operator=(const CIDL& idl);
	CIDL& operator=(CIDL&&);
	bool operator==(const CIDL& idl) const;
	bool operator!=(const CIDL& idl) const;
	CIDL operator + (const CIDL& idl) const;
	//CIDL operator - (const CIDL& idl) const;
	//Destructor
	virtual ~CIDL();
	//Ptr
	LPITEMIDLIST ptr() { return m_pIDL; }
//	LPITEMIDLIST ptr() const { return m_pIDL; }
	const LPITEMIDLIST ptr() const { return m_pIDL; }

	LPITEMIDLIST* ptrptr() 
	{
		FALSE_THROW(m_pIDL == nullptr);
		return &m_pIDL; }
	LPCITEMIDLIST* constptrptr() const 
	{ 
		return (LPCITEMIDLIST*)(&m_pIDL);
	}
	//Attach
	void Attach(LPITEMIDLIST pIdl);
	LPITEMIDLIST Detach();
	void Clear();
	//Cast
	operator bool()const { return (bool)m_pIDL; }
	//operator LPITEMIDLIST()const{return m_pIDL;}
	//operator LPCITEMIDLIST()const{return m_pIDL;}
	//Method
	void Append(CIDL idl);
	//void Insert(CIDL idl);
	//std::pair<CIDL, CIDL> Split()const;
	CIDL CloneFull()const;
	LPITEMIDLIST FindLastID()const;
	BOOL RemoveLastID();
	CIDL CloneLastID()const;
	CIDL CloneParentIDL()const;
	UINT GetListCount()const;
	//void SetSpecialFolderLocation(HWND hWnd, int nFolder);

	std::wstring strret2wstring(STRRET& strret)const;
public:
	//void Create(UINT uSize);	
	//UINT GetSize();


	//One ItemId
	static PITEMID_CHILD GetNextItemId(PITEMID_CHILD pIdl);
	//ItemIdList
	static LPITEMIDLIST CreateItemIdList(UINT uSize);
	static LPITEMIDLIST CopyItemIdList(LPITEMIDLIST pidl);
	static LPITEMIDLIST GetPreviousItemIdList(LPITEMIDLIST pidl);
	static LPITEMIDLIST ConcatItemIdList(LPITEMIDLIST pidl1,LPITEMIDLIST pidl2);
	static LPITEMIDLIST GetItemIdList(LPCWSTR lpwstrPath);
	static UINT GetItemIdListSize(LPITEMIDLIST pIdl);
	static UINT GetItemIdListCount(LPITEMIDLIST pIdl);
	static CIDL ConcatItemIdPtr(CIDL pIdl1,CIDL pIdl2);
	static PITEMID_CHILD GetLastItemId(LPITEMIDLIST pIdl);
public:	



};