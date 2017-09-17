#pragma once

class CIDLPtr
{
public:
	LPITEMIDLIST m_pIDL;
	//	[&](LPITEMIDLIST pidl){
	//		CSingletonMalloc::GetInstance()->Free(pidl);
	//}> m_spIDL
public:
	//Constructor

	CIDLPtr(LPITEMIDLIST pIdl=nullptr);
	CIDLPtr(LPCWSTR lpszPath);
	CIDLPtr(const CIDLPtr& idlPtr);
	//Operator
	CIDLPtr& operator=(const CIDLPtr& idlPtr);
	bool operator==(const CIDLPtr& idlPtr) const;
	bool operator!=(const CIDLPtr& idlPtr) const;
	const CIDLPtr operator + ( const CIDLPtr& pidl ) const;
	operator bool()const{return m_pIDL!=nullptr;}
	LPITEMIDLIST* operator&() throw()
	{
		return &m_pIDL;
	}
	//Destructor
	virtual ~CIDLPtr();
	//Attach
	void Attach(LPITEMIDLIST pIdl);
	LPITEMIDLIST Detach();
	//Cast
	operator LPITEMIDLIST()const{return m_pIDL;}
	operator LPCITEMIDLIST()const{return m_pIDL;}
	//Method
	bool IsEmpty()const{return !m_pIDL;}
	void Append(CIDLPtr idl);
	void Insert(CIDLPtr idl);
	std::pair<CIDLPtr, CIDLPtr> Split()const;
	CIDLPtr Clone()const;
	LPITEMIDLIST FindLastID();
	CIDLPtr GetLastIDLPtr()const;
	CIDLPtr GetPreviousIDLPtr()const;
	void SetSpecialFolderLocation(HWND hWnd, int nFolder);

	std::wstring STRRET2WSTR(STRRET& strret)const;
private:
	void Create(UINT uSize);	
	UINT GetSize();


	//One ItemId
	static PITEMID_CHILD GetNextItemId(PITEMID_CHILD pIdl);
	//ItemIdList
	static LPITEMIDLIST CreateItemIdList(UINT uSize);
	static LPITEMIDLIST CopyItemIdList(LPITEMIDLIST pidl);
	static LPITEMIDLIST GetPreviousItemIdList(LPITEMIDLIST pidl);
	static LPITEMIDLIST ConcatItemIdList(LPITEMIDLIST pidl1,LPITEMIDLIST pidl2);
	static LPITEMIDLIST GetItemIdList(LPCWSTR lpwstrPath);
	static UINT GetItemIdListSize(LPITEMIDLIST pIdl);
	static CIDLPtr ConcatItemIdPtr(CIDLPtr pIdl1,CIDLPtr pIdl2);
	static PITEMID_CHILD GetLastItemId(LPITEMIDLIST pIdl);
public:	



};