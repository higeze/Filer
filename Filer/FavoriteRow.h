#pragma once
#include "FileRow.h"

class CGridView;
class CShellFile;

class CFavoriteRow:public CFileRow
{
private:
	std::wstring m_name;
public:
	CFavoriteRow(CGridView* pGrid, std::shared_ptr<CShellFile> spFile, std::wstring& name);
	virtual ~CFavoriteRow(){}
	std::wstring GetName()const{return m_name;}
};

