#pragma once

std::wstring STRRET2WSTR(STRRET& strret, LPITEMIDLIST pidl);

class CIDL
{
public:
	LPITEMIDLIST m_pIDL;
	//	[&](LPITEMIDLIST pidl){
	//		CSingletonMalloc::GetInstance()->Free(pidl);
	//}> m_spIDL
public:
	//Constructor
	CIDL(LPITEMIDLIST pIdl=nullptr);
	CIDL(LPCWSTR lpszPath);
	CIDL(const CIDL& idlPtr);
	//Operator
	CIDL& operator=(const CIDL& idl);
	bool operator==(const CIDL& idl) const;
	bool operator!=(const CIDL& idl) const;
	CIDL operator + (const CIDL& idl) const;
	//CIDL operator - (const CIDL& idl) const;
	//Destructor
	virtual ~CIDL();
	//Ptr
	LPITEMIDLIST ptr() { return m_pIDL; }
	LPCITEMIDLIST ptr() const { return m_pIDL; }
	LPITEMIDLIST* ptrptr() {return &m_pIDL; }
	LPCITEMIDLIST* ptrptr() const { return (LPCITEMIDLIST*)(&m_pIDL); }
	//Attach
	void Attach(LPITEMIDLIST pIdl);
	LPITEMIDLIST Detach();
	void Clear();
	//Cast
	operator bool()const { return (bool)m_pIDL && m_pIDL->mkid.cb != 0; }
	//operator LPITEMIDLIST()const{return m_pIDL;}
	//operator LPCITEMIDLIST()const{return m_pIDL;}
	//Method
	void Append(CIDL idl);
	//void Insert(CIDL idl);
	//std::pair<CIDL, CIDL> Split()const;
	CIDL CloneFull()const;
	LPITEMIDLIST FindLastID()const;
	CIDL CloneLastID()const;
	CIDL CloneParentIDL()const;
	//void SetSpecialFolderLocation(HWND hWnd, int nFolder);

	std::wstring STRRET2WSTR(STRRET& strret)const;
private:
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
	static CIDL ConcatItemIdPtr(CIDL pIdl1,CIDL pIdl2);
	static PITEMID_CHILD GetLastItemId(LPITEMIDLIST pIdl);
public:	



};