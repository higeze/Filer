#pragma once
#include "IDL.h"

class CKnownFolder
{
private:
	CIDL m_idl;
	std::wstring m_path;
public:
	CKnownFolder(CIDL absoluteIdl, std::wstring path);
	CIDL GetIDL() const { return m_idl; }
	std::wstring GetPath() const { return m_path; }
};

class CKnownFolderManager
{
private:
	std::vector<CKnownFolder> m_knownFolders;
public:
	CKnownFolderManager();
	bool IsKnownFolder(CIDL& idl);
	std::vector<CKnownFolder> GetKnownFolders()const { return m_knownFolders; }
};

